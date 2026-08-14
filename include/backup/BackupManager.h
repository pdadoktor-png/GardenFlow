#pragma once

#include <Arduino.h>

class SettingsManager;
class WeatherManager;
class Scheduler;
class WaterManager;
class SeasonManager;
class AdvisorEngine;
class GardenManager;

class BackupManager
{
public:
    void begin(
        SettingsManager& settingsManager,
        WeatherManager& weatherManager,
        Scheduler& scheduler,
        WaterManager& waterManager,
        SeasonManager& seasonManager,
        AdvisorEngine& advisorEngine,
        GardenManager& gardenManager);

    String createBackupJson() const;
    bool restoreBackupJson(const String& json, String& message);
    time_t lastBackupEpoch() const;

private:
    SettingsManager* settingsManager_ = nullptr;
    WeatherManager* weatherManager_ = nullptr;
    Scheduler* scheduler_ = nullptr;
    WaterManager* waterManager_ = nullptr;
    SeasonManager* seasonManager_ = nullptr;
    AdvisorEngine* advisorEngine_ = nullptr;
    GardenManager* gardenManager_ = nullptr;
    mutable time_t lastBackupEpoch_ = 0;

    static String jsonEscape(const String& value);
    bool ready() const;
};
