#pragma once

#include <Arduino.h>

namespace GardenProfiles
{
    enum class ProfileId : uint8_t
    {
        General = 0,
        Lawn = 1,
        Flowers = 2,
        Vegetables = 3,
        Tomatoes = 4,
        Hedge = 5,
        Trees = 6,
        Mediterranean = 7
    };

    struct Profile
    {
        ProfileId id = ProfileId::General;
        char name[24] = "";
        char symbol[16] = "";

        // Feste Profilkorrektur auf die saisonale Grundlaufzeit.
        int16_t correctionPercent = 0;

        // Stärke, mit der die jeweiligen Wetterbeiträge wirken.
        uint8_t temperatureSensitivityPercent = 100;
        uint8_t humiditySensitivityPercent = 100;
        uint8_t rainSensitivityPercent = 100;

        uint16_t minimumMinutes = 1;
        uint16_t maximumMinutes = 240;
    };

    static constexpr uint8_t PROFILE_COUNT = 8;

    void begin();

    const Profile& profile(ProfileId id);
    const Profile& profileByIndex(uint8_t index);

    bool isValid(uint8_t id);
    const char* name(uint8_t id);
    const char* symbol(uint8_t id);

    bool update(
        uint8_t id,
        const String& name,
        const String& symbol,
        int16_t correctionPercent,
        uint8_t temperatureSensitivityPercent,
        uint8_t humiditySensitivityPercent,
        uint8_t rainSensitivityPercent,
        uint16_t minimumMinutes,
        uint16_t maximumMinutes
    );

    bool save();
    void resetDefaults();
}
