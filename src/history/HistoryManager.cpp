#include "history/HistoryManager.h"

#include <LittleFS.h>
#include <cstring>
#include <time.h>

#include "advisor/AdvisorEngine.h"
#include "smart/SmartControlManager.h"
#include "time/TimeManager.h"
#include "water/WaterManager.h"

namespace
{
constexpr const char* FIRMWARE_VERSION = "0.39.3a";
}

bool HistoryManager::begin(
    TimeManager& timeManager,
    WaterManager& waterManager,
    SmartControlManager& smartControlManager,
    AdvisorEngine& advisorEngine)
{
    timeManager_ = &timeManager;
    waterManager_ = &waterManager;
    smartControlManager_ = &smartControlManager;
    advisorEngine_ = &advisorEngine;

    ready_ = openStorage();

    if (ready_)
    {
        Serial.printf(
            "HistoryManager bereit: %u/%u Einträge\n",
            static_cast<unsigned>(header_.entryCount),
            static_cast<unsigned>(MAX_ENTRIES)
        );
    }
    else
    {
        Serial.println("HistoryManager: LittleFS nicht verfügbar");
    }

    return ready_;
}

bool HistoryManager::isReady() const
{
    return ready_;
}

uint16_t HistoryManager::count() const
{
    return ready_ ? header_.entryCount : 0;
}

bool HistoryManager::recordStart(
    uint32_t programId,
    uint8_t valveIndex,
    uint8_t profileId,
    uint32_t plannedSeconds,
    bool automatic)
{
    HistoryEntry entry;

    fillCommon(
        entry,
        programId,
        valveIndex,
        profileId,
        plannedSeconds,
        automatic,
        "start",
        automatic ? "automatic" : "manual"
    );

    return append(entry);
}

bool HistoryManager::recordStop(
    uint32_t programId,
    uint8_t valveIndex,
    uint8_t profileId,
    uint32_t plannedSeconds,
    uint32_t actualSeconds,
    bool automatic,
    bool aborted)
{
    HistoryEntry entry;

    fillCommon(
        entry,
        programId,
        valveIndex,
        profileId,
        plannedSeconds,
        automatic,
        "stop",
        aborted ? "user_stop" : "completed"
    );

    entry.actualSeconds = actualSeconds;

    if (waterManager_ != nullptr)
    {
        entry.liters =
            waterManager_->valveFlowRate(valveIndex) *
            (static_cast<float>(actualSeconds) / 60.0f);

        entry.costEuro =
            (entry.liters / 1000.0f) *
            waterManager_->waterPrice();
    }

    return append(entry);
}

bool HistoryManager::recordSkipped(
    uint32_t programId,
    uint8_t valveIndex,
    uint8_t profileId,
    uint32_t plannedSeconds,
    const char* reason)
{
    HistoryEntry entry;

    fillCommon(
        entry,
        programId,
        valveIndex,
        profileId,
        plannedSeconds,
        true,
        "skipped",
        reason
    );

    return append(entry);
}

bool HistoryManager::openStorage()
{
    if (!LittleFS.begin(true))
    {
        return false;
    }

    if (!LittleFS.exists(HISTORY_FILE))
    {
        return initializeStorage();
    }

    if (!loadHeader())
    {
        Serial.println(
            "HistoryManager: Speicher ungültig, wird neu angelegt"
        );

        LittleFS.remove(HISTORY_FILE);
        return initializeStorage();
    }

    return true;
}

bool HistoryManager::loadHeader()
{
    File file = LittleFS.open(HISTORY_FILE, "r");

    if (!file)
    {
        return false;
    }

    const size_t read =
        file.read(
            reinterpret_cast<uint8_t*>(&header_),
            sizeof(header_)
        );

    file.close();

    if (read != sizeof(header_) ||
        header_.magic != STORAGE_MAGIC ||
        header_.version != STORAGE_VERSION ||
        header_.writeIndex >= MAX_ENTRIES ||
        header_.entryCount > MAX_ENTRIES ||
        header_.nextEventId == 0)
    {
        return false;
    }

    return true;
}

