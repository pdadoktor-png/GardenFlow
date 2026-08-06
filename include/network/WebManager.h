#pragma once

#include <Arduino.h>
#include <WebServer.h>

class Scheduler;
class RuntimeManager;
class ValveManager;
class TimeManager;
class WeatherManager;
class SmartControlManager;
class SettingsManager;
class AdvisorEngine;
class WaterManager;
class SeasonManager;

class WebManager
{
public:
    void begin(Scheduler& scheduler,
               RuntimeManager& runtimeManager,
               ValveManager& valveManager,
               TimeManager& timeManager,
               WeatherManager& weatherManager,
               SmartControlManager& smartControlManager,
               SettingsManager& settingsManager,
               AdvisorEngine& advisorEngine,
               WaterManager& waterManager,
               SeasonManager& seasonManager);
    void update();
    bool isStarted() const;

private:
    WebServer server_{80};
    Scheduler* scheduler_ = nullptr;
    RuntimeManager* runtimeManager_ = nullptr;
    ValveManager* valveManager_ = nullptr;
    TimeManager* timeManager_ = nullptr;
    WeatherManager* weatherManager_ = nullptr;
    SmartControlManager* smartControlManager_ = nullptr;
    SettingsManager* settingsManager_ = nullptr;
    AdvisorEngine* advisorEngine_ = nullptr;
    WaterManager* waterManager_ = nullptr;
    SeasonManager* seasonManager_ = nullptr;
    bool started_ = false;
    bool otaStarted_ = false;
    bool wifiWasConnected_ = false;
    uint32_t restartRequestedAtMs_ = 0;

    void startServices();
    void configureRoutes();
    void configureOta();

    void handleRoot();
    void handleStatus();
    void handlePrograms();
    void handleCreateProgram();
    void handleUpdateProgram();
    void handleDeleteProgram();
    void handleCopyProgram();
    void handleToggleProgram();
    void handleStartProgram();
    void handleStop();
    void handleToggleValve();
    void handleWeatherRefresh();
    void handleWeatherSettings();
    void handleSmartSettings();
    void handleLog();
    void handleLogClear();
    void handleSetupSettings();
    void handleSetupSave();
    void handleSetupPortalStart();
    void handleWaterSettings();
    void handleWaterReset();
    void handleProfiles();
    void handleProfileSave();
    void handleProfilesReset();
    void handleNotFound();

    void sendJson(int code, const String& body);
    static String jsonEscape(const String& value);
    static String weekdayText(uint8_t mask);
};
