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

    stateStoreReady_ = stateStore_.begin("valve-state", false);

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
        channel.stateKnown = false;
        if (stateStoreReady_)
        {
            char key[12];
            snprintf(key, sizeof(key), "state%u", i);
            const uint8_t saved = stateStore_.getUChar(key, 0xFF);
            channel.stateKnown = (saved <= 1);
            channel.assumedOpen = (saved == 1);
            snprintf(key, sizeof(key), "pending%u", i);
            if (stateStore_.getBool(key, false))
            {
                channel.stateKnown = false;
                Serial.printf("WARNUNG: %s - Stromausfall waehrend Schaltimpuls moeglich, Stellung unbekannt!\n", channel.name);
            }
        }
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

    // Vor dem Ausgangsimpuls persistent markieren: Unterbrechung => unbekannt.
    saveState(index, true);
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

bool ValveManager::restoreAssumedState(
    uint8_t index,
    bool open)
{
    if (!validIndex(index))
    {
        return false;
    }

    Channel& channel = channels_[index];

    if (channel.pulseActive)
    {
        return false;
    }

    channel.assumedOpen = open;
    channel.stateKnown = true;
    saveState(index, false);
    notify(index);

    Log.addf(
        LogManager::Category::Valve,
        LogManager::Level::Warning,
        "%s: logischer Zustand nach Neustart rekonstruiert = %s (kein Impuls)",
        channel.name,
        open ? "OFFEN" : "GESCHLOSSEN"
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
    // Ein unklarer Ausgangszustand bleibt auch nach einem Toggle unklar.
    saveState(index, false);

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

void ValveManager::saveState(uint8_t index, bool pending)
{
    if (!stateStoreReady_ || !validIndex(index)) return;
    char key[12];
    if (pending)
    {
        snprintf(key, sizeof(key), "pending%u", index);
        stateStore_.putBool(key, true);
    }
    else
    {
        snprintf(key, sizeof(key), "state%u", index);
        stateStore_.putUChar(key, channels_[index].stateKnown ? (channels_[index].assumedOpen ? 1 : 0) : 0xFF);
        snprintf(key, sizeof(key), "pending%u", index);
        stateStore_.putBool(key, false);
    }
}
