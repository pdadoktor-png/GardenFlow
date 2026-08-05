#pragma once

#include <Arduino.h>

class Scheduler;
class ValveManager;
class TimeManager;
class WeatherManager;
class SmartControlManager;
class WaterManager;

class RuntimeManager
{
public:
    void begin(
        Scheduler& scheduler,
        ValveManager& valveManager,
        TimeManager& timeManager
    );

    void update();

    void setWeatherManager(
        WeatherManager& weatherManager
    );

    void setSmartControlManager(
        SmartControlManager&
            smartControlManager
    );

    void setWaterManager(
        WaterManager& waterManager
    );

    bool startProgram(
        uint8_t programIndex,
        bool automatic = false
    );

    bool stop();

    bool isRunning() const;
    bool isProgramRunning(
        uint8_t programIndex
    ) const;

    bool isAutomaticRun() const;

    int16_t runningProgramIndex() const;
    uint32_t remainingSeconds() const;
    uint32_t durationSeconds() const;
    uint8_t runningValveIndex() const;
    int16_t nextProgramIndex() const;

private:
    Scheduler* scheduler_ = nullptr;
    ValveManager* valveManager_ = nullptr;
    TimeManager* timeManager_ = nullptr;
    WeatherManager* weatherManager_ = nullptr;
    SmartControlManager*
        smartControlManager_ = nullptr;
    WaterManager* waterManager_ = nullptr;

    int16_t runningProgramIndex_ = -1;
    uint32_t startedAtMs_ = 0;
    uint32_t durationSeconds_ = 0;
    uint8_t valveIndex_ = 0;

    bool automaticRun_ = false;

    int32_t lastCheckedDayKey_ = -1;
    int16_t lastCheckedMinute_ = -1;

    uint32_t lastStartedProgramId_ = 0;
    int32_t lastStartedDayKey_ = -1;
    int16_t lastStartedMinute_ = -1;

    void clearState();
    bool allValvesIdleAndClosed() const;
    void checkAutomaticStart();

    uint32_t elapsedSeconds() const;
    void recordWaterUsage();
};
