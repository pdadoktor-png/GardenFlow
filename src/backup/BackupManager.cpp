#include "backup/BackupManager.h"

#include <ArduinoJson.h>

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
        return F("{\"system\":{\"version\":\"0.39.1c\",\"format\":1,\"status\":\"error\"},\"error\":\"BackupManager nicht bereit\"}");
    }

    String body;
    body.reserve(9500);

    body += F("{\"system\":{");
    body += F("\"version\":\"0.39.1c\"");
    body += F(",\"format\":1");
    body += F(",\"status\":\"ok\"");
    body += F(",\"secretsIncluded\":false");
    body += F("}");

    // Dauerhaft wiederherstellbare Konfiguration.
    body += F(",\"configuration\":{");
    body += F("\"settings\":{");
    body += F("\"ssid\":\"");
    body += jsonEscape(settingsManager_->wifiSsid());
    body += F("\",\"timezone\":\"");
    body += jsonEscape(settingsManager_->timezone());
    body += F("\",\"latitude\":");
    body += String(settingsManager_->latitude(), 5);
    body += F(",\"longitude\":");
    body += String(settingsManager_->longitude(), 5);
    body += F("}");

    body += F(",\"weather\":{");
    body += F("\"automaticPauseEnabled\":");
    body += weatherManager_->automaticPauseEnabled() ? F("true") : F("false");
    body += F(",\"rainLimitMm\":");
    body += String(weatherManager_->rainLimitMm(), 1);
    body += F(",\"probabilityLimitPercent\":");
    body += String(weatherManager_->probabilityLimitPercent());
    body += F("}");

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

    body += F(",\"water\":{");
    body += F("\"flowValve1Lpm\":");
    body += String(waterManager_->valveFlowRate(0), 2);
    body += F(",\"flowValve2Lpm\":");
    body += String(waterManager_->valveFlowRate(1), 2);
    body += F(",\"priceEuroPerM3\":");
    body += String(waterManager_->waterPrice(), 2);
    body += F("}");
    body += F("}"); // configuration

    // Momentaufnahme des aktuellen Systemzustands. Dieser Bereich wird
    // später beim Restore standardmäßig nicht zurückgeschrieben.
    body += F(",\"runtime\":{");
    body += F("\"weather\":{");
    body += F("\"temperatureC\":");
    body += String(weatherManager_->temperatureC(), 1);
    body += F(",\"humidityPercent\":");
    body += String(weatherManager_->humidityPercent());
    body += F(",\"rainMm24h\":");
    body += String(weatherManager_->rainMmNext24Hours(), 1);
    body += F(",\"rainProbabilityPercent\":");
    body += String(weatherManager_->maxRainProbabilityPercent());
    body += F("}");

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
    body += F("}"); // runtime

    // Persistente Verbrauchsstatistik.
    const WaterStatistics& water = waterManager_->statistics();
    body += F(",\"statistics\":{");
    body += F("\"water\":{");
    body += F("\"todayLiters\":");
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
    body += F("}"); // statistics

    body += F("}");

    const time_t now = time(nullptr);
    if (now > 0)
    {
        lastBackupEpoch_ = now;
    }

    return body;
}

time_t BackupManager::lastBackupEpoch() const
{
    return lastBackupEpoch_;
}


