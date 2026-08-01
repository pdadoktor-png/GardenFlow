#pragma once

#include <Arduino.h>
#include <Preferences.h>

class SettingsManager
{
public:
    void begin();

    const String& wifiSsid() const;
    const String& wifiPassword() const;
    const String& timezone() const;
    float latitude() const;
    float longitude() const;

    bool credentialsConfigured() const;
    bool setupPortalRequested() ;
    void requestSetupPortal(bool requested);

    bool saveNetworkLocation(
        const String& ssid,
        const String& password,
        float latitude,
        float longitude,
        const String& timezone
    );

private:
    Preferences preferences_;
    String wifiSsid_;
    String wifiPassword_;
    String timezone_;
    float latitude_ = 0.0f;
    float longitude_ = 0.0f;

    void load();
};
