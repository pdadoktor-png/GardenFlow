#include "hardware/HeadlessInputManager.h"

#include <WiFi.h>

#include "hardware/ValveManager.h"
#include "runtime/RuntimeManager.h"
#include "log/LogManager.h"

void HeadlessInputManager::begin(
    ValveManager& valveManager,
    RuntimeManager& runtimeManager)
{
    valveManager_ = &valveManager;
    runtimeManager_ = &runtimeManager;

    buttons_[0].gpio = GARDENFLOW_BUTTON1_GPIO;
    buttons_[1].gpio = GARDENFLOW_BUTTON2_GPIO;

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (buttons_[i].gpio < 0)
        {
            continue;
        }

#if GARDENFLOW_BUTTON_ACTIVE_LOW
        pinMode(buttons_[i].gpio, INPUT_PULLUP);
#else
        pinMode(buttons_[i].gpio, INPUT_PULLDOWN);
#endif

        const bool pressed =
            readPressed(buttons_[i].gpio);

        buttons_[i].rawPressed = pressed;
        buttons_[i].stablePressed = pressed;
        buttons_[i].changedAtMs = millis();

        Serial.printf(
            "Headless Taster %u: GPIO %d, %s\n",
            i + 1,
            static_cast<int>(buttons_[i].gpio),
#if GARDENFLOW_BUTTON_ACTIVE_LOW
            "gegen GND (INPUT_PULLUP)"
#else
            "gegen 3V3 (INPUT_PULLDOWN)"
#endif
        );
    }

#if GARDENFLOW_STATUS_LED_GPIO >= 0
    pinMode(GARDENFLOW_STATUS_LED_GPIO, OUTPUT);
    writeLed(false);
    Serial.printf(
        "Headless Status-LED: GPIO %d\n",
        GARDENFLOW_STATUS_LED_GPIO
    );
#endif

    Serial.println("HeadlessInputManager initialisiert");
}

void HeadlessInputManager::update()
{
    updateButton(0);
    updateButton(1);
    updateStatusLed();
}

bool HeadlessInputManager::readPressed(
    int8_t gpio) const
{
    if (gpio < 0)
    {
        return false;
    }

    const int level = digitalRead(gpio);

#if GARDENFLOW_BUTTON_ACTIVE_LOW
    return level == LOW;
#else
    return level == HIGH;
#endif
}

void HeadlessInputManager::updateButton(
    uint8_t index)
{
    if (index >= 2 ||
        buttons_[index].gpio < 0)
    {
        return;
    }

    Button& button = buttons_[index];
    const bool pressed = readPressed(button.gpio);
    const uint32_t now = millis();

    if (pressed != button.rawPressed)
    {
        button.rawPressed = pressed;
        button.changedAtMs = now;
    }

    if (pressed == button.stablePressed ||
        static_cast<uint32_t>(
            now - button.changedAtMs
        ) < GARDENFLOW_BUTTON_DEBOUNCE_MS)
    {
        return;
    }

    button.stablePressed = pressed;

    // Nur auf die Druckflanke reagieren.
    if (pressed)
    {
        handlePress(index);
    }
}

void HeadlessInputManager::handlePress(
    uint8_t index)
{
    if (valveManager_ == nullptr ||
        runtimeManager_ == nullptr)
    {
        return;
    }

    // Während eines Programmlaufs nicht manuell dazwischenfunken.
    if (runtimeManager_->isRunning())
    {
        Log.addf(
            LogManager::Category::Valve,
            LogManager::Level::Warning,
            "Taster Ventil %u ignoriert: Programmlauf aktiv",
            index + 1
        );
        return;
    }

    if (valveManager_->toggle(index))
    {
        Log.addf(
            LogManager::Category::Valve,
            LogManager::Level::Info,
            "Headless-Taster: Ventil %u umgeschaltet",
            index + 1
        );
    }
    else
    {
        Log.addf(
            LogManager::Category::Valve,
            LogManager::Level::Warning,
            "Headless-Taster: Ventil %u momentan gesperrt",
            index + 1
        );
    }
}

void HeadlessInputManager::updateStatusLed()
{
#if GARDENFLOW_STATUS_LED_GPIO < 0
    return;
#else
    if (valveManager_ == nullptr)
    {
        writeLed(false);
        return;
    }

    const bool valveOpen =
        valveManager_->channel(0).assumedOpen ||
        valveManager_->channel(1).assumedOpen;

    // Ein geöffnetes Ventil hat Priorität: LED dauerhaft EIN.
    if (valveOpen)
    {
        writeLed(true);
        return;
    }

    // WLAN verbunden und alles geschlossen: LED AUS.
    if (WiFi.status() == WL_CONNECTED)
    {
        writeLed(false);
        return;
    }

    // WLAN nicht verbunden: langsames Blinken.
    const uint32_t now = millis();

    if (static_cast<uint32_t>(
            now - lastLedUpdateMs_
        ) >= 500UL)
    {
        lastLedUpdateMs_ = now;
        ledLevel_ = !ledLevel_;
        writeLed(ledLevel_);
    }
#endif
}

void HeadlessInputManager::writeLed(bool on)
{
#if GARDENFLOW_STATUS_LED_GPIO >= 0
#if GARDENFLOW_STATUS_LED_ACTIVE_HIGH
    digitalWrite(
        GARDENFLOW_STATUS_LED_GPIO,
        on ? HIGH : LOW
    );
#else
    digitalWrite(
        GARDENFLOW_STATUS_LED_GPIO,
        on ? LOW : HIGH
    );
#endif
#else
    (void)on;
#endif
}
