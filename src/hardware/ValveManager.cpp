#include "hardware/ValveManager.h"

#include "log/LogManager.h"

void ValveManager::begin()
{
    static const char* names[
        HardwareProfile::VALVE_COUNT
    ] =
    {
        "Ventil 1",
        "Ventil 2"
    };

    for (uint8_t i = 0;
         i < HardwareProfile::VALVE_COUNT;
         ++i)
    {
        Channel& channel = channels_[i];
        const auto& config =
            HardwareProfile::VALVES[i];

        channel.name = names[i];
        channel.gpio = config.gpio;
        channel.activeHigh =
            config.activeHigh;
        channel.assumedOpen = false;
        channel.pulseActive = false;
        channel.pulseDurationMs =
            config.pulseDurationMs;
        channel.pulseCount = 0;

        if (channel.gpio >= 0)
        {
            pinMode(channel.gpio, OUTPUT);
            setOutput(channel, false);
        }

        Serial.printf(
            "%s: GPIO %d, Impuls %lu ms, %s\n",
            channel.name,
            static_cast<int>(channel.gpio),
            static_cast<unsigned long>(
                channel.pulseDurationMs
            ),
            channel.activeHigh
                ? "aktiv HIGH"
                : "aktiv LOW"
        );
    }
}

void ValveManager::update()
{
    const uint32_t now = millis();

    for (uint8_t i = 0;
         i < HardwareProfile::VALVE_COUNT;
         ++i)
    {
        Channel& channel = channels_[i];

        if (channel.pulseActive &&
            static_cast<uint32_t>(
                now - channel.pulseStartedMs
            ) >= channel.pulseDurationMs)
        {
            finishPulse(i);
        }
    }
}

bool ValveManager::toggle(uint8_t index)
{
    return pulse(index);
}

bool ValveManager::pulse(uint8_t index)
{
    if (!validIndex(index))
    {
        return false;
    }

    Channel& channel = channels_[index];
    const uint32_t now = millis();

    if (channel.pulseActive)
    {
        return false;
    }

    if (static_cast<uint32_t>(
            now - channel.lastCommandMs
        ) < AppConfig::BUTTON_LOCKOUT_MS)
    {
        return false;
    }

    channel.lastCommandMs = now;
    channel.pulseStartedMs = now;
    channel.pulseActive = true;
    ++channel.pulseCount;

    setOutput(channel, true);
    notify(index);

    Log.addf(
        LogManager::Category::Valve,
        LogManager::Level::Info,
        "%s: Impuls gestartet",
        channel.name
    );

    return true;
}

const ValveManager::Channel&
ValveManager::channel(uint8_t index) const
{
    static Channel invalidChannel;

    return validIndex(index)
        ? channels_[index]
        : invalidChannel;
}

uint8_t ValveManager::count() const
{
    return HardwareProfile::VALVE_COUNT;
}

void ValveManager::setPulseDurationAll(
    uint32_t durationMs)
{
    durationMs = constrain(
        durationMs,
        100UL,
        2000UL
    );

    for (uint8_t i = 0;
         i < HardwareProfile::VALVE_COUNT;
         ++i)
    {
        channels_[i].pulseDurationMs =
            durationMs;
    }
}

void ValveManager::setStateChangedCallback(
    StateChangedCallback callback)
{
    stateChangedCallback_ = callback;
}

void ValveManager::setOutput(
    const Channel& channel,
    bool active)
{
    if (channel.gpio < 0)
    {
        return;
    }

    const bool level =
        active
            ? channel.activeHigh
            : !channel.activeHigh;

    digitalWrite(
        channel.gpio,
        level ? HIGH : LOW
    );
}

void ValveManager::finishPulse(
    uint8_t index)
{
    Channel& channel = channels_[index];

    setOutput(channel, false);

    channel.pulseActive = false;
    channel.assumedOpen =
        !channel.assumedOpen;

    notify(index);

    Log.addf(
        LogManager::Category::Valve,
        LogManager::Level::Info,
        "%s: Impuls beendet, Zustand = %s",
        channel.name,
        channel.assumedOpen
            ? "OFFEN"
            : "GESCHLOSSEN"
    );
}

bool ValveManager::validIndex(
    uint8_t index) const
{
    return index <
        HardwareProfile::VALVE_COUNT;
}

void ValveManager::notify(uint8_t index)
{
    if (stateChangedCallback_ != nullptr)
    {
        stateChangedCallback_(index);
    }
}
