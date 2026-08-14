#include "garden/GardenManager.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

bool GardenManager::begin()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("GardenManager: LittleFS konnte nicht gestartet werden");
        ready_ = false;
        return false;
    }

    ready_ = true;

    if (!LittleFS.exists(FILE_PATH))
    {
        count_ = 0;

        if (!save())
        {
            Serial.println("GardenManager: garden.json konnte nicht angelegt werden");
            ready_ = false;
            return false;
        }

        Serial.println("GardenManager: neue /garden.json angelegt");
        return true;
    }

    if (!load())
    {
        Serial.println("GardenManager: /garden.json ungültig, Sicherung bleibt unangetastet");
        ready_ = false;
        return false;
    }

    Serial.printf(
        "GardenManager bereit: %u Zone(n) aus /garden.json\n",
        static_cast<unsigned>(count_)
    );

    return true;
}

bool GardenManager::isReady() const
{
    return ready_;
}

uint8_t GardenManager::count() const
{
    return count_;
}

const GardenManager::Zone& GardenManager::zone(uint8_t index) const
{
    static Zone empty;

    if (index >= count_)
    {
        return empty;
    }

    return zones_[index];
}

bool GardenManager::addZone(const Zone& source)
{
    if (!ready_ || count_ >= MAX_ZONES)
    {
        return false;
    }

    Zone value = source;
    normalizeZone(value);

    if (value.id == 0)
    {
        uint32_t maxId = 0;

        for (uint8_t i = 0; i < count_; ++i)
        {
            if (zones_[i].id > maxId)
            {
                maxId = zones_[i].id;
            }
        }

        value.id = maxId + 1U;
    }

    zones_[count_++] = value;
    return save();
}

bool GardenManager::updateZone(uint8_t index, const Zone& source)
{
    if (!ready_ || index >= count_)
    {
        return false;
    }

    Zone value = source;
    normalizeZone(value);

    if (value.id == 0)
    {
        value.id = zones_[index].id;
    }

    zones_[index] = value;
    return save();
}

bool GardenManager::removeZone(uint8_t index)
{
    if (!ready_ || index >= count_)
    {
        return false;
    }

    for (uint8_t i = index; i + 1U < count_; ++i)
    {
        zones_[i] = zones_[i + 1U];
    }

    --count_;
    return save();
}

bool GardenManager::reset()
{
    if (!ready_)
    {
        return false;
    }

    count_ = 0;
    return save();
}

bool GardenManager::load()
{
    if (!ready_)
    {
        return false;
    }

    File file = LittleFS.open(FILE_PATH, "r");

    if (!file)
    {
        return false;
    }

    String json;
    json.reserve(file.size() + 1U);

    while (file.available())
    {
        json += static_cast<char>(file.read());
    }

    file.close();

    String message;
    return parseJson(json, message);
}

bool GardenManager::save() const
{
    if (!ready_)
    {
        return false;
    }

    File file = LittleFS.open(FILE_PATH, "w");

    if (!file)
    {
        return false;
    }

    const String json = exportJson();
    const size_t written = file.print(json);

    file.flush();
    file.close();

    return written == json.length();
}

String GardenManager::exportJson() const
{
    JsonDocument document;

    document["version"] = FORMAT_VERSION;
    JsonArray zones = document["zones"].to<JsonArray>();

    for (uint8_t i = 0; i < count_; ++i)
    {
        const Zone& item = zones_[i];
        JsonObject zone = zones.add<JsonObject>();

        zone["id"] = item.id;
        zone["name"] = item.name;
        zone["profile"] = item.profileId;
        zone["valve"] = item.valve;
        zone["program"] = item.programIndex;
        zone["shape"] = item.shape;
        zone["x"] = item.x;
        zone["y"] = item.y;
        zone["width"] = item.width;
        zone["height"] = item.height;
        zone["color"] = item.color;
    }

    String json;
    serializeJson(document, json);
    return json;
}

