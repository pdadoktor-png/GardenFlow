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

int WifiManager::scanNetworks(bool showHidden)
{
    clearScanResults();
    return WiFi.scanNetworks(false, showHidden);
}

String WifiManager::scannedSsid(int index) const
{
    return WiFi.SSID(index);
}

int32_t WifiManager::scannedRssi(int index) const
{
    return WiFi.RSSI(index);
}

wifi_auth_mode_t WifiManager::scannedEncryption(int index) const
{
    return WiFi.encryptionType(index);
}

void WifiManager::clearScanResults()
{
    WiFi.scanDelete();
}

bool WifiManager::isOpenNetwork(wifi_auth_mode_t mode)
{
    return mode == WIFI_AUTH_OPEN;
}


bool WifiManager::connectStationForTest(
    const String& ssid,
    const String& password,
    uint32_t timeoutMs)
{
    if (ssid.length() == 0)
    {
        return false;
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false, false);
    delay(100);

    WiFi.begin(ssid.c_str(), password.c_str());

    const uint32_t startedAt = millis();

    while (WiFi.status() != WL_CONNECTED &&
           static_cast<uint32_t>(millis() - startedAt) < timeoutMs)
    {
        delay(100);
    }

    return WiFi.status() == WL_CONNECTED;
}

void WifiManager::disconnectTestStation()
{
    WiFi.disconnect(false, false);
    delay(100);

    if (setupAccessPointActive_)
    {
        WiFi.mode(WIFI_AP);
    }
}
