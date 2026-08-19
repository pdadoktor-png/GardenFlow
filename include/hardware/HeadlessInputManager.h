#pragma once

#include <Arduino.h>

class ValveManager;
class RuntimeManager;

#ifndef GARDENFLOW_BUTTON1_GPIO
#define GARDENFLOW_BUTTON1_GPIO 32
#endif

#ifndef GARDENFLOW_BUTTON2_GPIO
#define GARDENFLOW_BUTTON2_GPIO 33
#endif

#ifndef GARDENFLOW_BUTTON_ACTIVE_LOW
#define GARDENFLOW_BUTTON_ACTIVE_LOW 1
#endif

#ifndef GARDENFLOW_BUTTON_DEBOUNCE_MS
#define GARDENFLOW_BUTTON_DEBOUNCE_MS 40
#endif

#ifndef GARDENFLOW_STATUS_LED_GPIO
#define GARDENFLOW_STATUS_LED_GPIO -1
#endif

#ifndef GARDENFLOW_STATUS_LED_ACTIVE_HIGH
#define GARDENFLOW_STATUS_LED_ACTIVE_HIGH 1
#endif

class HeadlessInputManager
{
public:
    void begin(
        ValveManager& valveManager,
        RuntimeManager& runtimeManager
    );

    void update();

private:
    struct Button
    {
        int8_t gpio = -1;
        bool rawPressed = false;
        bool stablePressed = false;
        uint32_t changedAtMs = 0;
    };

    ValveManager* valveManager_ = nullptr;
    RuntimeManager* runtimeManager_ = nullptr;

    Button buttons_[2];
    uint32_t lastLedUpdateMs_ = 0;
    bool ledLevel_ = false;

    bool readPressed(int8_t gpio) const;
    void updateButton(uint8_t index);
    void handlePress(uint8_t index);
    void updateStatusLed();
    void writeLed(bool on);
};