bool BackupManager::restoreBackupJson(
    const String& json,
    String& message)
{
    if (!ready())
    {
        message = "BackupManager nicht bereit";
        return false;
    }

    JsonDocument document;
    const DeserializationError error =
        deserializeJson(document, json);

    if (error)
    {
        message =
            String("JSON ungültig: ") +
            error.c_str();
        return false;
    }

    JsonObject system =
        document["system"].as<JsonObject>();

    JsonObject configuration =
        document["configuration"].as<JsonObject>();

    if (system.isNull() ||
        configuration.isNull())
    {
        message =
            "Kein GardenFlow-Backupformat";
        return false;
    }

    const int format =
        system["format"] | 0;

    if (format != 1)
    {
        message =
            String("Backupformat nicht unterstützt: ") +
            format;
        return false;
    }

    JsonArray programs =
        configuration["programs"].as<JsonArray>();

    JsonArray profiles =
        configuration["profiles"].as<JsonArray>();

    if (programs.isNull() ||
        profiles.isNull())
    {
        message =
            "Programme oder Profile fehlen";
        return false;
    }

    if (programs.size() >
        Scheduler::MAX_PROGRAMS)
    {
        message =
            "Zu viele Programme im Backup";
        return false;
    }

    // Erst vollständig prüfen, danach erst bestehende Daten ändern.
    for (JsonObject program : programs)
    {
        const int valve =
            program["valve"] | -1;
        const int profile =
            program["profile"] | -1;
        const int hour =
            program["hour"] | -1;
        const int minute =
            program["minute"] | -1;
        const int durationSeconds =
            program["durationSeconds"] | 0;
        const int weekdays =
            program["weekdays"] | -1;

        if (valve < 0 ||
            valve >= Scheduler::VALVE_COUNT ||
            profile < 0 ||
            !GardenProfiles::isValid(
                static_cast<uint8_t>(profile)
            ) ||
            hour < 0 ||
            hour > 23 ||
            minute < 0 ||
            minute > 59 ||
            durationSeconds <
                Scheduler::MIN_DURATION_MINUTES * 60 ||
            durationSeconds >
                Scheduler::MAX_DURATION_MINUTES * 60 ||
            weekdays < 0 ||
            weekdays > 0x7F)
        {
            message =
                "Ungültige Programmdaten im Backup";
            return false;
        }
    }

    for (JsonObject profile : profiles)
    {
        const int id =
            profile["id"] | -1;
        const int correction =
            profile["correctionPercent"] | 0;
        const int temperature =
            profile["temperatureSensitivityPercent"] | 100;
        const int humidity =
            profile["humiditySensitivityPercent"] | 100;
        const int rain =
            profile["rainSensitivityPercent"] | 100;
        const int minimum =
            profile["minimumMinutes"] | 1;
        const int maximum =
            profile["maximumMinutes"] | 240;

        const String name =
            profile["name"] | "";

        if (id < 0 ||
            !GardenProfiles::isValid(
                static_cast<uint8_t>(id)
            ) ||
            name.length() == 0 ||
            correction < -80 ||
            correction > 100 ||
            temperature < 0 ||
            temperature > 200 ||
            humidity < 0 ||
            humidity > 200 ||
            rain < 0 ||
            rain > 200 ||
            minimum < 1 ||
            maximum < minimum ||
            maximum > 240)
        {
            message =
                "Ungültige Profildaten im Backup";
            return false;
        }
    }

    // Standort und Zeitzone werden übernommen.
    // SSID, WLAN-Passwort und API-Key bleiben bewusst auf dem Gerät.
    JsonObject settings =
        configuration["settings"].as<JsonObject>();

    if (!settings.isNull())
    {
        const float latitude =
            settings["latitude"] |
            settingsManager_->latitude();

        const float longitude =
            settings["longitude"] |
            settingsManager_->longitude();

        const String timezone =
            settings["timezone"] |
            settingsManager_->timezone();

        if (!settingsManager_->saveNetworkLocation(
                settingsManager_->wifiSsid(),
                String(),
                latitude,
                longitude,
                timezone,
                String()
            ))
        {
            message =
                "Standort/Zeitzone konnten nicht wiederhergestellt werden";
            return false;
        }
    }

    JsonObject weather =
        configuration["weather"].as<JsonObject>();

    if (!weather.isNull())
    {
        weatherManager_->setAutomaticPauseEnabled(
            weather["automaticPauseEnabled"] |
            weatherManager_->automaticPauseEnabled()
        );

        weatherManager_->setRainLimitMm(
            weather["rainLimitMm"] |
            weatherManager_->rainLimitMm()
        );

        weatherManager_->setProbabilityLimitPercent(
            static_cast<uint8_t>(
                weather["probabilityLimitPercent"] |
                weatherManager_->
                    probabilityLimitPercent()
            )
        );
    }

    // Profile zuerst, damit Programme sofort auf die restaurierten
    // Profilwerte zeigen.
    for (JsonObject profile : profiles)
    {
        const uint8_t id =
            profile["id"].as<uint8_t>();

        if (!GardenProfiles::update(
                id,
                String(
                    profile["name"] |
                    GardenProfiles::name(id)
                ),
                String(
                    profile["symbol"] |
                    GardenProfiles::symbol(id)
                ),
                profile["correctionPercent"] | 0,
                profile["temperatureSensitivityPercent"] | 100,
                profile["humiditySensitivityPercent"] | 100,
                profile["rainSensitivityPercent"] | 100,
                profile["minimumMinutes"] | 1,
                profile["maximumMinutes"] | 240
            ))
        {
            message =
                String("Profil ") +
                id +
                " konnte nicht gespeichert werden";
            return false;
        }
    }

    // Bestehende Programme entfernen.
    for (uint8_t i = 0;
         i < Scheduler::MAX_PROGRAMS;
         ++i)
    {
        if (scheduler_->isProgramUsed(i) &&
            !scheduler_->deleteProgram(i))
        {
            message =
                "Bestehende Programme konnten nicht gelöscht werden";
            return false;
        }
    }

    // Programme neu anlegen. Die fachlichen Daten bleiben identisch;
    // interne Programm-IDs werden bewusst neu vergeben.
    for (JsonObject source : programs)
    {
        const uint8_t valve =
            source["valve"].as<uint8_t>();

        const int16_t index =
            scheduler_->createProgram(valve);

        if (index < 0)
        {
            message =
                "Programm konnte nicht angelegt werden";
            return false;
        }

        Scheduler::IrrigationProgram& target =
            scheduler_->program(
                static_cast<uint8_t>(index)
            );

        target.enabled =
            source["enabled"] | false;

        target.profileId =
            source["profile"] | 0;

        target.startHour =
            source["hour"] | 6;

        target.startMinute =
            source["minute"] | 0;

        target.durationSeconds =
            source["durationSeconds"] |
            (15UL * 60UL);

        target.weekdays =
            source["weekdays"] | 0x7F;

        target.running = false;
        target.startedAtMs = 0;
    }

    if (!scheduler_->save())
    {
        message =
            "Programme konnten nicht gespeichert werden";
        return false;
    }

    JsonObject water =
        configuration["water"].as<JsonObject>();

    if (!water.isNull())
    {
        if (!waterManager_->setValveFlowRate(
                0,
                water["flowValve1Lpm"] |
                waterManager_->valveFlowRate(0)
            ) ||
            !waterManager_->setValveFlowRate(
                1,
                water["flowValve2Lpm"] |
                waterManager_->valveFlowRate(1)
            ) ||
            !waterManager_->setWaterPrice(
                water["priceEuroPerM3"] |
                waterManager_->waterPrice()
            ))
        {
            message =
                "Wasserparameter konnten nicht wiederhergestellt werden";
            return false;
        }
    }

    message =
        String(programs.size()) +
        " Programme und " +
        String(profiles.size()) +
        " Profile wiederhergestellt";

    return true;
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
