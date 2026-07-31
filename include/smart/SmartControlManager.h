#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

class SmartControlManager
{
public:
    void begin();

    bool vacationEnabled() const;
    uint32_t vacationStartDate() const;
    uint32_t vacationEndDate() const;
    uint8_t vacationIntervalDays() const;
    uint8_t vacationPercent() const;
    uint8_t seasonPercent() const;

    void setVacationEnabled(bool enabled);
    void setVacationDates(uint32_t startDate, uint32_t endDate);
    void setVacationIntervalDays(uint8_t days);
    void setVacationPercent(uint8_t percent);
    void setSeasonPercent(uint8_t percent);

    bool vacationActive(const struct tm& localTime) const;
    bool automaticRunAllowed(const struct tm& localTime) const;
    uint16_t automaticDurationPercent(const struct tm& localTime) const;

    static uint32_t dateKey(const struct tm& localTime);
    static bool validDateKey(uint32_t value);

private:
    Preferences preferences_;

    bool vacationEnabled_ = false;
    uint32_t vacationStartDate_ = 0;
    uint32_t vacationEndDate_ = 0;
    uint8_t vacationIntervalDays_ = 1;
    uint8_t vacationPercent_ = 100;
    uint8_t seasonPercent_ = 100;

    void load();
    void save();
    static time_t dateKeyToEpoch(uint32_t value);
};
