#include "profiles/GardenProfiles.h"

#include <Preferences.h>
#include <cstring>

namespace
{
    constexpr const char* NVS_NAMESPACE = "gardenprofiles";
    constexpr const char* KEY_VERSION = "version";
    constexpr const char* KEY_PROFILES = "profiles";
    constexpr uint32_t STORAGE_VERSION = 1;

    Preferences preferences;
    bool initialized = false;

    GardenProfiles::Profile profiles[
        GardenProfiles::PROFILE_COUNT
    ];

    void copyText(
        char* destination,
        size_t destinationSize,
        const String& source)
    {
        if (destination == nullptr ||
            destinationSize == 0)
        {
            return;
        }

        String clean = source;
        clean.trim();

        std::strncpy(
            destination,
            clean.c_str(),
            destinationSize - 1
        );

        destination[destinationSize - 1] = '\0';
    }

    GardenProfiles::Profile makeProfile(
        GardenProfiles::ProfileId id,
        const char* name,
        const char* symbol,
        int16_t correction,
        uint8_t temperature,
        uint8_t humidity,
        uint8_t rain,
        uint16_t minimum,
        uint16_t maximum)
    {
        GardenProfiles::Profile profile;
        profile.id = id;

        std::strncpy(
            profile.name,
            name,
            sizeof(profile.name) - 1
        );

        std::strncpy(
            profile.symbol,
            symbol,
            sizeof(profile.symbol) - 1
        );

        profile.correctionPercent = correction;
        profile.temperatureSensitivityPercent = temperature;
        profile.humiditySensitivityPercent = humidity;
        profile.rainSensitivityPercent = rain;
        profile.minimumMinutes = minimum;
        profile.maximumMinutes = maximum;

        return profile;
    }

    void applyDefaults()
    {
        profiles[0] = makeProfile(
            GardenProfiles::ProfileId::General,
            "Allgemein",
            "Garten",
            0,
            100,
            100,
            100,
            1,
            240
        );

        profiles[1] = makeProfile(
            GardenProfiles::ProfileId::Lawn,
            "Rasen",
            "Rasen",
            10,
            110,
            100,
            100,
            5,
            60
        );

        profiles[2] = makeProfile(
            GardenProfiles::ProfileId::Flowers,
            "Blumen",
            "Blume",
            0,
            100,
            105,
            110,
            3,
            45
        );

        profiles[3] = makeProfile(
            GardenProfiles::ProfileId::Vegetables,
            "Gemüse",
            "Gemüse",
            20,
            115,
            110,
            90,
            5,
            45
        );

        profiles[4] = makeProfile(
            GardenProfiles::ProfileId::Tomatoes,
            "Tomaten",
            "Tomate",
            30,
            130,
            120,
            85,
            5,
            40
        );

        profiles[5] = makeProfile(
            GardenProfiles::ProfileId::Hedge,
            "Hecke",
            "Hecke",
            0,
            90,
            90,
            115,
            10,
            90
        );

        profiles[6] = makeProfile(
            GardenProfiles::ProfileId::Trees,
            "Bäume",
            "Baum",
            -10,
            80,
            80,
            120,
            15,
            120
        );

        profiles[7] = makeProfile(
            GardenProfiles::ProfileId::Mediterranean,
            "Mediterran",
            "Sonne",
            -20,
            70,
            70,
            130,
            3,
            40
        );
    }
}

void GardenProfiles::begin()
{
    if (initialized)
    {
        return;
    }

    preferences.begin(
        NVS_NAMESPACE,
        false
    );

    const size_t expected =
        sizeof(profiles);

    const bool valid =
        preferences.getUInt(
            KEY_VERSION,
            0
        ) == STORAGE_VERSION &&
        preferences.getBytesLength(
            KEY_PROFILES
        ) == expected &&
        preferences.getBytes(
            KEY_PROFILES,
            profiles,
            expected
        ) == expected;

    if (!valid)
    {
        applyDefaults();
        save();
        Serial.println(
            "GardenProfiles: Standardprofile geladen"
        );
    }
    else
    {
        Serial.println(
            "GardenProfiles: Profile aus NVS geladen"
        );
    }

    initialized = true;
}

const GardenProfiles::Profile&
GardenProfiles::profile(ProfileId id)
{
    return profileByIndex(
        static_cast<uint8_t>(id)
    );
}

const GardenProfiles::Profile&
GardenProfiles::profileByIndex(
    uint8_t index)
{
    if (!initialized)
    {
        begin();
    }

    return index < PROFILE_COUNT
        ? profiles[index]
        : profiles[0];
}

bool GardenProfiles::isValid(uint8_t id)
{
    return id < PROFILE_COUNT;
}

const char* GardenProfiles::name(uint8_t id)
{
    return profileByIndex(id).name;
}

const char* GardenProfiles::symbol(uint8_t id)
{
    return profileByIndex(id).symbol;
}

bool GardenProfiles::update(
    uint8_t id,
    const String& newName,
    const String& newSymbol,
    int16_t correctionPercent,
    uint8_t temperatureSensitivityPercent,
    uint8_t humiditySensitivityPercent,
    uint8_t rainSensitivityPercent,
    uint16_t minimumMinutes,
    uint16_t maximumMinutes)
{
    if (!isValid(id) ||
        newName.length() == 0 ||
        newName.length() >=
            sizeof(Profile::name) ||
        newSymbol.length() >=
            sizeof(Profile::symbol) ||
        correctionPercent < -80 ||
        correctionPercent > 100 ||
        temperatureSensitivityPercent < 0 ||
        temperatureSensitivityPercent > 200 ||
        humiditySensitivityPercent > 200 ||
        rainSensitivityPercent > 200 ||
        minimumMinutes < 1 ||
        maximumMinutes < minimumMinutes ||
        maximumMinutes > 240)
    {
        return false;
    }

    Profile& selected = profiles[id];

    copyText(
        selected.name,
        sizeof(selected.name),
        newName
    );

    copyText(
        selected.symbol,
        sizeof(selected.symbol),
        newSymbol
    );

    selected.correctionPercent =
        correctionPercent;

    selected.temperatureSensitivityPercent =
        temperatureSensitivityPercent;

    selected.humiditySensitivityPercent =
        humiditySensitivityPercent;

    selected.rainSensitivityPercent =
        rainSensitivityPercent;

    selected.minimumMinutes =
        minimumMinutes;

    selected.maximumMinutes =
        maximumMinutes;

    return save();
}

bool GardenProfiles::save()
{
    const size_t expected =
        sizeof(profiles);

    const size_t written =
        preferences.putBytes(
            KEY_PROFILES,
            profiles,
            expected
        );

    if (written != expected)
    {
        Serial.printf(
            "GardenProfiles speichern fehlgeschlagen: "
            "%u von %u Bytes\n",
            static_cast<unsigned>(written),
            static_cast<unsigned>(expected)
        );

        return false;
    }

    preferences.putUInt(
        KEY_VERSION,
        STORAGE_VERSION
    );

    return true;
}

void GardenProfiles::resetDefaults()
{
    applyDefaults();
    save();
}
