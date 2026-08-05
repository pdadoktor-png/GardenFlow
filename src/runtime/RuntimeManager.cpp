#include "runtime/RuntimeManager.h"

#include "scheduler/Scheduler.h"
#include "hardware/ValveManager.h"
#include "time/TimeManager.h"
#include "weather/WeatherManager.h"
#include "smart/SmartControlManager.h"
#include "water/WaterManager.h"
#include "log/LogManager.h"

void RuntimeManager::begin(
    Scheduler& scheduler,
    ValveManager& valveManager,
    TimeManager& timeManager)
{
    scheduler_ = &scheduler;
    valveManager_ = &valveManager;
    timeManager_ = &timeManager;

    clearState();

    Serial.println(
        "RuntimeManager initialisiert"
    );
}

void RuntimeManager::setWeatherManager(
    WeatherManager& weatherManager)
{
    weatherManager_ = &weatherManager;
}

void RuntimeManager::setSmartControlManager(
    SmartControlManager&
        smartControlManager)
{
    smartControlManager_ =
        &smartControlManager;
}

void RuntimeManager::setWaterManager(
    WaterManager& waterManager)
{
    waterManager_ = &waterManager;
}

void RuntimeManager::update()
{
    checkAutomaticStart();

    if (!isRunning())
    {
        return;
    }

    if (elapsedSeconds() <
            durationSeconds_ ||
        valveManager_->channel(
            valveIndex_
        ).pulseActive)
    {
        return;
    }

    if (valveManager_->pulse(
            valveIndex_
        ))
    {
        recordWaterUsage();

        Log.addf(
            LogManager::Category::Program,
            LogManager::Level::Info,
            "Programmlauf beendet; "
            "Ventil %u wird geschlossen",
            valveIndex_ + 1
        );

        clearState();
    }
}

void RuntimeManager::checkAutomaticStart()
{
    if (isRunning() ||
        timeManager_ == nullptr ||
        !timeManager_->isValid())
    {
        return;
    }

    if (weatherManager_ != nullptr &&
        weatherManager_->
            automaticPauseActive())
    {
        return;
    }

    struct tm local = {};

    if (!timeManager_->getLocalTime(
            local
        ))
    {
        return;
    }

    if (smartControlManager_ != nullptr &&
        !smartControlManager_->
            automaticRunAllowed(local))
    {
        return;
    }

    const int32_t dayKey =
        (local.tm_year + 1900) *
        1000 +
        local.tm_yday;

    const int16_t minute =
        local.tm_hour * 60 +
        local.tm_min;

    if (dayKey ==
            lastCheckedDayKey_ &&
        minute ==
            lastCheckedMinute_)
    {
        return;
    }

    lastCheckedDayKey_ = dayKey;
    lastCheckedMinute_ = minute;

    const uint8_t weekday =
        timeManager_->
            weekdayMondayZero();

    for (uint8_t i = 0;
         i < Scheduler::MAX_PROGRAMS;
         ++i)
    {
        if (!scheduler_->
                isProgramUsed(i))
        {
            continue;
        }

        const auto& program =
            scheduler_->program(i);

        if (!program.enabled ||
            !(program.weekdays &
                (1U << weekday)) ||
            program.startHour !=
                local.tm_hour ||
            program.startMinute !=
                local.tm_min)
        {
            continue;
        }

        if (lastStartedProgramId_ ==
                program.id &&
            lastStartedDayKey_ ==
                dayKey &&
            lastStartedMinute_ ==
                minute)
        {
            continue;
        }

        if (startProgram(i, true))
        {
            lastStartedProgramId_ =
                program.id;

            lastStartedDayKey_ =
                dayKey;

            lastStartedMinute_ =
                minute;
        }

        break;
    }
}

bool RuntimeManager::startProgram(
    uint8_t programIndex,
    bool automatic)
{
    if (scheduler_ == nullptr ||
        valveManager_ == nullptr ||
        isRunning() ||
        !scheduler_->isProgramUsed(
            programIndex
        ) ||
        !allValvesIdleAndClosed())
    {
        return false;
    }

    const auto& program =
        scheduler_->program(
            programIndex
        );

    if (program.valveIndex >=
            Scheduler::VALVE_COUNT ||
        program.durationSeconds == 0 ||
        !valveManager_->pulse(
            program.valveIndex
        ))
    {
        return false;
    }

    uint32_t effectiveDuration =
        program.durationSeconds;

    if (automatic &&
        smartControlManager_ != nullptr &&
        timeManager_ != nullptr)
    {
        struct tm local = {};

        if (timeManager_->getLocalTime(
                local
            ))
        {
            effectiveDuration =
                static_cast<uint32_t>(
                    (
                        static_cast<uint64_t>(
                            effectiveDuration
                        ) *
                        smartControlManager_->
                            automaticDurationPercent(
                                local
                            )
                    ) /
                    100ULL
                );

            if (effectiveDuration < 60U)
            {
                effectiveDuration = 60U;
            }
        }
    }

    runningProgramIndex_ =
        programIndex;

    startedAtMs_ = millis();
    durationSeconds_ =
        effectiveDuration;
    valveIndex_ =
        program.valveIndex;
    automaticRun_ = automatic;

    Log.addf(
        LogManager::Category::Program,
        LogManager::Level::Info,
        "Programm %lu %s gestartet: "
        "Ventil %u, %lu Sekunden",
        static_cast<unsigned long>(
            program.id
        ),
        automatic
            ? "automatisch"
            : "manuell",
        program.valveIndex + 1,
        static_cast<unsigned long>(
            durationSeconds_
        )
    );

    return true;
}

