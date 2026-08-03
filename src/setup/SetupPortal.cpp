#include "setup/SetupPortal.h"


#include "settings/SettingsManager.h"
#include "network/WifiManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

constexpr uint16_t SetupPortal::DNS_PORT;

namespace
{
constexpr char AP_SSID[] = "GardenFlow-Setup";
constexpr char AP_PASSWORD[] = "gardenflow";

const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);
}

void SetupPortal::begin(SettingsManager& settingsManager, WifiManager& wifiManager)
{
    settingsManager_ = &settingsManager;
    wifiManager_ = &wifiManager;

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
    if (wifiManager_ == nullptr)
    {
        Serial.println("SetupPortal: WifiManager fehlt");
        return;
    }

    if (!wifiManager_->startSetupAccessPoint(
            AP_SSID,
            AP_PASSWORD,
            AP_IP,
            AP_GATEWAY,
            AP_SUBNET))
    {
        Serial.println("Setup-Portal konnte nicht gestartet werden");
        return;
    }

    configureRoutes();

    dnsServer_.start(
        DNS_PORT,
        "*",
        wifiManager_->setupAccessPointIp()
    );

    webServer_.begin();
    active_ = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("GardenFlow Setup-Portal aktiv");
    Serial.printf("SSID: %s\n", AP_SSID);
    Serial.printf("Passwort: %s\n", AP_PASSWORD);
    Serial.printf(
        "Adresse: http://%s/\n",
        wifiManager_->setupAccessPointIp().toString().c_str()
    );
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

    webServer_.on("/networks", HTTP_GET, [this]() {
        handleNetworks();
    });

    webServer_.on("/weather-test", HTTP_POST, [this]() {
        handleWeatherTest();
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

    if (!webServer_.hasArg("ssid") ||
        !webServer_.hasArg("latitude") ||
        !webServer_.hasArg("longitude") ||
        !webServer_.hasArg("timezone"))
    {
        sendSetupPage(
            400,
            "Bitte WLAN, Standort und Zeitzone vollständig eingeben.",
            false
        );
        return;
    }

    String ssid = webServer_.arg("ssid");
    String password = webServer_.hasArg("password")
        ? webServer_.arg("password")
        : String();
    String timezone = webServer_.arg("timezone");
    String weatherApiKey = webServer_.hasArg("weatherApiKey")
        ? webServer_.arg("weatherApiKey")
        : String();

    ssid.trim();
    timezone.trim();

    const float latitude = webServer_.arg("latitude").toFloat();
    const float longitude = webServer_.arg("longitude").toFloat();

    if (ssid.length() == 0 || ssid.length() > 32)
    {
        sendSetupPage(
            400,
            "Der WLAN-Name ist leer oder zu lang.",
            false
        );
        return;
    }

    const bool saved = settingsManager_->saveNetworkLocation(
        ssid,
        password,
        latitude,
        longitude,
        timezone,
        weatherApiKey
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
        "WLAN, Standort, Zeitzone und Wetterzugang gespeichert. GardenFlow startet neu und verbindet sich mit dem Heimnetz.",
        true
    );
}

void SetupPortal::handleNetworks()
{
    if (wifiManager_ == nullptr)
    {
        webServer_.send(
            503,
            "application/json; charset=utf-8",
            "{\"error\":\"WifiManager nicht bereit\"}"
        );
        return;
    }

    const int count = wifiManager_->scanNetworks(true);

    struct Item
    {
        String ssid;
        int32_t rssi;
        bool secure;
    };

    Item items[24];
    int itemCount = 0;

    for (int i = 0; i < count && itemCount < 24; ++i)
    {
        const String ssid = wifiManager_->scannedSsid(i);
        if (ssid.length() == 0)
        {
            continue;
        }

        const int32_t rssi = wifiManager_->scannedRssi(i);
        const bool secure = !WifiManager::isOpenNetwork(
            wifiManager_->scannedEncryption(i)
        );

        int existing = -1;
        for (int j = 0; j < itemCount; ++j)
        {
            if (items[j].ssid == ssid)
            {
                existing = j;
                break;
            }
        }

        if (existing >= 0)
        {
            if (rssi > items[existing].rssi)
            {
                items[existing].rssi = rssi;
                items[existing].secure = secure;
            }
            continue;
        }

        items[itemCount].ssid = ssid;
        items[itemCount].rssi = rssi;
        items[itemCount].secure = secure;
        ++itemCount;
    }

    for (int i = 0; i < itemCount - 1; ++i)
    {
        for (int j = i + 1; j < itemCount; ++j)
        {
            if (items[j].rssi > items[i].rssi)
            {
                const Item temp = items[i];
                items[i] = items[j];
                items[j] = temp;
            }
        }
    }

    String body;
    body.reserve(1800);
    body += F("{\"networks\":[");

    for (int i = 0; i < itemCount; ++i)
    {
        if (i > 0)
        {
            body += ',';
        }

        body += F("{\"ssid\":\"");
        body += jsonEscape(items[i].ssid);
        body += F("\",\"rssi\":");
        body += String(items[i].rssi);
        body += F(",\"secure\":");
        body += items[i].secure ? F("true") : F("false");
        body += '}';
    }

    body += F("]}");
    wifiManager_->clearScanResults();

    webServer_.sendHeader("Cache-Control", "no-store");
    webServer_.send(
        200,
        "application/json; charset=utf-8",
        body
    );
}

void SetupPortal::handleWeatherTest()
{
    if (settingsManager_ == nullptr || wifiManager_ == nullptr)
    {
        webServer_.send(
            503,
            "application/json; charset=utf-8",
            "{\"error\":\"Setup nicht bereit\"}"
        );
        return;
    }

    const String ssid = webServer_.hasArg("ssid")
        ? webServer_.arg("ssid")
        : String();

    const String password = webServer_.hasArg("password")
        ? webServer_.arg("password")
        : String();

    String apiKey = webServer_.hasArg("weatherApiKey")
        ? webServer_.arg("weatherApiKey")
        : String();

    apiKey.trim();

    if (apiKey.length() == 0)
    {
        apiKey = settingsManager_->weatherApiKey();
    }

    const float latitude = webServer_.hasArg("latitude")
        ? webServer_.arg("latitude").toFloat()
        : settingsManager_->latitude();

    const float longitude = webServer_.hasArg("longitude")
        ? webServer_.arg("longitude").toFloat()
        : settingsManager_->longitude();

    if (ssid.length() == 0)
    {
        webServer_.send(
            400,
            "application/json; charset=utf-8",
            "{\"error\":\"WLAN-Name fehlt\"}"
        );
        return;
    }

    if (apiKey.length() == 0 ||
        apiKey == "DEIN_OPENWEATHER_API_SCHLUESSEL")
    {
        webServer_.send(
            400,
            "application/json; charset=utf-8",
            "{\"error\":\"OpenWeather API-Schlüssel fehlt\"}"
        );
        return;
    }

    if (latitude < -90.0f || latitude > 90.0f ||
        longitude < -180.0f || longitude > 180.0f)
    {
        webServer_.send(
            400,
            "application/json; charset=utf-8",
            "{\"error\":\"Ungültige Standortkoordinaten\"}"
        );
        return;
    }

    if (!wifiManager_->connectStationForTest(
            ssid,
            password,
            15000UL))
    {
        wifiManager_->disconnectTestStation();

        webServer_.send(
            409,
            "application/json; charset=utf-8",
            "{\"error\":\"Verbindung zum Heim-WLAN fehlgeschlagen\"}"
        );
        return;
    }

    String url;
    url.reserve(240);
    url = "https://api.openweathermap.org/data/2.5/forecast?lat=";
    url += String(latitude, 5);
    url += "&lon=";
    url += String(longitude, 5);
    url += "&appid=";
    url += apiKey;
    url += "&units=metric&lang=de&cnt=1";

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(10000);
    http.setTimeout(15000);

    if (!http.begin(client, url))
    {
        wifiManager_->disconnectTestStation();

        webServer_.send(
            500,
            "application/json; charset=utf-8",
            "{\"error\":\"HTTPS konnte nicht gestartet werden\"}"
        );
        return;
    }

    const int statusCode = http.GET();

    if (statusCode != HTTP_CODE_OK)
    {
        http.end();
        wifiManager_->disconnectTestStation();

        String body = F("{\"error\":\"OpenWeather HTTP ");
        body += String(statusCode);
        body += F("\"}");

        webServer_.send(
            statusCode == 401 ? 401 : 502,
            "application/json; charset=utf-8",
            body
        );
        return;
    }

    JsonDocument filter;
    filter["list"][0]["main"]["temp"] = true;
    filter["list"][0]["main"]["humidity"] = true;
    filter["list"][0]["weather"][0]["description"] = true;

    JsonDocument document;
    const DeserializationError error = deserializeJson(
        document,
        http.getStream(),
        DeserializationOption::Filter(filter)
    );

    http.end();
    wifiManager_->disconnectTestStation();

    if (error)
    {
        String body = F("{\"error\":\"JSON: ");
        body += jsonEscape(String(error.c_str()));
        body += F("\"}");

        webServer_.send(
            502,
            "application/json; charset=utf-8",
            body
        );
        return;
    }

    JsonObject first = document["list"][0];

    if (first.isNull())
    {
        webServer_.send(
            502,
            "application/json; charset=utf-8",
            "{\"error\":\"Keine Wetterdaten empfangen\"}"
        );
        return;
    }

    const float temperature =
        first["main"]["temp"] | 0.0f;

    const uint8_t humidity =
        static_cast<uint8_t>(
            first["main"]["humidity"] | 0
        );

    const String description =
        first["weather"][0]["description"] | "unbekannt";

    String body;
    body.reserve(180);
    body += F("{\"ok\":true,\"temperature\":");
    body += String(temperature, 1);
    body += F(",\"humidity\":");
    body += String(humidity);
    body += F(",\"description\":\"");
    body += jsonEscape(description);
    body += F("\"}");

    webServer_.sendHeader("Cache-Control", "no-store");
    webServer_.send(
        200,
        "application/json; charset=utf-8",
        body
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
            "<div style='display:flex;gap:8px;align-items:end'><label style='flex:1'>WLAN-Name (SSID)"
            "<input id='ssid' name='ssid' maxlength='32' required autocomplete='username' "
            "placeholder='WLAN auswählen oder eingeben'></label>"
            "<button class='secondary' type='button' onclick='scanWifi(this)'>Suchen</button></div>"
            "<div id='wifiList' style='display:none;margin-top:10px;border:1px solid #34483b;border-radius:10px;overflow:hidden'></div>"
            "<label>WLAN-Passwort"
            "<input name='password' type='password' maxlength='64' "
            "autocomplete='current-password'></label>"
            "<label>Breitengrad"
            "<input name='latitude' type='number' min='-90' max='90' "
            "step='0.00001' required value='"
        );
        html += String(settingsManager_ ? settingsManager_->latitude() : 0.0f, 5);
        html += F(
            "'></label>"
            "<label>Längengrad"
            "<input name='longitude' type='number' min='-180' max='180' "
            "step='0.00001' required value='"
        );
        html += String(settingsManager_ ? settingsManager_->longitude() : 0.0f, 5);
        html += F(
            "'></label>"
            "<label>Zeitzone"
            "<input name='timezone' maxlength='96' required value='"
        );
        html += htmlEscape(settingsManager_ ? settingsManager_->timezone() : String("CET-1CEST,M3.5.0/2,M10.5.0/3"));
        html += F(
            "'></label>"
            "<label>OpenWeather API-Schlüssel"
            "<input id='weatherApiKey' name='weatherApiKey' type='password' maxlength='64' "
            "placeholder='leer = vorhandenen Schlüssel behalten' "
            "autocomplete='off'></label>"
            "<button class='secondary' type='button' onclick='testWeather(this)'>Wetterzugang testen</button>"
            "<div id='weatherTestResult' class='info' style='display:none'></div>"
            "<div class='small'>Deutschland: "
            "CET-1CEST,M3.5.0/2,M10.5.0/3<br>"
            "Sommer- und Winterzeit werden automatisch berücksichtigt.</div>"
            "<button type='submit'>Speichern und neu starten</button>"
            "</form>"
        );
    }

    html += F(
        "<div class='small'>Setup-WLAN: GardenFlow-Setup<br>"
        "Adresse: 192.168.4.1</div>"
        "<script>"
        "function chooseWifi(s){document.getElementById('ssid').value=s;document.getElementById('wifiList').style.display='none';}"
        "async function testWeather(button){"
        "const result=document.getElementById('weatherTestResult');"
        "button.disabled=true;button.textContent='Teste...';"
        "result.style.display='block';result.className='info';result.textContent='WLAN und Wetterzugang werden geprüft...';"
        "try{"
        "const body=new URLSearchParams({"
        "ssid:document.getElementById('ssid').value,"
        "password:document.querySelector('input[name=password]').value,"
        "latitude:document.querySelector('input[name=latitude]').value,"
        "longitude:document.querySelector('input[name=longitude]').value,"
        "weatherApiKey:document.getElementById('weatherApiKey').value"
        "});"
        "const response=await fetch('/weather-test',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body});"
        "const data=await response.json();"
        "if(!response.ok)throw new Error(data.error||('HTTP '+response.status));"
        "result.className='info ok';"
        "result.textContent='OK: '+Number(data.temperature).toFixed(1)+' °C, '+data.humidity+' % Feuchte, '+data.description;"
        "}catch(error){"
        "result.className='info err';result.textContent='Fehler: '+error.message;"
        "}finally{button.disabled=false;button.textContent='Wetterzugang testen';}"
        "}"
        "async function scanWifi(b){b.disabled=true;b.textContent='Suche...';const l=document.getElementById('wifiList');l.style.display='block';l.innerHTML='<div style=\"padding:10px\">Suche...</div>';try{const r=await fetch('/networks',{cache:'no-store'});const d=await r.json();if(!r.ok)throw new Error();l.innerHTML=(d.networks||[]).map(function(n){return '<button type=\"button\" style=\"display:flex;justify-content:space-between;width:100%;padding:11px;border:0;border-top:1px solid #2b3a31;background:#223128;color:#edf5ef;text-align:left\" onclick=\"chooseWifi('+JSON.stringify(n.ssid).replace(/\"/g,'&quot;')+')\"><span>'+n.ssid.replace(/&/g,'&amp;').replace(/</g,'&lt;')+'</span><span>'+n.rssi+' dBm '+(n.secure?'LOCK':'offen')+'</span></button>';}).join('')||'<div style=\"padding:10px\">Keine Netze gefunden</div>';}catch(e){l.innerHTML='<div style=\"padding:10px;color:#ff9e98\">WLAN-Suche fehlgeschlagen</div>';}b.disabled=false;b.textContent='Neu suchen';}"
        "</script>"
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

String SetupPortal::jsonEscape(const String& value)
{
    String escaped;
    escaped.reserve(value.length() + 8);

    for (size_t i = 0; i < value.length(); ++i)
    {
        const char c = value[i];
        switch (c)
        {
            case '\\': escaped += F("\\\\"); break;
            case '"': escaped += F("\\\""); break;
            case '\n': escaped += F("\\n"); break;
            case '\r': escaped += F("\\r"); break;
            case '\t': escaped += F("\\t"); break;
            default: escaped += c; break;
        }
    }

    return escaped;
}
