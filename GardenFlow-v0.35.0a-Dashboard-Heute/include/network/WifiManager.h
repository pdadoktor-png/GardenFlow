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

    int scanNetworks(bool showHidden = true);
    String scannedSsid(int index) const;
    int32_t scannedRssi(int index) const;
    wifi_auth_mode_t scannedEncryption(int index) const;
    void clearScanResults();
    static bool isOpenNetwork(wifi_auth_mode_t mode);

    bool connectStationForTest(
        const String& ssid,
        const String& password,
        uint32_t timeoutMs
    );
    void disconnectTestStation();

private:
    SettingsManager* settingsManager_ = nullptr;
    bool setupAccessPointActive_ = false;
};
