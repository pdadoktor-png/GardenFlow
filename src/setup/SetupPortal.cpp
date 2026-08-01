#include "setup/SetupPortal.h"

#include <WiFi.h>

#include "settings/SettingsManager.h"

constexpr uint16_t SetupPortal::DNS_PORT;

namespace
{
constexpr char AP_SSID[] = "GardenFlow-Setup";
constexpr char AP_PASSWORD[] = "gardenflow";

const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);
}

void SetupPortal::begin(SettingsManager& settingsManager)
{
    settingsManager_ = &settingsManager;

    const bool requested =
        settingsManager_->setupPortalRequested();

    if (settingsManager_->credentialsConfigured() && !requested)
    {
        active_ = false;
        return;
    }

    startAccessPoint();
}

void SetupPortal::update()
{
    if (!active_)
    {
        return;
    }

    dnsServer_.processNextRequest();
    webServer_.handleClient();

    if (restartPending_ &&
        static_cast<uint32_t>(millis() - restartRequestedAtMs_) >= 1500UL)
    {
        ESP.restart();
    }
}

bool SetupPortal::isActive() const
{
    return active_;
}

String SetupPortal::accessPointSsid() const
{
    return String(AP_SSID);
}

String SetupPortal::accessPointIp() const
{
    return AP_IP.toString();
}

void SetupPortal::startAccessPoint()
{
    WiFi.disconnect(true, false);
    delay(100);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

    if (!WiFi.softAP(AP_SSID, AP_PASSWORD))
    {
        Serial.println("Setup-Portal konnte nicht gestartet werden");
        return;
    }

    configureRoutes();

    dnsServer_.start(DNS_PORT, "*", AP_IP);
    webServer_.begin();

    active_ = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("GardenFlow Setup-Portal aktiv");
    Serial.printf("SSID: %s\n", AP_SSID);
    Serial.printf("Passwort: %s\n", AP_PASSWORD);
    Serial.printf("Adresse: http://%s/\n", AP_IP.toString().c_str());
    Serial.println("================================");
}

void SetupPortal::configureRoutes()
{
    webServer_.on("/", HTTP_GET, [this]() {
        handleRoot();
    });

    webServer_.on("/save", HTTP_POST, [this]() {
        handleSave();
    });

    // Typical captive-portal detection URLs.
    webServer_.on("/generate_204", HTTP_ANY, [this]() {
        handleRoot();
    });
    webServer_.on("/hotspot-detect.html", HTTP_ANY, [this]() {
        handleRoot();
    });
    webServer_.on("/connecttest.txt", HTTP_ANY, [this]() {
        handleRoot();
    });
    webServer_.on("/ncsi.txt", HTTP_ANY, [this]() {
        handleRoot();
    });
    webServer_.on("/fwlink", HTTP_ANY, [this]() {
        handleRoot();
    });

    webServer_.onNotFound([this]() {
        handleNotFound();
    });
}

void SetupPortal::handleRoot()
{
    sendSetupPage(200, String(), false);
}

void SetupPortal::handleSave()
{
    if (!settingsManager_)
    {
        sendSetupPage(
            503,
            "Interner Fehler: Einstellungen nicht bereit.",
            false
        );
        return;
    }

    if (!webServer_.hasArg("ssid"))
    {
        sendSetupPage(
            400,
            "Bitte einen WLAN-Namen eingeben.",
            false
        );
        return;
    }

    String ssid = webServer_.arg("ssid");
    String password = webServer_.hasArg("password")
        ? webServer_.arg("password")
        : String();

    ssid.trim();

    if (ssid.length() == 0 || ssid.length() > 32)
    {
        sendSetupPage(
            400,
            "Der WLAN-Name ist leer oder zu lang.",
            false
        );
        return;
    }

    /*
     * In 0033a werden ausschließlich WLAN-Daten geändert.
     * Standort und Zeitzone bleiben auf ihren bisherigen
     * beziehungsweise werkseitigen Werten.
     */
    const bool saved = settingsManager_->saveNetworkLocation(
        ssid,
        password,
        settingsManager_->latitude(),
        settingsManager_->longitude(),
        settingsManager_->timezone()
    );

    if (!saved)
    {
        sendSetupPage(
            400,
            "Die WLAN-Daten konnten nicht gespeichert werden.",
            false
        );
        return;
    }

    Serial.printf(
        "Setup-Portal: WLAN '%s' gespeichert; Neustart\n",
        ssid.c_str()
    );

    settingsManager_->requestSetupPortal(false);

    restartPending_ = true;
    restartRequestedAtMs_ = millis();

    sendSetupPage(
        200,
        "WLAN-Daten gespeichert. GardenFlow startet neu und verbindet sich mit dem Heimnetz.",
        true
    );
}

