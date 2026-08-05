#pragma once

#include <Arduino.h>
#include <Preferences.h>

class TimeManager;

struct WaterStatistics
{
    float todayLiters = 0.0f;
    float weekLiters = 0.0f;
    float monthLiters = 0.0f;
    float yearLiters = 0.0f;

    float savedLiters = 0.0f;

    float todayCost = 0.0f;
    float monthCost = 0.0f;
    float yearCost = 0.0f;
};

class WaterManager
{
public:
    static constexpr uint8_t VALVE_COUNT = 2;

    void begin(TimeManager& timeManager);
    void update();

    bool setValveFlowRate(
        uint8_t valve,
        float litersPerMinute
    );

    bool setWaterPrice(
        float euroPerCubicMeter
    );

    void addRuntime(
        uint8_t valve,
        uint32_t runtimeSeconds
    );

    void addSavedWater(
        uint8_t valve,
        uint32_t runtimeSeconds
    );

    const WaterStatistics&
        statistics() const;

    float valveFlowRate(
        uint8_t valve
    ) const;

    float waterPrice() const;

    void resetAll();

private:
    Preferences preferences_;
    TimeManager* timeManager_ = nullptr;

    float valveFlow_[VALVE_COUNT] =
    {
        12.0f,
        12.0f
    };

    float waterPriceEuroPerCubicMeter_ =
        7.10f;

    WaterStatistics stats_;

    int32_t currentDayKey_ = -1;
    int32_t currentWeekKey_ = -1;
    int32_t currentMonthKey_ = -1;
    int32_t currentYearKey_ = -1;

    bool validValve(
        uint8_t valve
    ) const;

    float runtimeToLiters(
        uint8_t valve,
        uint32_t runtimeSeconds
    ) const;

    void load();
    void save();
    void updateCosts();
    void updatePeriods();

    static int32_t dayKey(
        const struct tm& local
    );

    static int32_t weekKey(
        const struct tm& local
    );

    static int32_t monthKey(
        const struct tm& local
    );

    static int32_t yearKey(
        const struct tm& local
    );
};
