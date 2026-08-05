#pragma once

#include <Arduino.h>

/*
 * Ventil-Hardwareprofil
 *
 * Pro Ventil wird genau ein GPIO verwendet. Jeder Zustandswechsel
 * (Öffnen oder Schließen) erfolgt durch einen kurzen Impuls auf
 * demselben GPIO. Der angenommene Ventilzustand wird im ValveManager
 * mitgeführt.
 *
 * GPIOs können in platformio.ini gesetzt werden:
 *
 * build_flags =
 *   -D GARDENFLOW_VALVE1_GPIO=4
 *   -D GARDENFLOW_VALVE2_GPIO=5
 *
 * Ohne Definition bleibt -1 aktiv: sicherer Simulationsbetrieb.
 */

#ifndef GARDENFLOW_VALVE1_GPIO
#define GARDENFLOW_VALVE1_GPIO -1
#endif

#ifndef GARDENFLOW_VALVE2_GPIO
#define GARDENFLOW_VALVE2_GPIO -1
#endif

#ifndef GARDENFLOW_VALVE_ACTIVE_HIGH
#define GARDENFLOW_VALVE_ACTIVE_HIGH 1
#endif

#ifndef GARDENFLOW_VALVE_PULSE_MS
#define GARDENFLOW_VALVE_PULSE_MS 250
#endif

namespace HardwareProfile
{
    struct ValveOutputConfig
    {
        int8_t gpio;
        bool activeHigh;
        uint32_t pulseDurationMs;
    };

    static constexpr uint8_t VALVE_COUNT = 2;

    static constexpr ValveOutputConfig VALVES[VALVE_COUNT] =
    {
        {
            static_cast<int8_t>(GARDENFLOW_VALVE1_GPIO),
            GARDENFLOW_VALVE_ACTIVE_HIGH != 0,
            static_cast<uint32_t>(GARDENFLOW_VALVE_PULSE_MS)
        },
        {
            static_cast<int8_t>(GARDENFLOW_VALVE2_GPIO),
            GARDENFLOW_VALVE_ACTIVE_HIGH != 0,
            static_cast<uint32_t>(GARDENFLOW_VALVE_PULSE_MS)
        }
    };
}
