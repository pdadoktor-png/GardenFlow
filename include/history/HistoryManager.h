#pragma once

#include <Arduino.h>
#include <FS.h>

class TimeManager;
class WaterManager;
class SmartControlManager;
class AdvisorEngine;

class HistoryManager
{
public:
    static constexpr uint16_t MAX_ENTRIES = 1000;

    struct HistoryEntry
    {
        uint32_t eventId = 0;
        int64_t timestamp = 0;
        uint32_t programId = 0;

        uint32_t plannedSeconds = 0;
        uint32_t actualSeconds = 0;

        float liters = 0.0f;
        float costEuro = 0.0f;

        int16_t advisorPercent = 0;
        uint16_t seasonPercent = 100;

        uint8_t valveIndex = 0;
        uint8_t profileId = 0;
        uint8_t automatic = 0;

        char event[16] = "";
        char reason[24] = "";
        char firmware[12] = "";
    };

    bool begin(
        TimeManager& timeManager,
        WaterManager& waterManager,
        SmartControlManager& smartControlManager,
        AdvisorEngine& advisorEngine
    );

    bool isReady() const;
    uint16_t count() const;

    bool readNewest(
        uint16_t newestIndex,
        HistoryEntry& entry
    ) const;

    bool recordStart(
        uint32_t programId,
        uint8_t valveIndex,
        uint8_t profileId,
        uint32_t plannedSeconds,
        bool automatic
    );

    bool recordStop(
        uint32_t programId,
        uint8_t valveIndex,
        uint8_t profileId,
        uint32_t plannedSeconds,
        uint32_t actualSeconds,
        bool automatic,
        bool aborted
    );

    bool recordSkipped(
        uint32_t programId,
        uint8_t valveIndex,
        uint8_t profileId,
        uint32_t plannedSeconds,
        const char* reason
    );

private:
    struct StorageHeader
    {
        uint32_t magic = 0;
        uint16_t version = 0;
        uint16_t writeIndex = 0;
        uint16_t entryCount = 0;
        uint16_t reserved = 0;
        uint32_t nextEventId = 1;
    };

    static constexpr uint32_t STORAGE_MAGIC = 0x47464831UL; // "GFH1"
    static constexpr uint16_t STORAGE_VERSION = 1;
    static constexpr const char* HISTORY_FILE = "/history.bin";

    TimeManager* timeManager_ = nullptr;
    WaterManager* waterManager_ = nullptr;
    SmartControlManager* smartControlManager_ = nullptr;
    AdvisorEngine* advisorEngine_ = nullptr;

    StorageHeader header_;
    bool ready_ = false;

    bool openStorage();
    bool loadHeader();
    bool initializeStorage();
    bool saveHeader();

    bool append(HistoryEntry& entry);

    void fillCommon(
        HistoryEntry& entry,
        uint32_t programId,
        uint8_t valveIndex,
        uint8_t profileId,
        uint32_t plannedSeconds,
        bool automatic,
        const char* event,
        const char* reason
    );

    int64_t currentTimestamp() const;
    int16_t currentAdvisorPercent() const;
    uint16_t currentSeasonPercent() const;

    static void copyText(
        char* destination,
        size_t destinationSize,
        const char* source
    );
};
