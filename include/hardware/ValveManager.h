#pragma once
#include <Arduino.h>
#include "AppConfig.h"

class ValveManager
{
public:
    struct Channel
    {
        const char* name = "";
        int8_t gpio = -1;
        bool assumedOpen = false;
        bool pulseActive = false;
        uint32_t pulseStartedMs = 0;
        uint32_t lastCommandMs = 0;
        uint32_t pulseDurationMs = AppConfig::DEFAULT_PULSE_MS;
        uint32_t pulseCount = 0;
    };

    using StateChangedCallback = void (*)(uint8_t index);

    void begin();
    void update();
    bool toggle(uint8_t index);
    bool pulse(uint8_t index);
    const Channel& channel(uint8_t index) const;
    uint8_t count() const;
    void setPulseDurationAll(uint32_t durationMs);
    void setStateChangedCallback(StateChangedCallback callback);

private:
    Channel channels_[AppConfig::MAX_VALVE_COUNT];
    StateChangedCallback stateChangedCallback_ = nullptr;

    void setOutput(const Channel& channel, bool active);
    void finishPulse(uint8_t index);
    bool validIndex(uint8_t index) const;
    void notify(uint8_t index);
};
