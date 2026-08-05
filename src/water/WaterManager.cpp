#include "water/WaterManager.h"

#include <cmath>

#include "time/TimeManager.h"

namespace
{
constexpr float MIN_FLOW_LPM = 0.0f;
constexpr float MAX_FLOW_LPM = 250.0f;
constexpr float MIN_PRICE_EURO_M3 = 0.0f;
constexpr float MAX_PRICE_EURO_M3 = 100.0f;
constexpr float LITERS_PER_CUBIC_METER = 1000.0f;
}

void WaterManager::begin(
    TimeManager& timeManager)
{
    timeManager_ = &timeManager;

    preferences_.begin(
        "waterstats",
        false
    );

    load();
    updateCosts();
    updatePeriods();

    Serial.printf(
        "WaterManager initialisiert: "
        "V1 %.1f l/min, V2 %.1f l/min, "
        "Wasserpreis %.2f EUR/m3\n",
        valveFlow_[0],
        valveFlow_[1],
        waterPriceEuroPerCubicMeter_
    );
}

void WaterManager::update()
{
    updatePeriods();
}

bool WaterManager::setValveFlowRate(
    uint8_t valve,
    float litersPerMinute)
{
    if (!validValve(valve) ||
        !std::isfinite(litersPerMinute) ||
        litersPerMinute < MIN_FLOW_LPM ||
        litersPerMinute > MAX_FLOW_LPM)
    {
        return false;
    }

    valveFlow_[valve] =
        litersPerMinute;

    save();
    return true;
}

bool WaterManager::setWaterPrice(
    float euroPerCubicMeter)
{
    if (!std::isfinite(
            euroPerCubicMeter
        ) ||
        euroPerCubicMeter <
            MIN_PRICE_EURO_M3 ||
        euroPerCubicMeter >
            MAX_PRICE_EURO_M3)
    {
        return false;
    }

    waterPriceEuroPerCubicMeter_ =
        euroPerCubicMeter;

    updateCosts();
    save();
    return true;
}

void WaterManager::addRuntime(
    uint8_t valve,
    uint32_t runtimeSeconds)
{
    const float liters =
        runtimeToLiters(
            valve,
            runtimeSeconds
        );

    if (liters <= 0.0f)
    {
        return;
    }

    updatePeriods();

    stats_.todayLiters += liters;
    stats_.weekLiters += liters;
    stats_.monthLiters += liters;
    stats_.yearLiters += liters;

    updateCosts();
    save();

    Serial.printf(
        "Wasserverbrauch: Ventil %u, "
        "%lu s, %.2f Liter\n",
        static_cast<unsigned>(
            valve + 1
        ),
        static_cast<unsigned long>(
            runtimeSeconds
        ),
        liters
    );
}

void WaterManager::addSavedWater(
    uint8_t valve,
    uint32_t runtimeSeconds)
{
    const float liters =
        runtimeToLiters(
            valve,
            runtimeSeconds
        );

    if (liters <= 0.0f)
    {
        return;
    }

    stats_.savedLiters += liters;
    save();

    Serial.printf(
        "Wasser eingespart: "
        "Ventil %u, %.2f Liter\n",
        static_cast<unsigned>(
            valve + 1
        ),
        liters
    );
}

const WaterStatistics&
WaterManager::statistics() const
{
    return stats_;
}

float WaterManager::valveFlowRate(
    uint8_t valve) const
{
    if (!validValve(valve))
    {
        return 0.0f;
    }

    return valveFlow_[valve];
}

float WaterManager::waterPrice() const
{
    return
        waterPriceEuroPerCubicMeter_;
}

void WaterManager::resetAll()
{
    stats_ = WaterStatistics();
    updateCosts();
    save();
}

bool WaterManager::validValve(
    uint8_t valve) const
{
    return valve < VALVE_COUNT;
}

float WaterManager::runtimeToLiters(
    uint8_t valve,
    uint32_t runtimeSeconds) const
{
    if (!validValve(valve) ||
        runtimeSeconds == 0)
    {
        return 0.0f;
    }

    return valveFlow_[valve] *
        (
            static_cast<float>(
                runtimeSeconds
            ) / 60.0f
        );
}