void SetupPortal::handleNotFound()
{
    webServer_.sendHeader(
        "Location",
        String("http://") + AP_IP.toString() + "/",
        true
    );
    webServer_.send(302, "text/plain", "");
}

void SetupPortal::sendSetupPage(
    int statusCode,
    const String& message,
    bool success)
{
    String html;
    html.reserve(4300);

    html += F(
        "<!doctype html><html lang='de'><head>"
        "<meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>GardenFlow Setup</title>"
        "<style>"
        ":root{font-family:system-ui,-apple-system,sans-serif;color-scheme:dark;"
        "background:#101714;color:#edf5ef}"
        "body{margin:0;min-height:100vh;display:grid;place-items:center;padding:18px}"
        ".card{width:min(460px,100%);box-sizing:border-box;background:#18231d;"
        "border:1px solid #34483b;border-radius:18px;padding:22px;"
        "box-shadow:0 12px 32px #0007}"
        "h1{margin:0 0 4px;font-size:1.8rem}"
        ".muted{color:#a8b7ad;margin-bottom:18px}"
        "label{display:flex;flex-direction:column;gap:7px;margin-top:14px}"
        "input{border:1px solid #4a6050;border-radius:10px;background:#101714;"
        "color:#edf5ef;padding:12px;font-size:1rem}"
        "button{width:100%;margin-top:20px;border:0;border-radius:11px;"
        "padding:13px;font-size:1rem;font-weight:800;background:#7fda98;color:#102016}"
        ".info{margin-top:16px;padding:12px;border-radius:10px;background:#223128}"
        ".ok{background:#214d31;color:#bff1ca}"
        ".err{background:#5e302e;color:#ffd0cc}"
        ".small{font-size:.86rem;color:#a8b7ad;margin-top:16px}"
        "</style></head><body><main class='card'>"
        "<h1>GardenFlow Setup</h1>"
        "<div class='muted'>Mit dem heimischen WLAN verbinden</div>"
    );

    if (message.length() > 0)
    {
        html += F("<div class='info ");
        html += success ? F("ok") : F("err");
        html += F("'>");
        html += htmlEscape(message);
        html += F("</div>");
    }

    if (!success)
    {
        html += F(
            "<form method='post' action='/save'>"
            "<label>WLAN-Name (SSID)"
            "<input name='ssid' maxlength='32' required autocomplete='username' "
            "placeholder='z. B. FRITZ!Box 7590'></label>"
            "<label>WLAN-Passwort"
            "<input name='password' type='password' maxlength='64' "
            "autocomplete='current-password'></label>"
            "<button type='submit'>Speichern und neu starten</button>"
            "</form>"
        );
    }

    html += F(
        "<div class='small'>Setup-WLAN: GardenFlow-Setup<br>"
        "Adresse: 192.168.4.1</div>"
        "</main></body></html>"
    );

    webServer_.sendHeader("Cache-Control", "no-store");
    webServer_.send(
        statusCode,
        "text/html; charset=utf-8",
        html
    );
}

String SetupPortal::htmlEscape(const String& value)
{
    String escaped;
    escaped.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); ++i)
    {
        switch (value[i])
        {
            case '&': escaped += F("&amp;"); break;
            case '<': escaped += F("&lt;"); break;
            case '>': escaped += F("&gt;"); break;
            case '"': escaped += F("&quot;"); break;
            case '\'': escaped += F("&#39;"); break;
            default: escaped += value[i]; break;
        }
    }

    return escaped;
}
