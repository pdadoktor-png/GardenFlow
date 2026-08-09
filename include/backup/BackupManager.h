#pragma once

#include <Arduino.h>

class SettingsManager;
class WeatherManager;
class Scheduler;
class WaterManager;
class SeasonManager;
class AdvisorEngine;

class BackupManager
{
public:
    void begin(
        SettingsManager& settingsManager,
        WeatherManager& weatherManager,
        Scheduler& scheduler,
        WaterManager& waterManager,
        SeasonManager& seasonManager,
        AdvisorEngine& advisorEngine);

    String createBackupJson() const;

private:
    SettingsManager* settingsManager_ = nullptr;
    WeatherManager* weatherManager_ = nullptr;
    Scheduler* scheduler_ = nullptr;
    WaterManager* waterManager_ = nullptr;
    SeasonManager* seasonManager_ = nullptr;
    AdvisorEngine* advisorEngine_ = nullptr;

    static String jsonEscape(const String& value);
    bool ready() const;
};
