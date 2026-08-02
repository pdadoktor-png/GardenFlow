#include "network/WifiManager.h"
#include "settings/SettingsManager.h"

void WifiManager::begin(SettingsManager& settingsManager)
{
    settingsManager_ = &settingsManager;
}

bool WifiManager::startSetupAccessPoint(
    const char* ssid,
    const char* password,
    const IPAddress& ip,
    const IPAddress& gateway,
    const IPAddress& subnet)
{
    WiFi.persistent(false);
    WiFi.disconnect(true, false);
    delay(150);

    WiFi.mode(WIFI_OFF);
    delay(150);

    WiFi.mode(WIFI_AP);
    delay(150);

    if (!WiFi.softAPConfig(ip, gateway, subnet))
    {
        Serial.println("WifiManager: AP-IP-Konfiguration fehlgeschlagen");
        setupAccessPointActive_ = false;
        return false;
    }

    if (!WiFi.softAP(ssid, password))
    {
        Serial.println("WifiManager: Access Point konnte nicht gestartet werden");
        setupAccessPointActive_ = false;
        return false;
    }

    delay(250);
    setupAccessPointActive_ = true;
    return true;
}

void WifiManager::stopSetupAccessPoint()
{
    if (!setupAccessPointActive_)
    {
        return;
    }

    WiFi.softAPdisconnect(true);
    setupAccessPointActive_ = false;
}

bool WifiManager::setupAccessPointActive() const
{
    return setupAccessPointActive_;
}

IPAddress WifiManager::setupAccessPointIp() const
{
    return WiFi.softAPIP();
}