bool GardenManager::importJson(const String& json, String& message)
{
    if (!ready_)
    {
        message = "GardenManager nicht bereit";
        return false;
    }

    Zone backup[MAX_ZONES];
    const uint8_t backupCount = count_;

    for (uint8_t i = 0; i < backupCount; ++i)
    {
        backup[i] = zones_[i];
    }

    if (!parseJson(json, message))
    {
        count_ = backupCount;

        for (uint8_t i = 0; i < backupCount; ++i)
        {
            zones_[i] = backup[i];
        }

        return false;
    }

    if (!save())
    {
        count_ = backupCount;

        for (uint8_t i = 0; i < backupCount; ++i)
        {
            zones_[i] = backup[i];
        }

        message = "garden.json konnte nicht gespeichert werden";
        return false;
    }

    message = "Gartenkarte gespeichert";
    return true;
}

bool GardenManager::parseJson(const String& json, String& message)
{
    JsonDocument document;
    const DeserializationError error =
        deserializeJson(document, json);

    if (error)
    {
        message =
            String("JSON ungültig: ") +
            error.c_str();
        return false;
    }

    const uint8_t version =
        document["version"] | 0;

    if (version != FORMAT_VERSION)
    {
        message = "Nicht unterstützte Gartenkarten-Version";
        return false;
    }

    JsonArray zones = document["zones"].as<JsonArray>();

    if (zones.isNull())
    {
        message = "zones fehlt";
        return false;
    }

    if (zones.size() > MAX_ZONES)
    {
        message = "Zu viele Gartenbereiche";
        return false;
    }

    Zone parsed[MAX_ZONES];
    uint8_t parsedCount = 0;

    for (JsonObject source : zones)
    {
        Zone item;

        item.id = source["id"] | 0U;
        item.name =
            String(
                source["name"] |
                "Zone"
            );
        item.profileId =
            source["profile"] | 0;
        item.valve =
            source["valve"] | 0;
        item.programIndex =
            source["program"] | -1;
        item.shape =
            String(
                source["shape"] |
                "rect"
            );
        item.x =
            source["x"] | 10.0f;
        item.y =
            source["y"] | 10.0f;
        item.width =
            source["width"] | 24.0f;
        item.height =
            source["height"] | 18.0f;
        item.color =
            String(
                source["color"] |
                "#2d7645"
            );

        normalizeZone(item);
        parsed[parsedCount++] = item;
    }

    count_ = parsedCount;

    for (uint8_t i = 0; i < count_; ++i)
    {
        zones_[i] = parsed[i];
    }

    message = "OK";
    return true;
}

bool GardenManager::validColor(const String& color)
{
    if (color.length() != 7U ||
        color[0] != '#')
    {
        return false;
    }

    for (uint8_t i = 1; i < 7; ++i)
    {
        const char c = color[i];

        if (!isxdigit(
                static_cast<unsigned char>(c)
            ))
        {
            return false;
        }
    }

    return true;
}

void GardenManager::normalizeZone(Zone& zone)
{
    zone.profileId =
        min<uint8_t>(zone.profileId, 31U);
    zone.valve =
        min<uint8_t>(zone.valve, 1U);

    if (zone.programIndex < -1 ||
        zone.programIndex > 63)
    {
        zone.programIndex = -1;
    }

    if (zone.shape != "rect")
    {
        zone.shape = "rect";
    }

    zone.x =
        constrain(zone.x, 0.0f, 92.0f);
    zone.y =
        constrain(zone.y, 0.0f, 92.0f);
    zone.width =
        constrain(
            zone.width,
            8.0f,
            100.0f - zone.x
        );
    zone.height =
        constrain(
            zone.height,
            8.0f,
            100.0f - zone.y
        );

    if (zone.name.length() == 0)
    {
        zone.name = "Zone";
    }

    if (zone.name.length() > 24U)
    {
        zone.name.remove(24U);
    }

    if (!validColor(zone.color))
    {
        zone.color = "#2d7645";
    }
}
