#pragma once

#include <Arduino.h>

class GardenManager
{
public:
    static constexpr uint8_t MAX_ZONES = 24;

    struct Zone
    {
        uint32_t id = 0;
        String name = "";
        uint8_t profileId = 0;
        uint8_t valve = 0;
        int16_t programIndex = -1;
        String shape = "rect";
        float x = 10.0f;
        float y = 10.0f;
        float width = 24.0f;
        float height = 18.0f;
        String color = "#2d7645";
    };

    bool begin();

    bool isReady() const;
    uint8_t count() const;

    const Zone& zone(uint8_t index) const;

    bool addZone(const Zone& zone);
    bool updateZone(uint8_t index, const Zone& zone);
    bool removeZone(uint8_t index);
    bool reset();

    bool load();
    bool save() const;

    String exportJson() const;
    bool importJson(const String& json, String& message);

private:
    static constexpr const char* FILE_PATH = "/garden.json";
    static constexpr uint8_t FORMAT_VERSION = 1;

    Zone zones_[MAX_ZONES];
    uint8_t count_ = 0;
    bool ready_ = false;

    bool parseJson(const String& json, String& message);
    static bool validColor(const String& color);
    static void normalizeZone(Zone& zone);
};
