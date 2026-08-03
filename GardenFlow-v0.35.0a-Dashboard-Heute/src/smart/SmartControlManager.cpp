#include "smart/SmartControlManager.h"

namespace
{
    constexpr uint8_t MIN_PERCENT = 10;
    constexpr uint8_t MAX_SEASON_PERCENT = 200;
}

void SmartControlManager::begin()
{
    preferences_.begin("smartctrl", false);
    load();
    Serial.println("SmartControlManager initialisiert");
}

bool SmartControlManager::vacationEnabled() const { return vacationEnabled_; }
uint32_t SmartControlManager::vacationStartDate() const { return vacationStartDate_; }
uint32_t SmartControlManager::vacationEndDate() const { return vacationEndDate_; }
uint8_t SmartControlManager::vacationIntervalDays() const { return vacationIntervalDays_; }
uint8_t SmartControlManager::vacationPercent() const { return vacationPercent_; }
uint8_t SmartControlManager::seasonPercent() const { return seasonPercent_; }

void SmartControlManager::setVacationEnabled(bool enabled)
{
    vacationEnabled_ = enabled;
    save();
}

void SmartControlManager::setVacationDates(uint32_t startDate, uint32_t endDate)
{
    if (validDateKey(startDate) && validDateKey(endDate) && endDate >= startDate)
    {
        vacationStartDate_ = startDate;
        vacationEndDate_ = endDate;
        save();
    }
}

void SmartControlManager::setVacationIntervalDays(uint8_t days)
{
    vacationIntervalDays_ = constrain(days, static_cast<uint8_t>(1), static_cast<uint8_t>(7));
    save();
}

void SmartControlManager::setVacationPercent(uint8_t percent)
{
    vacationPercent_ = constrain(percent, MIN_PERCENT, static_cast<uint8_t>(100));
    save();
}

void SmartControlManager::setSeasonPercent(uint8_t percent)
{
    seasonPercent_ = constrain(percent, MIN_PERCENT, MAX_SEASON_PERCENT);
    save();
}

bool SmartControlManager::vacationActive(const struct tm& localTime) const
{
    if (!vacationEnabled_ || !validDateKey(vacationStartDate_) || !validDateKey(vacationEndDate_))
    {
        return false;
    }

    const uint32_t today = dateKey(localTime);
    return today >= vacationStartDate_ && today <= vacationEndDate_;
}

bool SmartControlManager::automaticRunAllowed(const struct tm& localTime) const
{
    if (!vacationActive(localTime) || vacationIntervalDays_ <= 1)
    {
        return true;
    }

    const time_t startEpoch = dateKeyToEpoch(vacationStartDate_);
    const time_t todayEpoch = dateKeyToEpoch(dateKey(localTime));

    if (startEpoch <= 0 || todayEpoch < startEpoch)
    {
        return true;
    }

    const uint32_t daysSinceStart =
        static_cast<uint32_t>((todayEpoch - startEpoch) / 86400L);

    return (daysSinceStart % vacationIntervalDays_) == 0;
}

uint16_t SmartControlManager::automaticDurationPercent(const struct tm& localTime) const
{
    uint16_t result = seasonPercent_;

    if (vacationActive(localTime))
    {
        result = static_cast<uint16_t>(
            (static_cast<uint32_t>(result) * vacationPercent_) / 100UL
        );
    }

    return constrain(
        result,
        static_cast<uint16_t>(MIN_PERCENT),
        static_cast<uint16_t>(MAX_SEASON_PERCENT)
    );
}

uint32_t SmartControlManager::dateKey(const struct tm& localTime)
{
    return static_cast<uint32_t>(localTime.tm_year + 1900) * 10000UL +
           static_cast<uint32_t>(localTime.tm_mon + 1) * 100UL +
           static_cast<uint32_t>(localTime.tm_mday);
}

bool SmartControlManager::validDateKey(uint32_t value)
{
    const uint32_t year = value / 10000UL;
    const uint32_t month = (value / 100UL) % 100UL;
    const uint32_t day = value % 100UL;

    return year >= 2024UL && year <= 2099UL &&
           month >= 1UL && month <= 12UL &&
           day >= 1UL && day <= 31UL;
}

void SmartControlManager::load()
{
    vacationEnabled_ = preferences_.getBool("vacEnabled", false);
    vacationStartDate_ = preferences_.getUInt("vacStart", 0);
    vacationEndDate_ = preferences_.getUInt("vacEnd", 0);
    vacationIntervalDays_ = constrain(
        preferences_.getUChar("vacEvery", 1),
        static_cast<uint8_t>(1),
        static_cast<uint8_t>(7)
    );
    vacationPercent_ = constrain(
        preferences_.getUChar("vacPercent", 100),
        MIN_PERCENT,
        static_cast<uint8_t>(100)
    );
    seasonPercent_ = constrain(
        preferences_.getUChar("season", 100),
        MIN_PERCENT,
        MAX_SEASON_PERCENT
    );
}

void SmartControlManager::save()
{
    preferences_.putBool("vacEnabled", vacationEnabled_);
    preferences_.putUInt("vacStart", vacationStartDate_);
    preferences_.putUInt("vacEnd", vacationEndDate_);
    preferences_.putUChar("vacEvery", vacationIntervalDays_);
    preferences_.putUChar("vacPercent", vacationPercent_);
    preferences_.putUChar("season", seasonPercent_);
}

time_t SmartControlManager::dateKeyToEpoch(uint32_t value)
{
    if (!validDateKey(value))
    {
        return 0;
    }

    struct tm local = {};
    local.tm_year = static_cast<int>(value / 10000UL) - 1900;
    local.tm_mon = static_cast<int>((value / 100UL) % 100UL) - 1;
    local.tm_mday = static_cast<int>(value % 100UL);
    local.tm_hour = 12;
    local.tm_isdst = -1;
    return mktime(&local);
}
