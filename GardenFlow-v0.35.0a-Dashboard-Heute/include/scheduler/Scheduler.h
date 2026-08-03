#pragma once

#include <Arduino.h>
#include <Preferences.h>

class Scheduler
{
public:
    static constexpr uint8_t MAX_PROGRAMS = 16;
    static constexpr uint8_t VALVE_COUNT = 2;
    static constexpr uint16_t MIN_DURATION_MINUTES = 1;
    static constexpr uint16_t MAX_DURATION_MINUTES = 240;
    static constexpr uint32_t INVALID_PROGRAM_ID = 0;

    enum class Weekday : uint8_t
    {
        Monday = 0,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday
    };

    struct IrrigationProgram
    {
        uint32_t id = INVALID_PROGRAM_ID;
        bool used = false;
        bool enabled = false;
        uint8_t valveIndex = 0;
        uint8_t startHour = 6;
        uint8_t startMinute = 0;
        uint32_t durationSeconds = 15UL * 60UL;
        uint8_t weekdays = 0;
        bool running = false;
        uint32_t startedAtMs = 0;
    };

    void begin();
    void update();

    IrrigationProgram& program(uint8_t index);
    const IrrigationProgram& program(uint8_t index) const;

    uint8_t programCount() const;
    uint8_t usedProgramCount() const;
    uint8_t usedProgramCountForValve(uint8_t valveIndex) const;
    bool isProgramUsed(uint8_t index) const;

    int16_t createProgram(uint8_t valveIndex);
    bool deleteProgram(uint8_t index);

    uint32_t programId(uint8_t index) const;
    int16_t findProgramIndexById(uint32_t id) const;

    bool setProgramEnabled(uint8_t index, bool enabled);
    bool setValve(uint8_t programIndex, uint8_t valveIndex);
    bool setStartTime(uint8_t programIndex, uint8_t hour, uint8_t minute);
    bool setDurationMinutes(uint8_t programIndex, uint16_t minutes);
    uint16_t durationMinutes(uint8_t programIndex) const;
    bool setWeekday(uint8_t programIndex, Weekday weekday, bool enabled);
    bool isWeekdayEnabled(uint8_t programIndex, Weekday weekday) const;

    bool save();
    bool load();
    void restoreDefaults();

private:
    static constexpr uint32_t LEGACY_STORAGE_VERSION_1 = 1;
    static constexpr uint32_t LEGACY_STORAGE_VERSION_2 = 2;
    static constexpr uint32_t STORAGE_VERSION = 3;

    struct LegacyIrrigationProgramV1
    {
        bool enabled = false;
        uint8_t valveIndex = 0;
        uint8_t startHour = 6;
        uint8_t startMinute = 0;
        uint32_t durationSeconds = 15UL * 60UL;
        uint8_t weekdays = 0;
        bool running = false;
        uint32_t startedAtMs = 0;
    };

    struct LegacyIrrigationProgramV2
    {
        uint32_t id = INVALID_PROGRAM_ID;
        bool enabled = false;
        uint8_t valveIndex = 0;
        uint8_t startHour = 6;
        uint8_t startMinute = 0;
        uint32_t durationSeconds = 15UL * 60UL;
        uint8_t weekdays = 0;
        bool running = false;
        uint32_t startedAtMs = 0;
    };

    Preferences preferences_;
    IrrigationProgram programs_[MAX_PROGRAMS];
    uint32_t nextProgramId_ = 1;

    bool loadCurrentVersion();
    bool migrateFromVersion1();
    bool migrateFromVersion2();
    void resetRuntimeState();
    uint32_t allocateProgramId();
    int16_t findFreeSlot() const;

    bool validProgramIndex(uint8_t index) const;
    bool validUsedProgramIndex(uint8_t index) const;
    bool validValveIndex(uint8_t index) const;
};
