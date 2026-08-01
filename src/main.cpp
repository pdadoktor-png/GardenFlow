#include <Arduino.h>
#include <lv_conf.h>
#include <esp32_smartdisplay.h>

#include "DisplayManager.h"
#include "ValveManager.h"
#include "Scheduler.h"
#include "runtime/RuntimeManager.h"
#include "time/TimeManager.h"
#include "network/WebManager.h"
#include "weather/WeatherManager.h"
#include "smart/SmartControlManager.h"
#include "log/LogManager.h"
#include "settings/SettingsManager.h"
#include "setup/SetupPortal.h"

static ValveManager valveManager;
static DisplayManager displayManager;
static Scheduler scheduler;
static RuntimeManager runtimeManager;
static TimeManager timeManager;
static WebManager webManager;
static WeatherManager weatherManager;
static SmartControlManager smartControlManager;
static SettingsManager settingsManager;
static SetupPortal setupPortal;

void setup()
{
    Serial.begin(115200);
    delay(800);

    Serial.println();
    Serial.println("================================");
    Serial.println("GardenFlow Professional");
    Serial.println("ESP32-4827S043R");
    Serial.println("================================");

    settingsManager.begin();
    setupPortal.begin(settingsManager);
    valveManager.begin();
    scheduler.begin();


    if (setupPortal.isActive())
    {
        Serial.println(
            "Setupbetrieb: normaler WLAN- und Webbetrieb pausiert"
        );
    }
    else
    {
        timeManager.begin(settingsManager);

        runtimeManager.begin(
            scheduler,
            valveManager,
            timeManager
        );

        weatherManager.begin(
            timeManager,
            settingsManager
        );

        smartControlManager.begin();

        runtimeManager.setWeatherManager(
            weatherManager
        );

        runtimeManager.setSmartControlManager(
            smartControlManager
        );

        webManager.begin(
            scheduler,
            runtimeManager,
            valveManager,
            timeManager,
            weatherManager,
            smartControlManager,
            settingsManager
        );

        Log.begin(&timeManager);
        Log.info(
            LogManager::Category::System,
            "GardenFlow gestartet"
        );
    }

    Serial.println("System bereit");
    

  }

void loop()
{
    valveManager.update();

    if (setupPortal.isActive())
    {
        setupPortal.update();
        delay(2);
        return;
    }

    timeManager.update();
    runtimeManager.update();
    weatherManager.update();
    webManager.update();

    delay(2);
}