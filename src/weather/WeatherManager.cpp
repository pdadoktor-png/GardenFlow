#include "weather/WeatherManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "app/WeatherConfig.h"
#include "time/TimeManager.h"
#include "settings/SettingsManager.h"

void WeatherManager::begin(TimeManager& timeManager, SettingsManager& settingsManager)
{
    timeManager_ = &timeManager;
    settingsManager_ = &settingsManager;
    preferences_.begin("weather", false);
    loadSettings();

    if (isConfigured())
    {
        Serial.println("WeatherManager initialisiert");
    }
    else
    {
        Serial.println("WeatherManager deaktiviert: API-Schluessel fehlt");
    }
}

void WeatherManager::update()
{
    if (!shouldAttemptUpdate())
    {
        return;
    }

    refreshNow();
}

bool WeatherManager::refreshNow()
{
    if (!isConfigured() || timeManager_ == nullptr || !timeManager_->isWifiConnected() || updating_)
    {
        return false;
    }

    lastAttemptMs_ = millis();
    updating_ = true;
    const bool success = fetchForecast();
    updating_ = false;
    return success;
}

bool WeatherManager::isConfigured() const
{
    return WeatherConfig::API_KEY[0] != '\0';
}

bool WeatherManager::isValid() const
{
    return valid_;
}

bool WeatherManager::isUpdating() const
{
    return updating_;
}

bool WeatherManager::automaticPauseActive() const
{
    return automaticPauseEnabled_ && valid_ &&
        (rainMmNext24Hours_ >= rainLimitMm_ ||
         maxRainProbabilityPercent_ >= probabilityLimitPercent_);
}

bool WeatherManager::automaticPauseEnabled() const
{
    return automaticPauseEnabled_;
}

float WeatherManager::temperatureC() const { return temperatureC_; }
uint8_t WeatherManager::humidityPercent() const { return humidityPercent_; }
float WeatherManager::rainMmNext24Hours() const { return rainMmNext24Hours_; }
uint8_t WeatherManager::maxRainProbabilityPercent() const { return maxRainProbabilityPercent_; }
const String& WeatherManager::description() const { return description_; }
const String& WeatherManager::lastError() const { return lastError_; }
time_t WeatherManager::lastUpdateEpoch() const { return lastUpdateEpoch_; }
float WeatherManager::rainLimitMm() const { return rainLimitMm_; }
uint8_t WeatherManager::probabilityLimitPercent() const { return probabilityLimitPercent_; }

void WeatherManager::setAutomaticPauseEnabled(bool enabled)
{
    automaticPauseEnabled_ = enabled;
    saveSettings();
}

void WeatherManager::setRainLimitMm(float value)
{
    rainLimitMm_ = constrain(value, 0.1f, 100.0f);
    saveSettings();
}

void WeatherManager::setProbabilityLimitPercent(uint8_t value)
{
    probabilityLimitPercent_ = constrain(value, static_cast<uint8_t>(1), static_cast<uint8_t>(100));
    saveSettings();
}

bool WeatherManager::shouldAttemptUpdate() const
{
    if (!isConfigured() || timeManager_ == nullptr || !timeManager_->isWifiConnected() || updating_)
    {
        return false;
    }

    const uint32_t interval = valid_
        ? WeatherConfig::UPDATE_INTERVAL_MS
        : WeatherConfig::RETRY_INTERVAL_MS;

    return lastAttemptMs_ == 0 || static_cast<uint32_t>(millis() - lastAttemptMs_) >= interval;
}

