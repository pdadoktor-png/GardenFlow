#pragma once

#include <Arduino.h>

class TimeManager;
class SettingsManager;

class SeasonManager
{
public:
    void begin(
        TimeManager& timeManager,
        SettingsManager& settingsManager
    );

    void update();

    bool isValid() const;
    uint8_t seasonPercent() const;
    float dayLengthHours() const;
    uint16_t sunriseMinutes() const;
    uint16_t sunsetMinutes() const;
    const String& seasonName() const;
    const String& explanation() const;

private:
    TimeManager* timeManager_ = nullptr;
    SettingsManager* settingsManager_ = nullptr;

    bool valid_ = false;
    uint8_t seasonPercent_ = 100;
    float dayLengthHours_ = 12.0f;
    uint16_t sunriseMinutes_ = 360;
    uint16_t sunsetMinutes_ = 1080;
    String seasonName_ = "Unbekannt";
    String explanation_ = "Warte auf gültige Zeit und Standortdaten.";

    int32_t lastDayKey_ = -1;

    void calculate(const struct tm& local);
    static float interpolateFactor(float hours);
    static String formatMinutes(uint16_t minutes);
};