bool RuntimeManager::stop()
{
    if (!isRunning() ||
        valveManager_->channel(
            valveIndex_
        ).pulseActive ||
        !valveManager_->pulse(
            valveIndex_
        ))
    {
        return false;
    }

    recordWaterUsage();

    Log.addf(
        LogManager::Category::Program,
        LogManager::Level::Warning,
        "Programmlauf abgebrochen; "
        "Ventil %u wird geschlossen",
        valveIndex_ + 1
    );

    clearState();
    return true;
}

bool RuntimeManager::isRunning() const
{
    return
        scheduler_ != nullptr &&
        valveManager_ != nullptr &&
        runningProgramIndex_ >= 0 &&
        runningProgramIndex_ <
            Scheduler::MAX_PROGRAMS;
}

bool RuntimeManager::isProgramRunning(
    uint8_t programIndex) const
{
    return
        isRunning() &&
        runningProgramIndex_ ==
            programIndex;
}

bool RuntimeManager::isAutomaticRun() const
{
    return
        isRunning() &&
        automaticRun_;
}

int16_t
RuntimeManager::runningProgramIndex()
    const
{
    return isRunning()
        ? runningProgramIndex_
        : -1;
}

uint32_t
RuntimeManager::remainingSeconds()
    const
{
    if (!isRunning())
    {
        return 0;
    }

    const uint32_t elapsed =
        elapsedSeconds();

    return elapsed >= durationSeconds_
        ? 0
        : durationSeconds_ - elapsed;
}

uint32_t
RuntimeManager::durationSeconds() const
{
    return isRunning()
        ? durationSeconds_
        : 0;
}

uint8_t
RuntimeManager::runningValveIndex()
    const
{
    return isRunning()
        ? valveIndex_
        : 0;
}

int16_t
RuntimeManager::nextProgramIndex()
    const
{
    if (scheduler_ == nullptr ||
        timeManager_ == nullptr ||
        !timeManager_->isValid())
    {
        return -1;
    }

    struct tm local = {};
    timeManager_->getLocalTime(local);

    const int now =
        local.tm_hour * 60 +
        local.tm_min;

    const uint8_t today =
        timeManager_->
            weekdayMondayZero();

    int best = 99999;
    int16_t index = -1;

    for (uint8_t i = 0;
         i < Scheduler::MAX_PROGRAMS;
         ++i)
    {
        if (!scheduler_->
                isProgramUsed(i))
        {
            continue;
        }

        const auto& program =
            scheduler_->program(i);

        if (!program.enabled)
        {
            continue;
        }

        for (int day = 0;
             day < 8;
             ++day)
        {
            const uint8_t weekday =
                (today + day) % 7;

            if (!(program.weekdays &
                    (1U << weekday)))
            {
                continue;
            }

            const int delta =
                day * 1440 +
                program.startHour * 60 +
                program.startMinute -
                now;

            if (delta < 0)
            {
                continue;
            }

            if (delta < best)
            {
                best = delta;
                index = i;
            }

            break;
        }
    }

    return index;
}

void RuntimeManager::clearState()
{
    runningProgramIndex_ = -1;
    startedAtMs_ = 0;
    durationSeconds_ = 0;
    valveIndex_ = 0;
    automaticRun_ = false;
}

bool RuntimeManager::
allValvesIdleAndClosed() const
{
    for (uint8_t valve = 0;
         valve < Scheduler::VALVE_COUNT;
         ++valve)
    {
        const auto& channel =
            valveManager_->channel(
                valve
            );

        if (channel.pulseActive ||
            channel.assumedOpen)
        {
            return false;
        }
    }

    return true;
}

uint32_t
RuntimeManager::elapsedSeconds() const
{
    if (!isRunning())
    {
        return 0;
    }

    return
        static_cast<uint32_t>(
            millis() - startedAtMs_
        ) /
        1000UL;
}

void RuntimeManager::recordWaterUsage()
{
    if (waterManager_ == nullptr)
    {
        return;
    }

    uint32_t seconds =
        elapsedSeconds();

    if (seconds >
        durationSeconds_)
    {
        seconds =
            durationSeconds_;
    }

    waterManager_->addRuntime(
        valveIndex_,
        seconds
    );
}
