#pragma once
#include <Arduino.h>
#include <WiFi.h>

class SettingsManager;

class WifiManager
{
public:
    void begin(SettingsManager& settingsManager);

    bool startSetupAccessPoint(
        const char* ssid,
        const char* password,
        const IPAddress& ip,
        const IPAddress& gateway,
        const IPAddress& subnet
    );

    void stopSetupAccessPoint();
    bool setupAccessPointActive() const;
    IPAddress setupAccessPointIp() const;

private:
    SettingsManager* settingsManager_ = nullptr;
    bool setupAccessPointActive_ = false;
};