void WaterManager::load()
{
    valveFlow_[0] =
        preferences_.getFloat(
            "flow1",
            12.0f
        );

    valveFlow_[1] =
        preferences_.getFloat(
            "flow2",
            12.0f
        );

    waterPriceEuroPerCubicMeter_ =
        preferences_.getFloat(
            "price",
            7.10f
        );

    stats_.todayLiters =
        preferences_.getFloat(
            "todayL",
            0.0f
        );

    stats_.weekLiters =
        preferences_.getFloat(
            "weekL",
            0.0f
        );

    stats_.monthLiters =
        preferences_.getFloat(
            "monthL",
            0.0f
        );

    stats_.yearLiters =
        preferences_.getFloat(
            "yearL",
            0.0f
        );

    stats_.savedLiters =
        preferences_.getFloat(
            "savedL",
            0.0f
        );

    currentDayKey_ =
        preferences_.getInt(
            "dayKey",
            -1
        );

    currentWeekKey_ =
        preferences_.getInt(
            "weekKey",
            -1
        );

    currentMonthKey_ =
        preferences_.getInt(
            "monthKey",
            -1
        );

    currentYearKey_ =
        preferences_.getInt(
            "yearKey",
            -1
        );
}

void WaterManager::save()
{
    preferences_.putFloat(
        "flow1",
        valveFlow_[0]
    );

    preferences_.putFloat(
        "flow2",
        valveFlow_[1]
    );

    preferences_.putFloat(
        "price",
        waterPriceEuroPerCubicMeter_
    );

    preferences_.putFloat(
        "todayL",
        stats_.todayLiters
    );

    preferences_.putFloat(
        "weekL",
        stats_.weekLiters
    );

    preferences_.putFloat(
        "monthL",
        stats_.monthLiters
    );

    preferences_.putFloat(
        "yearL",
        stats_.yearLiters
    );

    preferences_.putFloat(
        "savedL",
        stats_.savedLiters
    );

    preferences_.putInt(
        "dayKey",
        currentDayKey_
    );

    preferences_.putInt(
        "weekKey",
        currentWeekKey_
    );

    preferences_.putInt(
        "monthKey",
        currentMonthKey_
    );

    preferences_.putInt(
        "yearKey",
        currentYearKey_
    );
}

void WaterManager::updateCosts()
{
    const float euroPerLiter =
        waterPriceEuroPerCubicMeter_ /
        LITERS_PER_CUBIC_METER;

    stats_.todayCost =
        stats_.todayLiters *
        euroPerLiter;

    stats_.monthCost =
        stats_.monthLiters *
        euroPerLiter;

    stats_.yearCost =
        stats_.yearLiters *
        euroPerLiter;
}

void WaterManager::updatePeriods()
{
    if (timeManager_ == nullptr ||
        !timeManager_->isValid())
    {
        return;
    }

    struct tm local = {};

    if (!timeManager_->getLocalTime(
            local
        ))
    {
        return;
    }

    const int32_t newDay =
        dayKey(local);

    const int32_t newWeek =
        weekKey(local);

    const int32_t newMonth =
        monthKey(local);

    const int32_t newYear =
        yearKey(local);

    bool changed = false;

    if (currentYearKey_ < 0)
    {
        currentDayKey_ = newDay;
        currentWeekKey_ = newWeek;
        currentMonthKey_ = newMonth;
        currentYearKey_ = newYear;
        changed = true;
    }
    else
    {
        if (newYear != currentYearKey_)
        {
            stats_.yearLiters = 0.0f;
            stats_.savedLiters = 0.0f;
            currentYearKey_ = newYear;
            changed = true;
        }

        if (newMonth != currentMonthKey_)
        {
            stats_.monthLiters = 0.0f;
            currentMonthKey_ = newMonth;
            changed = true;
        }

        if (newWeek != currentWeekKey_)
        {
            stats_.weekLiters = 0.0f;
            currentWeekKey_ = newWeek;
            changed = true;
        }

        if (newDay != currentDayKey_)
        {
            stats_.todayLiters = 0.0f;
            currentDayKey_ = newDay;
            changed = true;
        }
    }

    if (changed)
    {
        updateCosts();
        save();
    }
}

int32_t WaterManager::dayKey(
    const struct tm& local)
{
    return
        (local.tm_year + 1900) *
        1000 +
        local.tm_yday;
}

int32_t WaterManager::weekKey(
    const struct tm& local)
{
    return
        (local.tm_year + 1900) *
        100 +
        (local.tm_yday / 7);
}

int32_t WaterManager::monthKey(
    const struct tm& local)
{
    return
        (local.tm_year + 1900) *
        100 +
        local.tm_mon + 1;
}

int32_t WaterManager::yearKey(
    const struct tm& local)
{
    return local.tm_year + 1900;
}
