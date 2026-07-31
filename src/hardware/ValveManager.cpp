#include "ValveManager.h"

void ValveManager::begin()
{
    static const char* names[AppConfig::MAX_VALVE_COUNT] =
    {
        "Ventil 1", "Ventil 2", "Ventil 3", "Ventil 4",
        "Ventil 5", "Ventil 6", "Ventil 7", "Ventil 8"
    };

    for (uint8_t i = 0; i < AppConfig::MAX_VALVE_COUNT; ++i)
    {
        Channel& ch = channels_[i];
        ch.name = names[i];
        ch.gpio = AppConfig::VALVE_GPIO[i];
        ch.assumedOpen = false;
        ch.pulseActive = false;
        ch.pulseDurationMs = AppConfig::DEFAULT_PULSE_MS;
        ch.pulseCount = 0;

        if (ch.gpio >= 0)
        {
            pinMode(ch.gpio, OUTPUT);
            digitalWrite(ch.gpio, LOW);
        }
    }
}

void ValveManager::update()
{
    const uint32_t now = millis();
    for (uint8_t i = 0; i < AppConfig::MAX_VALVE_COUNT; ++i)
    {
        Channel& ch = channels_[i];
        if (ch.pulseActive && (now - ch.pulseStartedMs >= ch.pulseDurationMs))
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
    if (!validIndex(index)) return false;

    Channel& ch = channels_[index];
    const uint32_t now = millis();

    if (ch.pulseActive) return false;
    if ((now - ch.lastCommandMs) < AppConfig::BUTTON_LOCKOUT_MS) return false;

    ch.lastCommandMs = now;
    ch.pulseStartedMs = now;
    ch.pulseActive = true;
    ++ch.pulseCount;

    setOutput(ch, true);
    notify(index);
    Serial.printf("%s: Impuls gestartet\n", ch.name);
    return true;
}

const ValveManager::Channel& ValveManager::channel(uint8_t index) const
{
    static Channel invalidChannel;
    return validIndex(index) ? channels_[index] : invalidChannel;
}

uint8_t ValveManager::count() const
{
    return AppConfig::MAX_VALVE_COUNT;
}


void ValveManager::setPulseDurationAll(uint32_t durationMs)
{
    durationMs = constrain(durationMs, 100UL, 1000UL);

    for (uint8_t i = 0; i < AppConfig::MAX_VALVE_COUNT; ++i)
    {
        channels_[i].pulseDurationMs = durationMs;
    }
}

void ValveManager::setStateChangedCallback(StateChangedCallback callback)
{
    stateChangedCallback_ = callback;
}

void ValveManager::setOutput(const Channel& channel, bool active)
{
    if (channel.gpio >= 0)
    {
        digitalWrite(channel.gpio, active ? HIGH : LOW);
    }
}

void ValveManager::finishPulse(uint8_t index)
{
    Channel& ch = channels_[index];
    setOutput(ch, false);
    ch.pulseActive = false;
    ch.assumedOpen = !ch.assumedOpen;
    notify(index);

    Serial.printf("%s: Impuls beendet, Zustand = %s\n",
                  ch.name,
                  ch.assumedOpen ? "OFFEN" : "GESCHLOSSEN");
}

bool ValveManager::validIndex(uint8_t index) const
{
    return index < AppConfig::MAX_VALVE_COUNT;
}

void ValveManager::notify(uint8_t index)
{
    if (stateChangedCallback_ != nullptr)
    {
        stateChangedCallback_(index);
    }
}
