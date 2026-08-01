#pragma once
#include <Arduino.h>
#include <time.h>

class SettingsManager;

class TimeManager {
public:
    void begin(SettingsManager& settingsManager);
    void update();

    // true only after a real NTP synchronization
    bool isValid() const;
    bool isWifiConnected() const;
    bool isSynchronizing() const;

    bool getLocalTime(struct tm& value) const;
    uint8_t hour() const;
    uint8_t minute() const;
    uint8_t weekdayMondayZero() const;
    void formatTime(char* buffer, size_t size) const;
    void formatDate(char* buffer, size_t size) const;

    String wifiSsid() const;
    String ipAddress() const;
    int32_t wifiRssi() const;
    time_t lastSyncEpoch() const;

private:
    SettingsManager* settingsManager_ = nullptr;
    time_t fallbackEpoch_ = 0;
    uint32_t fallbackStartedMs_ = 0;
    uint32_t lastWifiAttemptMs_ = 0;
    uint32_t ntpStartedMs_ = 0;
    uint32_t lastStatusLogMs_ = 0;
    time_t lastSyncEpoch_ = 0;
    bool valid_ = false;
    bool ntpConfigured_ = false;
    bool wifiWasConnected_ = false;

    time_t nowEpoch() const;
    void startWifi();
    void startNtp();
    bool credentialsConfigured() const;
};
