#pragma once

#include <Arduino.h>
#include <Preferences.h>

class TimeManager;
class SettingsManager;

class WeatherManager
{
public:
    void begin(TimeManager& timeManager, SettingsManager& settingsManager);
    void update();
    bool refreshNow();

    bool isConfigured() const;
    bool isValid() const;
    bool isUpdating() const;
    bool automaticPauseActive() const;
    bool automaticPauseEnabled() const;

    float temperatureC() const;
    uint8_t humidityPercent() const;
    float rainMmNext24Hours() const;
    uint8_t maxRainProbabilityPercent() const;
    const String& description() const;
    const String& lastError() const;
    time_t lastUpdateEpoch() const;

    float rainLimitMm() const;
    uint8_t probabilityLimitPercent() const;
    void setAutomaticPauseEnabled(bool enabled);
    void setRainLimitMm(float value);
    void setProbabilityLimitPercent(uint8_t value);

private:
    TimeManager* timeManager_ = nullptr;
    SettingsManager* settingsManager_ = nullptr;
    Preferences preferences_;
    uint32_t lastAttemptMs_ = 0;
    time_t lastUpdateEpoch_ = 0;
    bool valid_ = false;
    bool updating_ = false;
    bool automaticPauseEnabled_ = true;
    float rainLimitMm_ = 3.0f;
    uint8_t probabilityLimitPercent_ = 70;
    float temperatureC_ = 0.0f;
    uint8_t humidityPercent_ = 0;
    float rainMmNext24Hours_ = 0.0f;
    uint8_t maxRainProbabilityPercent_ = 0;
    String description_ = "Keine Wetterdaten";
    String lastError_;

    bool shouldAttemptUpdate() const;
    bool fetchForecast();
    void loadSettings();
    void saveSettings();
};