bool HistoryManager::initializeStorage()
{
    header_ = StorageHeader();
    header_.magic = STORAGE_MAGIC;
    header_.version = STORAGE_VERSION;
    header_.nextEventId = 1;

    File file = LittleFS.open(HISTORY_FILE, "w");

    if (!file)
    {
        return false;
    }

    const size_t written =
        file.write(
            reinterpret_cast<const uint8_t*>(&header_),
            sizeof(header_)
        );

    file.flush();
    file.close();

    return written == sizeof(header_);
}

bool HistoryManager::saveHeader()
{
    File file = LittleFS.open(HISTORY_FILE, "r+");

    if (!file)
    {
        return false;
    }

    if (!file.seek(0, SeekSet))
    {
        file.close();
        return false;
    }

    const size_t written =
        file.write(
            reinterpret_cast<const uint8_t*>(&header_),
            sizeof(header_)
        );

    file.flush();
    file.close();

    return written == sizeof(header_);
}

bool HistoryManager::append(
    HistoryEntry& entry)
{
    if (!ready_)
    {
        return false;
    }

    entry.eventId =
        header_.nextEventId++;

    if (header_.nextEventId == 0)
    {
        header_.nextEventId = 1;
    }

    const size_t position =
        sizeof(StorageHeader) +
        static_cast<size_t>(header_.writeIndex) *
        sizeof(HistoryEntry);

    File file = LittleFS.open(HISTORY_FILE, "r+");

    if (!file)
    {
        return false;
    }

    if (!file.seek(position, SeekSet))
    {
        file.close();
        return false;
    }

    const size_t written =
        file.write(
            reinterpret_cast<const uint8_t*>(&entry),
            sizeof(entry)
        );

    file.flush();
    file.close();

    if (written != sizeof(entry))
    {
        return false;
    }

    header_.writeIndex =
        static_cast<uint16_t>(
            (header_.writeIndex + 1U) %
            MAX_ENTRIES
        );

    if (header_.entryCount < MAX_ENTRIES)
    {
        ++header_.entryCount;
    }

    if (!saveHeader())
    {
        return false;
    }

    Serial.printf(
        "History #%lu: %s Programm %lu, Ventil %u, %s\n",
        static_cast<unsigned long>(entry.eventId),
        entry.event,
        static_cast<unsigned long>(entry.programId),
        static_cast<unsigned>(entry.valveIndex + 1U),
        entry.reason
    );

    return true;
}

void HistoryManager::fillCommon(
    HistoryEntry& entry,
    uint32_t programId,
    uint8_t valveIndex,
    uint8_t profileId,
    uint32_t plannedSeconds,
    bool automatic,
    const char* event,
    const char* reason)
{
    entry.timestamp = currentTimestamp();
    entry.programId = programId;
    entry.valveIndex = valveIndex;
    entry.profileId = profileId;
    entry.plannedSeconds = plannedSeconds;
    entry.automatic = automatic ? 1U : 0U;
    entry.advisorPercent = currentAdvisorPercent();
    entry.seasonPercent = currentSeasonPercent();

    copyText(
        entry.event,
        sizeof(entry.event),
        event
    );

    copyText(
        entry.reason,
        sizeof(entry.reason),
        reason
    );

    copyText(
        entry.firmware,
        sizeof(entry.firmware),
        FIRMWARE_VERSION
    );
}

int64_t HistoryManager::currentTimestamp() const
{
    if (timeManager_ == nullptr ||
        !timeManager_->isValid())
    {
        return 0;
    }

    return static_cast<int64_t>(time(nullptr));
}

int16_t HistoryManager::currentAdvisorPercent() const
{
    if (advisorEngine_ == nullptr)
    {
        return 0;
    }

    return advisorEngine_->
        recommendation().
        adjustmentPercent;
}

uint16_t HistoryManager::currentSeasonPercent() const
{
    if (smartControlManager_ == nullptr)
    {
        return 100;
    }

    return smartControlManager_->seasonPercent();
}

void HistoryManager::copyText(
    char* destination,
    size_t destinationSize,
    const char* source)
{
    if (destination == nullptr ||
        destinationSize == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    std::strncpy(
        destination,
        source,
        destinationSize - 1
    );

    destination[destinationSize - 1] = '\0';
}
