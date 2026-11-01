#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "hardware/HeadlessInputManager.h"
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
#include "profiles/GardenProfiles.h"
#include "water/WaterManager.h"
#include "season/SeasonManager.h"
#include "backup/BackupManager.h"
#include "history/HistoryManager.h"
#include "garden/GardenManager.h"

static ValveManager valveManager;
static HeadlessInputManager headlessInputManager;
static bool otaReady = false;
static void serviceOta() {
    if (WiFi.status() != WL_CONNECTED) { otaReady = false; return; }
    if (!otaReady) {
        ArduinoOTA.setHostname("gardenflow-outdoor");
        ArduinoOTA.begin();
        otaReady = true;
        Serial.print("OTA bereit: "); Serial.println(WiFi.localIP());
    }
    ArduinoOTA.handle();
}
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
static SeasonManager seasonManager;
static BackupManager backupManager;
static HistoryManager historyManager;
static GardenManager gardenManager;

void setup()
{
    Serial.begin(115200);
    delay(800);

    Serial.println();
    Serial.println("================================");
    Serial.println("GardenFlow Outdoor");
    Serial.println("ESP32-WROOM-32");
    Serial.println("================================");

    settingsManager.begin();
    wifiManager.begin(settingsManager);
    setupPortal.begin(settingsManager, wifiManager);

    valveManager.begin();
    GardenProfiles::begin();
    scheduler.begin();
    

    gardenManager.begin();
    backupManager.begin(
        settingsManager,
        weatherManager,
        scheduler,
        waterManager,
        seasonManager,
        advisorEngine,
        gardenManager
    );
    if (setupPortal.isActive())
    {
        Serial.println("Setupbetrieb: normaler WLAN- und Webbetrieb pausiert");
    }
    else
    {
        timeManager.begin(settingsManager);
        waterManager.begin(timeManager);
        seasonManager.begin(timeManager, settingsManager);
        runtimeManager.begin(scheduler, valveManager, timeManager);
        weatherManager.begin(timeManager, settingsManager);
        smartControlManager.begin();
        smartControlManager.setSeasonManager(seasonManager);
        advisorEngine.begin(
            weatherManager,
            smartControlManager
        );

        historyManager.begin(
            timeManager,
            waterManager,
            smartControlManager,
            advisorEngine
        );

        runtimeManager.setWeatherManager(weatherManager);
        runtimeManager.setSmartControlManager(smartControlManager);
        runtimeManager.setWaterManager(waterManager);
        runtimeManager.setHistoryManager(historyManager);

        headlessInputManager.begin(valveManager, runtimeManager);

        webManager.begin(
            scheduler,
            runtimeManager,
            valveManager,
            timeManager,
            weatherManager,
            smartControlManager,
            settingsManager,
            advisorEngine,
            waterManager,
            seasonManager,
            backupManager,
            historyManager,
            gardenManager);

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
        serviceOta();
        delay(5);
        return;
    }

    timeManager.update();
    weatherManager.update();
    seasonManager.update();
    advisorEngine.update();
    waterManager.update();
    runtimeManager.update();
    headlessInputManager.update();
    serviceOta();
    webManager.update();

    delay(5);
}
