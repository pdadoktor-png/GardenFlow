#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>

class SettingsManager;

class SetupPortal
{
public:
    void begin(SettingsManager& settingsManager);
    void update();

    bool isActive() const;
    String accessPointSsid() const;
    String accessPointIp() const;

private:
    static constexpr uint16_t DNS_PORT = 53;

    SettingsManager* settingsManager_ = nullptr;
    DNSServer dnsServer_;
    WebServer webServer_{80};

    bool active_ = false;
    bool restartPending_ = false;
    uint32_t restartRequestedAtMs_ = 0;

    void startAccessPoint();
    void configureRoutes();

    void handleRoot();
    void handleSave();
    void handleNotFound();

    void sendSetupPage(
        int statusCode,
        const String& message,
        bool success
    );

    static String htmlEscape(const String& value);
};
