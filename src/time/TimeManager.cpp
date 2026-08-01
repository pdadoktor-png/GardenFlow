#include "time/TimeManager.h"
#include "app/AppConfig.h"
#include <WiFi.h>
#include <cstring>
#include "log/LogManager.h"

namespace {
constexpr time_t MIN_VALID_EPOCH = 1700000000; // 14.11.2023
constexpr uint32_t WIFI_RETRY_MS = 15000;
constexpr uint32_t STATUS_LOG_MS = 10000;

// Build time is used only for display until NTP has synchronized.
time_t compileEpoch() {
    struct tm t = {};
    const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char mon[4] = {__DATE__[0], __DATE__[1], __DATE__[2], 0};
    const char* p = strstr(months, mon);
    t.tm_mon = p ? static_cast<int>((p - months) / 3) : 0;
    t.tm_mday = atoi(__DATE__ + 4);
    t.tm_year = atoi(__DATE__ + 7) - 1900;
    t.tm_hour = atoi(__TIME__);
    t.tm_min = atoi(__TIME__ + 3);
    t.tm_sec = atoi(__TIME__ + 6);
    return mktime(&t);
}
}

void TimeManager::begin() {
    setenv("TZ", AppConfig::TZ_INFO, 1);
    tzset();

    fallbackEpoch_ = compileEpoch();
    fallbackStartedMs_ = millis();
    valid_ = false;

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

    Serial.println("TimeManager initialisiert");

    if (!credentialsConfigured()) {
        Serial.println("WLAN nicht konfiguriert: WIFI_SSID/WIFI_PASSWORD in include/app/AppConfig.h eintragen");
        return;
    }

    startWifi();
}

void TimeManager::update() {
    const bool connected = WiFi.status() == WL_CONNECTED;

    if (connected && !wifiWasConnected_) {
        wifiWasConnected_ = true;
        Log.addf(LogManager::Category::Wifi, LogManager::Level::Info, "WLAN verbunden: %s", WiFi.SSID().c_str());
        Log.addf(LogManager::Category::Wifi, LogManager::Level::Info, "IP-Adresse: %s", WiFi.localIP().toString().c_str());
        Serial.printf("Signalstaerke: %ld dBm\n", static_cast<long>(WiFi.RSSI()));
        startNtp();
    } else if (!connected && wifiWasConnected_) {
        wifiWasConnected_ = false;
        ntpConfigured_ = false;
        Log.warning(LogManager::Category::Wifi, "WLAN-Verbindung verloren");
    }

    if (!connected && credentialsConfigured() && millis() - lastWifiAttemptMs_ >= WIFI_RETRY_MS) {
        startWifi();
    }

    const time_t systemNow = time(nullptr);
    if (systemNow > MIN_VALID_EPOCH) {
        if (!valid_) {
            valid_ = true;
            lastSyncEpoch_ = systemNow;
            struct tm local = {};
            localtime_r(&systemNow, &local);
            char text[40];
            strftime(text, sizeof(text), "%d.%m.%Y %H:%M:%S", &local);
            Log.addf(LogManager::Category::Time, LogManager::Level::Info, "Systemzeit synchronisiert: %s", text);
            Log.info(LogManager::Category::Scheduler, "Automatik freigegeben");
        }
    } else if (connected && ntpConfigured_ && millis() - ntpStartedMs_ > STATUS_LOG_MS && millis() - lastStatusLogMs_ > STATUS_LOG_MS) {
        lastStatusLogMs_ = millis();
        Serial.println("NTP-Synchronisierung laeuft ...");
    }
}

bool TimeManager::credentialsConfigured() const {
    return AppConfig::WIFI_SSID[0] != '\0' &&
           strcmp(AppConfig::WIFI_SSID, "DEIN_WLAN") != 0;
}

void TimeManager::startWifi() {
    lastWifiAttemptMs_ = millis();
    Serial.printf("Verbinde mit WLAN: %s\n", AppConfig::WIFI_SSID);
    WiFi.disconnect(false, false);
    WiFi.begin(AppConfig::WIFI_SSID, AppConfig::WIFI_PASSWORD);
}

void TimeManager::startNtp() {
    ntpConfigured_ = true;
    ntpStartedMs_ = millis();
    Serial.println("NTP-Synchronisierung gestartet");
    configTzTime(
        AppConfig::TZ_INFO,
        AppConfig::NTP_SERVER_1,
        AppConfig::NTP_SERVER_2,
        AppConfig::NTP_SERVER_3
    );
}

time_t TimeManager::nowEpoch() const {
    const time_t systemNow = time(nullptr);
    if (systemNow > MIN_VALID_EPOCH) {
        return systemNow;
    }
    return fallbackEpoch_ + static_cast<time_t>((millis() - fallbackStartedMs_) / 1000UL);
}

bool TimeManager::isValid() const { return valid_; }
bool TimeManager::isWifiConnected() const { return WiFi.status() == WL_CONNECTED; }
bool TimeManager::isSynchronizing() const { return isWifiConnected() && ntpConfigured_ && !valid_; }

bool TimeManager::getLocalTime(struct tm& value) const {
    const time_t n = nowEpoch();
    return localtime_r(&n, &value) != nullptr;
}

uint8_t TimeManager::hour() const { struct tm t = {}; getLocalTime(t); return t.tm_hour; }
uint8_t TimeManager::minute() const { struct tm t = {}; getLocalTime(t); return t.tm_min; }
uint8_t TimeManager::weekdayMondayZero() const { struct tm t = {}; getLocalTime(t); return static_cast<uint8_t>((t.tm_wday + 6) % 7); }
void TimeManager::formatTime(char* b, size_t s) const { struct tm t = {}; getLocalTime(t); strftime(b, s, "%H:%M", &t); }
void TimeManager::formatDate(char* b, size_t s) const { struct tm t = {}; getLocalTime(t); strftime(b, s, "%d.%m.%Y", &t); }

String TimeManager::wifiSsid() const { return isWifiConnected() ? WiFi.SSID() : String(AppConfig::WIFI_SSID); }
String TimeManager::ipAddress() const { return isWifiConnected() ? WiFi.localIP().toString() : String("--"); }
int32_t TimeManager::wifiRssi() const { return isWifiConnected() ? WiFi.RSSI() : 0; }
time_t TimeManager::lastSyncEpoch() const { return lastSyncEpoch_; }
