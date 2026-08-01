#include "settings/SettingsManager.h"

#include <cmath>

#include "app/AppConfig.h"
#include "app/WeatherConfig.h"

void SettingsManager::begin()
{
    preferences_.begin("systemcfg", false);
    load();
    Serial.println("SettingsManager initialisiert");
}

const String& SettingsManager::wifiSsid() const
{
    return wifiSsid_;
}

const String& SettingsManager::wifiPassword() const
{
    return wifiPassword_;
}

const String& SettingsManager::timezone() const
{
    return timezone_;
}

float SettingsManager::latitude() const
{
    return latitude_;
}

float SettingsManager::longitude() const
{
    return longitude_;
}

bool SettingsManager::credentialsConfigured() const
{
    return wifiSsid_.length() > 0 &&
           wifiSsid_ != "DEIN_WLAN" &&
           wifiSsid_ != "xxxx";
}


bool SettingsManager::setupPortalRequested() 
{
    return preferences_.getBool("setupPortal", false);
}

void SettingsManager::requestSetupPortal(bool requested)
{
    preferences_.putBool("setupPortal", requested);
}

bool SettingsManager::saveNetworkLocation(
    const String& ssid,
    const String& password,
    float latitude,
    float longitude,
    const String& timezone)
{
    String cleanSsid = ssid;
    String cleanTimezone = timezone;
    cleanSsid.trim();
    cleanTimezone.trim();

    if (cleanSsid.length() == 0 ||
        cleanSsid.length() > 32 ||
        cleanTimezone.length() == 0 ||
        cleanTimezone.length() > 96 ||
        !std::isfinite(latitude) ||
        !std::isfinite(longitude) ||
        latitude < -90.0f ||
        latitude > 90.0f ||
        longitude < -180.0f ||
        longitude > 180.0f)
    {
        return false;
    }

    wifiSsid_ = cleanSsid;

    // Leeres Passwort bedeutet: vorhandenes Passwort behalten.
    if (password.length() > 0)
    {
        wifiPassword_ = password;
    }

    latitude_ = latitude;
    longitude_ = longitude;
    timezone_ = cleanTimezone;

    preferences_.putString("ssid", wifiSsid_);
    preferences_.putString("password", wifiPassword_);
    preferences_.putFloat("latitude", latitude_);
    preferences_.putFloat("longitude", longitude_);
    preferences_.putString("timezone", timezone_);

    return true;
}

void SettingsManager::load()
{
    wifiSsid_ = preferences_.getString(
        "ssid",
        AppConfig::WIFI_SSID
    );

    wifiPassword_ = preferences_.getString(
        "password",
        AppConfig::WIFI_PASSWORD
    );

    latitude_ = preferences_.getFloat(
        "latitude",
        WeatherConfig::LATITUDE
    );

    longitude_ = preferences_.getFloat(
        "longitude",
        WeatherConfig::LONGITUDE
    );

    timezone_ = preferences_.getString(
        "timezone",
        AppConfig::TZ_INFO
    );

    if (timezone_.length() == 0)
    {
        timezone_ = AppConfig::TZ_INFO;
    }
}
