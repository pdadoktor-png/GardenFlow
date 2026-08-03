#include "log/LogManager.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "time/TimeManager.h"

LogManager Log;

void LogManager::begin(TimeManager* timeManager)
{
    timeManager_ = timeManager;
    clear();
}

void LogManager::setTimeManager(TimeManager& timeManager)
{
    timeManager_ = &timeManager;
}

void LogManager::add(Category category, Level level, const char* message)
{
    Entry& entry = entries_[writeIndex_];
    entry.sequence = nextSequence_++;
    entry.uptimeMs = millis();
    entry.epoch = time(nullptr);
    entry.category = category;
    entry.level = level;

    if (message == nullptr)
    {
        entry.message[0] = '\0';
    }
    else
    {
        strncpy(entry.message, message, sizeof(entry.message) - 1);
        entry.message[sizeof(entry.message) - 1] = '\0';
    }

    writeIndex_ = static_cast<uint16_t>((writeIndex_ + 1U) % CAPACITY);
    if (count_ < CAPACITY)
    {
        ++count_;
    }

    char timestamp[24];
    formatTimestamp(entry, timestamp, sizeof(timestamp));
    Serial.printf("[%s] [%s] %s\n", timestamp, categoryName(category), entry.message);
}

void LogManager::addf(Category category, Level level, const char* format, ...)
{
    char message[112];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    add(category, level, message);
}

void LogManager::info(Category category, const char* message)
{
    add(category, Level::Info, message);
}

void LogManager::warning(Category category, const char* message)
{
    add(category, Level::Warning, message);
}

void LogManager::error(Category category, const char* message)
{
    add(category, Level::Error, message);
}

uint16_t LogManager::count() const
{
    return count_;
}

bool LogManager::entryNewestFirst(uint16_t position, Entry& entry) const
{
    if (position >= count_)
    {
        return false;
    }

    const int32_t newest = static_cast<int32_t>(writeIndex_) - 1;
    int32_t index = newest - static_cast<int32_t>(position);
    while (index < 0)
    {
        index += CAPACITY;
    }

    entry = entries_[static_cast<uint16_t>(index)];
    return true;
}

void LogManager::clear()
{
    writeIndex_ = 0;
    count_ = 0;
    nextSequence_ = 1;
    memset(entries_, 0, sizeof(entries_));
}

const char* LogManager::categoryName(Category category)
{
    switch (category)
    {
        case Category::System: return "System";
        case Category::Wifi: return "WLAN";
        case Category::Time: return "Zeit";
        case Category::Weather: return "Wetter";
        case Category::Program: return "Programm";
        case Category::Valve: return "Ventil";
        case Category::Scheduler: return "Scheduler";
        case Category::Error: return "Fehler";
        default: return "Unbekannt";
    }
}

const char* LogManager::levelName(Level level)
{
    switch (level)
    {
        case Level::Info: return "info";
        case Level::Warning: return "warning";
        case Level::Error: return "error";
        default: return "info";
    }
}

void LogManager::formatTimestamp(const Entry& entry, char* buffer, size_t size)
{
    if (buffer == nullptr || size == 0)
    {
        return;
    }

    if (entry.epoch > 1700000000)
    {
        struct tm local = {};
        localtime_r(&entry.epoch, &local);
        strftime(buffer, size, "%d.%m.%Y %H:%M:%S", &local);
    }
    else
    {
        const uint32_t seconds = entry.uptimeMs / 1000UL;
        snprintf(
            buffer,
            size,
            "+%02lu:%02lu:%02lu",
            static_cast<unsigned long>(seconds / 3600UL),
            static_cast<unsigned long>((seconds / 60UL) % 60UL),
            static_cast<unsigned long>(seconds % 60UL)
        );
    }
}
