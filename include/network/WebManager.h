#pragma once

#include <Arduino.h>
#include <WebServer.h>

class Scheduler;
class RuntimeManager;
class ValveManager;
class TimeManager;

class WebManager
{
public:
    void begin(Scheduler& scheduler,
               RuntimeManager& runtimeManager,
               ValveManager& valveManager,
               TimeManager& timeManager);
    void update();
    bool isStarted() const;

private:
    WebServer server_{80};
    Scheduler* scheduler_ = nullptr;
    RuntimeManager* runtimeManager_ = nullptr;
    ValveManager* valveManager_ = nullptr;
    TimeManager* timeManager_ = nullptr;
    bool started_ = false;
    bool otaStarted_ = false;
    bool wifiWasConnected_ = false;

    void startServices();
    void configureRoutes();
    void configureOta();

    void handleRoot();
    void handleStatus();
    void handlePrograms();
    void handleCreateProgram();
    void handleUpdateProgram();
    void handleDeleteProgram();
    void handleCopyProgram();
    void handleToggleProgram();
    void handleStartProgram();
    void handleStop();
    void handleToggleValve();
    void handleNotFound();

    void sendJson(int code, const String& body);
    static String jsonEscape(const String& value);
    static String weekdayText(uint8_t mask);
};
