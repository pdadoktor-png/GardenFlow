#include <Arduino.h>
#include <lv_conf.h>
#include <esp32_smartdisplay.h>

#include "DisplayManager.h"
#include "ValveManager.h"
#include "Scheduler.h"
#include "runtime/RuntimeManager.h"
#include "time/TimeManager.h"
#include "network/WebManager.h"
#include "network/WifiManager.h"
#include "weather/WeatherManager.h"
#include "smart/SmartControlManager.h"
#include "log/LogManager.h"
#include "settings/SettingsManager.h"
#include "setup/SetupPortal.h"
#include "advisor/AdvisorEngine.h"
#include "water/WaterManager.h"

static ValveManager valveManager;
static DisplayManager displayManager;
static Scheduler scheduler;
static RuntimeManager runtimeManager;
static TimeManager timeManager;
static WebManager webManager;
static WifiManager wifiManager;
static WeatherManager weatherManager;
static SmartControlManager smartControlManager;
static SettingsManager settingsManager;
static SetupPortal setupPortal;
static AdvisorEngine advisorEngine;
static WaterManager waterManager;

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
    wifiManager.begin(settingsManager);
    setupPortal.begin(settingsManager, wifiManager);

    valveManager.begin();
    scheduler.begin();

    if (setupPortal.isActive())
    {
        Serial.println("Setupbetrieb: normaler WLAN- und Webbetrieb pausiert");
    }
    else
    {
        timeManager.begin(settingsManager);
        waterManager.begin(timeManager);
        runtimeManager.begin(scheduler, valveManager, timeManager);
        weatherManager.begin(timeManager, settingsManager);
        smartControlManager.begin();
        advisorEngine.begin(
            weatherManager,
            smartControlManager
        );

        runtimeManager.setWeatherManager(weatherManager);
        runtimeManager.setSmartControlManager(smartControlManager);
        runtimeManager.setWaterManager(waterManager);

        displayManager.begin(
            valveManager,
            scheduler,
            runtimeManager,
            timeManager,
            settingsManager);

        webManager.begin(
            scheduler,
            runtimeManager,
            valveManager,
            timeManager,
            weatherManager,
            smartControlManager,
            settingsManager,
            advisorEngine,
            waterManager);

        Log.begin(&timeManager);
        Log.info(LogManager::Category::System, "GardenFlow gestartet");
    }

    Serial.println("System bereit");
}

void loop()
{
    valveManager.update();
    scheduler.update();

    if (setupPortal.isActive())
    {
        setupPortal.update();
        delay(5);
        return;
    }

    timeManager.update();
    weatherManager.update();
    advisorEngine.update();
    waterManager.update();
    runtimeManager.update();
    displayManager.update();
    webManager.update();

    delay(5);
}
