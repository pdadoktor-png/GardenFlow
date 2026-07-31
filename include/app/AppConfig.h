#pragma once

#include <Arduino.h>
#include "app/AppSecrets.h"

namespace AppConfig
{
    static constexpr uint8_t DISPLAYED_VALVE_COUNT = 2;
    static constexpr uint8_t MAX_VALVE_COUNT = 8;
    static constexpr uint32_t DEFAULT_PULSE_MS = 250UL;
    static constexpr uint32_t BUTTON_LOCKOUT_MS = 500UL;
    static constexpr float BACKLIGHT = 0.80f;

    // Energiesparen: Displaybeleuchtung nach Inaktivitaet ausschalten.
    static constexpr uint16_t DISPLAY_SLEEP_TIMEOUT_SECONDS = 20;
    static constexpr uint16_t DISPLAY_SLEEP_TIMEOUT_MIN_SECONDS = 5;
    static constexpr uint16_t DISPLAY_SLEEP_TIMEOUT_MAX_SECONDS = 300;

    // WLAN-Zugangsdaten stehen lokal in AppSecrets.h und werden nicht eingecheckt.
    static constexpr const char* WIFI_SSID = AppSecrets::WIFI_SSID;
    static constexpr const char* WIFI_PASSWORD = AppSecrets::WIFI_PASSWORD;

    // Netzwerkname und optionales Passwort fuer drahtlose Firmware-Updates.
    static constexpr char HOSTNAME[] = "gardenflow";
    static constexpr char OTA_PASSWORD[] = "";

    // Deutschland: automatische Sommer-/Winterzeit.
    static constexpr char TZ_INFO[] = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    static constexpr char NTP_SERVER_1[] = "pool.ntp.org";
    static constexpr char NTP_SERVER_2[] = "time.cloudflare.com";
    static constexpr char NTP_SERVER_3[] = "time.google.com";

    // -1 = Simulationsbetrieb. Ventilspulen nie direkt an GPIO anschliessen.
    static constexpr int8_t VALVE_GPIO[MAX_VALVE_COUNT] =
    {
        -1, -1, -1, -1, -1, -1, -1, -1
    };
}
