#pragma once

namespace WeatherConfig
{
    // Kostenlosen API-Schluessel unter openweathermap.org anlegen und hier eintragen.
    static constexpr char API_KEY[] = "5fb66db608b43b5ae20e0ad1e6f09da8";
    // 374789989ec2c2848f6d088081ead514
    // Standort fuer die Wetterabfrage. Beispiel: Langenfeld (Rheinland).
    static constexpr float LATITUDE = 51.1082f;
    static constexpr float LONGITUDE = 6.9483f;

    // Automatische Regenpause, wenn eine der Grenzen erreicht wird.
    static constexpr bool AUTO_RAIN_PAUSE_ENABLED = true;
    static constexpr float RAIN_LIMIT_MM_24H = 3.0f;
    static constexpr uint8_t RAIN_PROBABILITY_LIMIT_PERCENT = 70;

    // Wetterdaten werden im Normalbetrieb alle 30 Minuten aktualisiert.
    static constexpr unsigned long UPDATE_INTERVAL_MS = 30UL * 60UL * 1000UL;
    static constexpr unsigned long RETRY_INTERVAL_MS = 5UL * 60UL * 1000UL;
}
