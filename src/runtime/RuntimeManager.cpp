#include "runtime/RuntimeManager.h"

#include "scheduler/Scheduler.h"
#include "hardware/ValveManager.h"
#include "time/TimeManager.h"
#include "weather/WeatherManager.h"
#include "smart/SmartControlManager.h"
#include "water/WaterManager.h"
#include "log/LogManager.h"
#include "history/HistoryManager.h"
#include <time.h>

void RuntimeManager::begin(
    Scheduler& scheduler,
    ValveManager& valveManager,
    TimeManager& timeManager)
{
    scheduler_ = &scheduler;
    valveManager_ = &valveManager;
    timeManager_ = &timeManager;

    clearState();

    runPreferences_.begin("runtime", false);
    loadPersistedRun();

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

void RuntimeManager::setHistoryManager(
    HistoryManager& historyManager)
{
    historyManager_ = &historyManager;
}

void RuntimeManager::update()
{
    if (recoveryPending_)
    {
        processRecovery();
        return;
    }

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
        const uint32_t actualSeconds =
            min(
                elapsedSeconds(),
                durationSeconds_
            );

        if (historyManager_ != nullptr &&
            scheduler_ != nullptr &&
            runningProgramIndex_ >= 0)
        {
            const auto& program =
                scheduler_->program(
                    static_cast<uint8_t>(
                        runningProgramIndex_
                    )
                );

            historyManager_->recordStop(
                program.id,
                program.valveIndex,
                program.profileId,
                durationSeconds_,
                actualSeconds,
                automaticRun_,
                false
            );
        }

        recordWaterUsage();

        Log.addf(
            LogManager::Category::Program,
            LogManager::Level::Info,
            "Programmlauf beendet; "
            "Ventil %u wird geschlossen",
            valveIndex_ + 1
        );

        clearPersistedRun();
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
        recordSkippedDuePrograms(
            "weather_pause"
        );
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
        recordSkippedDuePrograms(
            "vacation"
        );
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

    savePersistedRun(
        program.id,
        program.valveIndex,
        durationSeconds_,
        automatic
    );

    if (historyManager_ != nullptr)
    {
        historyManager_->recordStart(
            program.id,
            program.valveIndex,
            program.profileId,
            durationSeconds_,
            automatic
        );
    }

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

    const uint32_t actualSeconds =
        min(
            elapsedSeconds(),
            durationSeconds_
        );

    if (historyManager_ != nullptr &&
        scheduler_ != nullptr &&
        runningProgramIndex_ >= 0)
    {
        const auto& program =
            scheduler_->program(
                static_cast<uint8_t>(
                    runningProgramIndex_
                )
            );

        historyManager_->recordStop(
            program.id,
            program.valveIndex,
            program.profileId,
            durationSeconds_,
            actualSeconds,
            automaticRun_,
            true
        );
    }

    recordWaterUsage();

    Log.addf(
        LogManager::Category::Program,
        LogManager::Level::Warning,
        "Programmlauf abgebrochen; "
        "Ventil %u wird geschlossen",
        valveIndex_ + 1
    );

    clearPersistedRun();
    clearState();
    return true;
}

void RuntimeManager::loadPersistedRun()
{
    recoveryPending_ = false;
    persistedRun_ = PersistedRun();

    if (runPreferences_.getBytesLength("active") != sizeof(PersistedRun))
    {
        return;
    }

    PersistedRun stored;
    const size_t read = runPreferences_.getBytes(
        "active",
        &stored,
        sizeof(stored)
    );

    if (read != sizeof(stored) ||
        stored.magic != RUN_MAGIC ||
        stored.programId == 0 ||
        stored.durationSeconds == 0 ||
        stored.valveIndex >= Scheduler::VALVE_COUNT)
    {
        clearPersistedRun();
        return;
    }

    persistedRun_ = stored;
    recoveryPending_ = true;

    Serial.printf(
        "Unterbrochenen Programmlauf gefunden: Programm %lu, Ventil %u\n",
        static_cast<unsigned long>(stored.programId),
        static_cast<unsigned>(stored.valveIndex + 1)
    );
}

void RuntimeManager::savePersistedRun(
    uint32_t programId,
    uint8_t valveIndex,
    uint32_t durationSeconds,
    bool automatic)
{
    PersistedRun stored;
    stored.magic = RUN_MAGIC;
    stored.programId = programId;
    stored.durationSeconds = durationSeconds;
    stored.valveIndex = valveIndex;
    stored.automatic = automatic ? 1 : 0;

    // Exakte Restzeit kann nach Neustart nur mit einer beim Start bereits
    // gueltigen NTP-Zeit berechnet werden.
    if (timeManager_ != nullptr && timeManager_->isValid())
    {
        stored.startEpoch = static_cast<int64_t>(time(nullptr));
    }

    persistedRun_ = stored;
    runPreferences_.putBytes(
        "active",
        &persistedRun_,
        sizeof(persistedRun_)
    );
}

void RuntimeManager::clearPersistedRun()
{
    recoveryPending_ = false;
    persistedRun_ = PersistedRun();
    runPreferences_.remove("active");
}

void RuntimeManager::processRecovery()
{
    if (!recoveryPending_ ||
        scheduler_ == nullptr ||
        valveManager_ == nullptr ||
        timeManager_ == nullptr)
    {
        return;
    }

    // Nicht mit Build-/Fallbackzeit entscheiden. Erst echte NTP-Zeit.
    if (!timeManager_->isValid())
    {
        return;
    }

    const int16_t programIndex =
        scheduler_->findProgramIndexById(persistedRun_.programId);

    const int64_t now = static_cast<int64_t>(time(nullptr));

    // Wenn die Startzeit nicht belastbar gespeichert werden konnte oder das
    // Programm inzwischen geloescht wurde, wird aus Sicherheitsgruenden nicht
    // fortgesetzt. Der persistierte Lauf bedeutet aber, dass vor dem Ausfall
    // ein Oeffnungsimpuls gestartet wurde: Zustand logisch als OFFEN setzen
    // und genau einen Schliessimpuls senden.
    if (programIndex < 0 ||
        persistedRun_.startEpoch <= 0 ||
        now <= 0 ||
        now < persistedRun_.startEpoch)
    {
        if (!valveManager_->restoreAssumedState(
                persistedRun_.valveIndex,
                true) ||
            !valveManager_->pulse(persistedRun_.valveIndex))
        {
            return;
        }

        Log.warning(
            LogManager::Category::Program,
            "Unterbrochener Lauf ohne sichere Zeitbasis: nicht fortgesetzt, Ventil wird geschlossen"
        );

        clearPersistedRun();
        return;
    }

    const uint64_t elapsed64 =
        static_cast<uint64_t>(now - persistedRun_.startEpoch);

    const uint32_t elapsed =
        elapsed64 > 0xFFFFFFFFULL
            ? 0xFFFFFFFFUL
            : static_cast<uint32_t>(elapsed64);

    const auto& program = scheduler_->program(
        static_cast<uint8_t>(programIndex)
    );

    if (elapsed >= persistedRun_.durationSeconds)
    {
        // Das Ventil blieb bei Stromausfall mechanisch in seiner Stellung.
        // Deshalb erst den logischen Zustand als OFFEN rekonstruieren und dann
        // genau EINEN Impuls zum Schliessen ausloesen.
        if (!valveManager_->restoreAssumedState(
                persistedRun_.valveIndex,
                true) ||
            !valveManager_->pulse(persistedRun_.valveIndex))
        {
            return;
        }

        if (historyManager_ != nullptr)
        {
            historyManager_->recordStop(
                program.id,
                program.valveIndex,
                program.profileId,
                persistedRun_.durationSeconds,
                elapsed,
                persistedRun_.automatic != 0,
                true
            );
        }

        if (waterManager_ != nullptr)
        {
            waterManager_->addRuntime(
                persistedRun_.valveIndex,
                elapsed
            );
        }

        Log.addf(
            LogManager::Category::Program,
            LogManager::Level::Warning,
            "Unterbrochener Lauf war bereits abgelaufen; Ventil %u wird nach %lu Sekunden geschlossen",
            static_cast<unsigned>(persistedRun_.valveIndex + 1),
            static_cast<unsigned long>(elapsed)
        );

        clearPersistedRun();
        return;
    }

    // Der Lauf waere noch aktiv. Das Latching-Ventil bleibt mechanisch offen,
    // daher KEIN neuer GPIO-Impuls. Nur den Softwarezustand rekonstruieren.
    if (!valveManager_->restoreAssumedState(
            persistedRun_.valveIndex,
            true))
    {
        return;
    }

    runningProgramIndex_ = programIndex;
    durationSeconds_ = persistedRun_.durationSeconds;
    valveIndex_ = persistedRun_.valveIndex;
    automaticRun_ = persistedRun_.automatic != 0;

    // millis() startet nach dem Reboot bei null. Durch das Zuruecksetzen des
    // virtuellen Startpunkts zaehlen die bereits vergangenen Sekunden mit.
    // Die maximale Programmdauer ist weit kleiner als der millis()-Ueberlauf.
    startedAtMs_ = millis() - (elapsed * 1000UL);

    recoveryPending_ = false;

    Log.addf(
        LogManager::Category::Program,
        LogManager::Level::Warning,
        "Programmlauf nach Stromausfall fortgesetzt: Programm %lu, Rest %lu Sekunden",
        static_cast<unsigned long>(program.id),
        static_cast<unsigned long>(
            persistedRun_.durationSeconds - elapsed)
    );
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

void RuntimeManager::recordSkippedDuePrograms(
    const char* reason)
{
    if (historyManager_ == nullptr ||
        scheduler_ == nullptr ||
        timeManager_ == nullptr ||
        !timeManager_->isValid())
    {
        return;
    }

    struct tm local = {};

    if (!timeManager_->getLocalTime(local))
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

    const uint8_t weekday =
        timeManager_->weekdayMondayZero();

    for (uint8_t i = 0;
         i < Scheduler::MAX_PROGRAMS;
         ++i)
    {
        if (!scheduler_->isProgramUsed(i))
        {
            continue;
        }

        const auto& program =
            scheduler_->program(i);

        if (!program.enabled ||
            !(program.weekdays &
              (1U << weekday)) ||
            program.startHour != local.tm_hour ||
            program.startMinute != local.tm_min)
        {
            continue;
        }

        if (lastSkippedProgramId_ ==
                program.id &&
            lastSkippedDayKey_ ==
                dayKey &&
            lastSkippedMinute_ ==
                minute)
        {
            return;
        }

        historyManager_->recordSkipped(
            program.id,
            program.valveIndex,
            program.profileId,
            program.durationSeconds,
            reason
        );

        lastSkippedProgramId_ =
            program.id;
        lastSkippedDayKey_ =
            dayKey;
        lastSkippedMinute_ =
            minute;

        return;
    }
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
