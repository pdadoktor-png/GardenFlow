#include "backup/BackupManager.h"

#include "settings/SettingsManager.h"
#include "weather/WeatherManager.h"
#include "scheduler/Scheduler.h"
#include "water/WaterManager.h"
#include "season/SeasonManager.h"
#include "advisor/AdvisorEngine.h"

void BackupManager::begin(
    SettingsManager& settingsManager,
    WeatherManager& weatherManager,
    Scheduler& scheduler,
    WaterManager& waterManager,
    SeasonManager& seasonManager,
    AdvisorEngine& advisorEngine)
{
    settingsManager_ = &settingsManager;
    weatherManager_ = &weatherManager;
    scheduler_ = &scheduler;
    waterManager_ = &waterManager;
    seasonManager_ = &seasonManager;
    advisorEngine_ = &advisorEngine;

    Serial.println("BackupManager initialisiert");
}

String BackupManager::createBackupJson()
{
    return R"({
        "version":"0.39.1a",
        "status":"ok"
    })";
}