bool WeatherManager::fetchForecast()
{
    String url;
    url.reserve(240);
    url = "https://api.openweathermap.org/data/2.5/forecast?lat=";
    url += String(settingsManager_ ? settingsManager_->latitude() : WeatherConfig::LATITUDE, 5);
    url += "&lon=";
    url += String(settingsManager_ ? settingsManager_->longitude() : WeatherConfig::LONGITUDE, 5);
    url += "&appid=";
    url += WeatherConfig::API_KEY;
    url += "&units=metric&lang=de";

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    if (!http.begin(client, url))
    {
        lastError_ = "HTTPS konnte nicht gestartet werden";
        Serial.printf("Wetterfehler: %s\n", lastError_.c_str());
        return false;
    }

    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK)
    {
        lastError_ = String("HTTP ") + statusCode;
        Serial.printf("Wetterfehler: %s\n", lastError_.c_str());
        http.end();
        return false;
    }

    JsonDocument filter;
    filter["list"][0]["dt"] = true;
    filter["list"][0]["main"]["temp"] = true;
    filter["list"][0]["main"]["humidity"] = true;
    filter["list"][0]["weather"][0]["description"] = true;
    filter["list"][0]["pop"] = true;
    filter["list"][0]["rain"]["3h"] = true;

    JsonDocument document;
    DeserializationError error = deserializeJson(
        document,
        http.getStream(),
        DeserializationOption::Filter(filter)
    );
    http.end();

    if (error)
    {
        lastError_ = String("JSON: ") + error.c_str();
        Serial.printf("Wetterfehler: %s\n", lastError_.c_str());
        return false;
    }

    JsonArray list = document["list"].as<JsonArray>();
    if (list.isNull() || list.size() == 0)
    {
        lastError_ = "Vorhersage ist leer";
        Serial.printf("Wetterfehler: %s\n", lastError_.c_str());
        return false;
    }

    JsonObject first = list[0];
    temperatureC_ = first["main"]["temp"] | 0.0f;
    humidityPercent_ = static_cast<uint8_t>(first["main"]["humidity"] | 0);
    description_ = first["weather"][0]["description"] | "unbekannt";

    rainMmNext24Hours_ = 0.0f;
    maxRainProbabilityPercent_ = 0;

    const time_t now = time(nullptr);
    const time_t limit = now + 24L * 60L * 60L;

    for (JsonObject item : list)
    {
        const time_t forecastEpoch = item["dt"] | 0;
        if (forecastEpoch <= 0 || forecastEpoch > limit)
        {
            continue;
        }

        const float probability = item["pop"] | 0.0f;
        const uint8_t probabilityPercent = static_cast<uint8_t>(
            constrain(static_cast<int>(probability * 100.0f + 0.5f), 0, 100)
        );
        if (probabilityPercent > maxRainProbabilityPercent_)
        {
            maxRainProbabilityPercent_ = probabilityPercent;
        }

        rainMmNext24Hours_ += item["rain"]["3h"] | 0.0f;
    }

    valid_ = true;
    lastUpdateEpoch_ = time(nullptr);
    lastError_ = "";

    Serial.printf(
        "Wetter aktualisiert: %.1f C, %u%% Feuchte, %.1f mm Regen/24h, %u%% Regenrisiko%s\n",
        temperatureC_,
        static_cast<unsigned>(humidityPercent_),
        rainMmNext24Hours_,
        static_cast<unsigned>(maxRainProbabilityPercent_),
        automaticPauseActive() ? ", Regenpause aktiv" : ""
    );

    return true;
}

void WeatherManager::loadSettings()
{
    automaticPauseEnabled_ = preferences_.getBool(
        "autoPause",
        WeatherConfig::AUTO_RAIN_PAUSE_ENABLED
    );
    rainLimitMm_ = preferences_.getFloat(
        "rainMm",
        WeatherConfig::RAIN_LIMIT_MM_24H
    );
    probabilityLimitPercent_ = preferences_.getUChar(
        "rainPop",
        WeatherConfig::RAIN_PROBABILITY_LIMIT_PERCENT
    );
}

void WeatherManager::saveSettings()
{
    preferences_.putBool("autoPause", automaticPauseEnabled_);
    preferences_.putFloat("rainMm", rainLimitMm_);
    preferences_.putUChar("rainPop", probabilityLimitPercent_);
}
