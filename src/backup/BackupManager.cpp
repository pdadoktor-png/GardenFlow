#include "backup/BackupManager.h"

#include "advisor/AdvisorEngine.h"
#include "profiles/GardenProfiles.h"
#include "scheduler/Scheduler.h"
#include "season/SeasonManager.h"
#include "settings/SettingsManager.h"
#include "water/WaterManager.h"
#include "weather/WeatherManager.h"

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

String BackupManager::createBackupJson() const
{
    if (!ready())
    {
        return F("{\"version\":\"0.39.1b\",\"status\":\"error\",\"error\":\"BackupManager nicht bereit\"}");
    }

    String body;
    body.reserve(9000);

    body += F("{\"version\":\"0.39.1b\",\"status\":\"ok\"");
    body += F(",\"secretsIncluded\":false");

    // Standort und allgemeine Einstellungen. Passwörter/API-Schlüssel werden
    // bewusst nicht über den offenen GET-Endpunkt ausgegeben.
    body += F(",\"settings\":{");
    body += F("\"ssid\":\"");
    body += jsonEscape(settingsManager_->wifiSsid());
    body += F("\",\"timezone\":\"");
    body += jsonEscape(settingsManager_->timezone());
    body += F("\",\"latitude\":");
    body += String(settingsManager_->latitude(), 5);
    body += F(",\"longitude\":");
    body += String(settingsManager_->longitude(), 5);
    body += F("}");

    // Wetter-Konfiguration und letzter bekannter Zustand.
    body += F(",\"weather\":{");
    body += F("\"automaticPauseEnabled\":");
    body += weatherManager_->automaticPauseEnabled() ? F("true") : F("false");
    body += F(",\"rainLimitMm\":");
    body += String(weatherManager_->rainLimitMm(), 1);
    body += F(",\"probabilityLimitPercent\":");
    body += String(weatherManager_->probabilityLimitPercent());
    body += F(",\"lastTemperatureC\":");
    body += String(weatherManager_->temperatureC(), 1);
    body += F(",\"lastHumidityPercent\":");
    body += String(weatherManager_->humidityPercent());
    body += F(",\"lastRainMm24h\":");
    body += String(weatherManager_->rainMmNext24Hours(), 1);
    body += F(",\"lastRainProbabilityPercent\":");
    body += String(weatherManager_->maxRainProbabilityPercent());
    body += F("}");

    // Bewässerungsprogramme.
    body += F(",\"programs\":[");
    bool firstProgram = true;
    for (uint8_t i = 0; i < scheduler_->programCount(); ++i)
    {
        if (!scheduler_->isProgramUsed(i))
        {
            continue;
        }

        const Scheduler::IrrigationProgram& program = scheduler_->program(i);
        if (!firstProgram)
        {
            body += ',';
        }
        firstProgram = false;

        body += F("{\"id\":");
        body += String(program.id);
        body += F(",\"enabled\":");
        body += program.enabled ? F("true") : F("false");
        body += F(",\"valve\":");
        body += String(program.valveIndex);
        body += F(",\"profile\":");
        body += String(program.profileId);
        body += F(",\"hour\":");
        body += String(program.startHour);
        body += F(",\"minute\":");
        body += String(program.startMinute);
        body += F(",\"durationSeconds\":");
        body += String(program.durationSeconds);
        body += F(",\"weekdays\":");
        body += String(program.weekdays);
        body += F("}");
    }
    body += F("]");

    // Editierbare Pflanzenprofile.
    body += F(",\"profiles\":[");
    for (uint8_t i = 0; i < GardenProfiles::PROFILE_COUNT; ++i)
    {
        if (i > 0)
        {
            body += ',';
        }

        const GardenProfiles::Profile& profile = GardenProfiles::profileByIndex(i);
        body += F("{\"id\":");
        body += String(i);
        body += F(",\"name\":\"");
        body += jsonEscape(String(profile.name));
        body += F("\",\"symbol\":\"");
        body += jsonEscape(String(profile.symbol));
        body += F("\",\"correctionPercent\":");
        body += String(profile.correctionPercent);
        body += F(",\"temperatureSensitivityPercent\":");
        body += String(profile.temperatureSensitivityPercent);
        body += F(",\"humiditySensitivityPercent\":");
        body += String(profile.humiditySensitivityPercent);
        body += F(",\"rainSensitivityPercent\":");
        body += String(profile.rainSensitivityPercent);
        body += F(",\"minimumMinutes\":");
        body += String(profile.minimumMinutes);
        body += F(",\"maximumMinutes\":");
        body += String(profile.maximumMinutes);
        body += F("}");
    }
    body += F("]");

    // Wasser- und Kostenparameter inklusive aktueller Zählerstände.
    const WaterStatistics& water = waterManager_->statistics();
    body += F(",\"water\":{");
    body += F("\"flowValve1Lpm\":");
    body += String(waterManager_->valveFlowRate(0), 2);
    body += F(",\"flowValve2Lpm\":");
    body += String(waterManager_->valveFlowRate(1), 2);
    body += F(",\"priceEuroPerM3\":");
    body += String(waterManager_->waterPrice(), 2);
    body += F(",\"todayLiters\":");
    body += String(water.todayLiters, 2);
    body += F(",\"weekLiters\":");
    body += String(water.weekLiters, 2);
    body += F(",\"monthLiters\":");
    body += String(water.monthLiters, 2);
    body += F(",\"yearLiters\":");
    body += String(water.yearLiters, 2);
    body += F(",\"savedLiters\":");
    body += String(water.savedLiters, 2);
    body += F("}");

    // Automatische Saisonberechnung.
    body += F(",\"season\":{");
    body += F("\"valid\":");
    body += seasonManager_->isValid() ? F("true") : F("false");
    body += F(",\"name\":\"");
    body += jsonEscape(seasonManager_->seasonName());
    body += F("\",\"percent\":");
    body += String(seasonManager_->seasonPercent());
    body += F(",\"dayLengthHours\":");
    body += String(seasonManager_->dayLengthHours(), 2);
    body += F("}");

    // Aktuelle Advisor-Empfehlung als Diagnose-Snapshot.
    const AdvisorRecommendation& advisor = advisorEngine_->recommendation();
    body += F(",\"advisor\":{");
    body += F("\"valid\":");
    body += advisor.valid ? F("true") : F("false");
    body += F(",\"adjustmentPercent\":");
    body += String(advisor.adjustmentPercent);
    body += F(",\"weatherAdjustmentPercent\":");
    body += String(advisor.weatherAdjustmentPercent);
    body += F(",\"seasonPercent\":");
    body += String(advisor.seasonPercent);
    body += F(",\"combinedPercent\":");
    body += String(advisor.combinedPercent);
    body += F(",\"confidencePercent\":");
    body += String(advisor.confidencePercent);
    body += F(",\"headline\":\"");
    body += jsonEscape(advisor.headline);
    body += F("\"}");

    body += F("}");
    return body;
}

String BackupManager::jsonEscape(const String& value)
{
    String escaped;
    escaped.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); ++i)
    {
        const char c = value.charAt(i);
        switch (c)
        {
            case '\\': escaped += F("\\\\"); break;
            case '"':  escaped += F("\\\""); break;
            case '\n': escaped += F("\\n"); break;
            case '\r': escaped += F("\\r"); break;
            case '\t': escaped += F("\\t"); break;
            default:
                if (static_cast<uint8_t>(c) >= 0x20U)
                {
                    escaped += c;
                }
                break;
        }
    }

    return escaped;
}

bool BackupManager::ready() const
{
    return settingsManager_ != nullptr &&
           weatherManager_ != nullptr &&
           scheduler_ != nullptr &&
           waterManager_ != nullptr &&
           seasonManager_ != nullptr &&
           advisorEngine_ != nullptr;
}
