#include <Arduino.h>
#include <lv_conf.h>
#include <esp32_smartdisplay.h>

#include "DisplayManager.h"
#include "ValveManager.h"
#include "Scheduler.h"
#include "runtime/RuntimeManager.h"
#include "time/TimeManager.h"
#include "network/WebManager.h"
#include "weather/WeatherManager.h"

static ValveManager valveManager;
static DisplayManager displayManager;
static Scheduler scheduler;
static RuntimeManager runtimeManager;
static TimeManager timeManager;
static WebManager webManager;
static WeatherManager weatherManager;

void setup()
{
    Serial.begin(115200);
    delay(800);

    Serial.println();
    Serial.println("================================");
    Serial.println("GardenFlow Professional");
    Serial.println("ESP32-4827S043R");
    Serial.println("================================");

    valveManager.begin();
    scheduler.begin();
    timeManager.begin();
    runtimeManager.begin(scheduler, valveManager, timeManager);
    weatherManager.begin(timeManager);
    runtimeManager.setWeatherManager(weatherManager);
    
    displayManager.begin(
        valveManager,
        scheduler,
        runtimeManager,
        timeManager);

    webManager.begin(
        scheduler,
        runtimeManager,
        valveManager,
        timeManager,
        weatherManager);

    Serial.println("System bereit");
}

void loop()
{
    valveManager.update();
    scheduler.update();
    timeManager.update();
    weatherManager.update();
    runtimeManager.update();
    displayManager.update();
    webManager.update();

    delay(5);
}