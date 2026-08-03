#pragma once

#include <Arduino.h>
#include <time.h>

class TimeManager;

class LogManager
{
public:
    enum class Category : uint8_t
    {
        System = 0,
        Wifi,
        Time,
        Weather,
        Program,
        Valve,
        Scheduler,
        Error
    };

    enum class Level : uint8_t
    {
        Info = 0,
        Warning,
        Error
    };

    struct Entry
    {
        uint32_t sequence = 0;
        time_t epoch = 0;
        uint32_t uptimeMs = 0;
        Category category = Category::System;
        Level level = Level::Info;
        char message[112] = {};
    };

    static constexpr uint16_t CAPACITY = 120;

    void begin(TimeManager* timeManager = nullptr);
    void setTimeManager(TimeManager& timeManager);

    void add(Category category, Level level, const char* message);
    void addf(Category category, Level level, const char* format, ...);

    void info(Category category, const char* message);
    void warning(Category category, const char* message);
    void error(Category category, const char* message);

    uint16_t count() const;
    bool entryNewestFirst(uint16_t position, Entry& entry) const;
    void clear();

    static const char* categoryName(Category category);
    static const char* levelName(Level level);
    static void formatTimestamp(const Entry& entry, char* buffer, size_t size);

private:
    TimeManager* timeManager_ = nullptr;
    Entry entries_[CAPACITY];
    uint16_t writeIndex_ = 0;
    uint16_t count_ = 0;
    uint32_t nextSequence_ = 1;
};

extern LogManager Log;
