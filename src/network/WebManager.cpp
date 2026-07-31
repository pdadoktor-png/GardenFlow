#include "network/WebManager.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "app/AppConfig.h"
#include "hardware/ValveManager.h"
#include "runtime/RuntimeManager.h"
#include "scheduler/Scheduler.h"
#include "time/TimeManager.h"

namespace
{
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>GardenFlow</title>
<style>
:root{font-family:system-ui,-apple-system,sans-serif;color-scheme:dark;background:#101714;color:#edf5ef}
body{margin:0;max-width:960px;padding:18px;margin:auto}.top{display:flex;justify-content:space-between;gap:12px;align-items:center;flex-wrap:wrap}
h1{margin:0;font-size:1.7rem}.muted{color:#a8b7ad}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px;margin-top:16px}
.card{background:#18231d;border:1px solid #2b3a31;border-radius:14px;padding:15px;box-shadow:0 6px 18px #0004}.big{font-size:1.55rem;font-weight:700}
button{border:0;border-radius:10px;padding:10px 14px;font-weight:700;cursor:pointer;background:#7fda98;color:#102016}button.stop{background:#ff8b84;color:#2c1110}
button.secondary{background:#33463a;color:#edf5ef}button:disabled{opacity:.45;cursor:not-allowed}.row{display:flex;gap:8px;align-items:center;justify-content:space-between;margin:8px 0}
.badge{padding:4px 9px;border-radius:999px;background:#304237;font-size:.82rem}.ok{background:#285c38}.warn{background:#745b23}.off{background:#4d3a3a}
.program{border-top:1px solid #314138;padding:11px 0}.program:first-child{border-top:0}.days{font-size:.86rem;color:#b5c3ba}.error{color:#ff9e98}
</style>
</head>
<body>
<div class="top"><div><h1>GardenFlow</h1><div class="muted" id="address">wird verbunden …</div></div><div id="clock" class="big">--:--</div></div>
<div class="grid">
  <section class="card"><div class="muted">System</div><div class="row"><span>WLAN</span><span id="wifi" class="badge">--</span></div><div class="row"><span>Zeit</span><span id="timeState" class="badge">--</span></div><div class="row"><span>Automatik</span><span id="autoState" class="badge">--</span></div></section>
  <section class="card"><div class="muted">Aktueller Lauf</div><div id="running" class="big">Kein Programm</div><div id="remaining" class="muted">--</div><div style="margin-top:12px"><button class="stop" id="stop" onclick="post('/api/stop')">STOPP</button></div></section>
  <section class="card"><div class="muted">Ventile</div><div id="valves"></div></section>
</div>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Programme</div><div class="big">Bewässerungsplan</div></div><div><button onclick="newProgram()">+ Neu</button> <button class="secondary" onclick="loadAll()">Aktualisieren</button></div></div><div id="programs"></div></section>
<script>
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
async function api(url,opt){const r=await fetch(url,opt);const t=await r.text();if(!r.ok)throw new Error(t||r.status);return t?JSON.parse(t):{};}
async function post(url,data){try{const opt={method:'POST'};if(data){opt.headers={'Content-Type':'application/x-www-form-urlencoded'};opt.body=new URLSearchParams(data)}await api(url,opt);await loadAll()}catch(e){alert(e.message)}}
function badge(id,text,cls){const e=document.getElementById(id);e.textContent=text;e.className='badge '+cls}
async function loadStatus(){try{const s=await api('/api/status');document.getElementById('clock').textContent=s.date+' '+s.time;document.getElementById('address').textContent=s.ssid+' · '+s.ip+' · '+s.rssi+' dBm';badge('wifi',s.wifi?'verbunden':'getrennt',s.wifi?'ok':'off');badge('timeState',s.timeValid?'synchronisiert':'wartet',s.timeValid?'ok':'warn');badge('autoState',s.timeValid?'bereit':'gesperrt',s.timeValid?'ok':'warn');document.getElementById('running').textContent=s.running?('Programm '+s.programId+' · Ventil '+(s.valve+1)):'Kein Programm';document.getElementById('remaining').textContent=s.running?(s.remaining+' Sekunden verbleibend'):'Bereit';document.getElementById('stop').disabled=!s.running;document.getElementById('valves').innerHTML=s.valves.map(v=>`<div class="row"><span>${esc(v.name)}</span><span><span class="badge ${v.open?'ok':'off'}">${v.open?'OFFEN':'ZU'}</span> <button class="secondary" ${s.running?'disabled':''} onclick="post('/api/valve/toggle?index=${v.index}')">Impuls</button></span></div>`).join('')}catch(e){document.getElementById('address').innerHTML='<span class="error">Verbindung unterbrochen</span>'}}
let programCache=[];
async function loadPrograms(){try{const p=await api('/api/programs');programCache=p.programs;document.getElementById('programs').innerHTML=p.programs.length?p.programs.map(x=>`<div class="program"><div class="row"><div><b>Programm ${x.id}</b> · Ventil ${x.valve+1}<div class="days">${esc(x.days)} · ${String(x.hour).padStart(2,'0')}:${String(x.minute).padStart(2,'0')} · ${x.durationMinutes} min · ${x.enabled?'aktiv':'inaktiv'}</div></div><div><button class="secondary" onclick="editProgram(${x.index})">Bearbeiten</button> <button class="secondary" onclick="post('/api/program/copy',{index:${x.index}})">Kopieren</button> <button class="secondary" onclick="post('/api/program/toggle',{index:${x.index}})">${x.enabled?'Aus':'Ein'}</button> <button class="stop" onclick="deleteProgram(${x.index})">Löschen</button> <button ${(!x.enabled||p.running)?'disabled':''} onclick="post('/api/program/start?index=${x.index}')">Start</button></div></div></div>`).join(''):'<div class="muted">Keine Programme vorhanden</div>'}catch(e){document.getElementById('programs').innerHTML='<div class="error">Programme konnten nicht geladen werden</div>'}}
function readProgramInput(x){const valve=prompt('Ventil (1 oder 2)',String((x?.valve??0)+1));if(valve===null)return null;const time=prompt('Startzeit HH:MM',`${String(x?.hour??6).padStart(2,'0')}:${String(x?.minute??0).padStart(2,'0')}`);if(time===null)return null;const duration=prompt('Dauer in Minuten',String(x?.durationMinutes??15));if(duration===null)return null;const days=prompt('Wochentage als Bitmaske: Mo=1, Di=2, Mi=4, Do=8, Fr=16, Sa=32, So=64; täglich=127',String(x?.weekdays??127));if(days===null)return null;const m=/^(\d{1,2}):(\d{2})$/.exec(time);if(!m){alert('Ungültige Uhrzeit');return null}return{valve:Number(valve)-1,hour:Number(m[1]),minute:Number(m[2]),duration:Number(duration),days:Number(days),enabled:x?.enabled?1:0};}
async function newProgram(){const d=readProgramInput(null);if(!d)return;await post('/api/program/create',d)}
async function editProgram(index){const x=programCache.find(p=>p.index===index);const d=readProgramInput(x);if(!d)return;d.index=index;d.enabled=x.enabled?1:0;await post('/api/program/update',d)}
async function deleteProgram(index){if(confirm('Programm wirklich löschen?'))await post('/api/program/delete',{index})}
async function loadAll(){await Promise.all([loadStatus(),loadPrograms()])}loadAll();setInterval(loadStatus,2000);setInterval(loadPrograms,15000);
</script>
</body></html>
)HTML";
}

void WebManager::begin(Scheduler& scheduler,
                       RuntimeManager& runtimeManager,
                       ValveManager& valveManager,
                       TimeManager& timeManager)
{
    scheduler_ = &scheduler;
    runtimeManager_ = &runtimeManager;
    valveManager_ = &valveManager;
    timeManager_ = &timeManager;
    configureRoutes();
    Serial.println("WebManager initialisiert");
}

void WebManager::update()
{
    if (!timeManager_)
    {
        return;
    }

    const bool connected = timeManager_->isWifiConnected();
    if (connected && !wifiWasConnected_)
    {
        wifiWasConnected_ = true;
        startServices();
    }
    else if (!connected && wifiWasConnected_)
    {
        wifiWasConnected_ = false;
        started_ = false;
        otaStarted_ = false;
        server_.stop();
        Serial.println("Webserver pausiert: WLAN getrennt");
    }

    if (started_)
    {
        server_.handleClient();
    }
    if (otaStarted_)
    {
        ArduinoOTA.handle();
    }
}

bool WebManager::isStarted() const
{
    return started_;
}

void WebManager::startServices()
{
    if (!started_)
    {
        server_.begin();
        started_ = true;
        Serial.printf("Weboberflaeche: http://%s/\n", WiFi.localIP().toString().c_str());
    }
    if (!otaStarted_)
    {
        configureOta();
        ArduinoOTA.begin();
        otaStarted_ = true;
        Serial.printf("OTA bereit: %s.local\n", AppConfig::HOSTNAME);
    }
}

void WebManager::configureRoutes()
{
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server_.on("/api/programs", HTTP_GET, [this]() { handlePrograms(); });
    server_.on("/api/program/create", HTTP_POST, [this]() { handleCreateProgram(); });
    server_.on("/api/program/update", HTTP_POST, [this]() { handleUpdateProgram(); });
    server_.on("/api/program/delete", HTTP_POST, [this]() { handleDeleteProgram(); });
    server_.on("/api/program/copy", HTTP_POST, [this]() { handleCopyProgram(); });
    server_.on("/api/program/toggle", HTTP_POST, [this]() { handleToggleProgram(); });
    server_.on("/api/program/start", HTTP_POST, [this]() { handleStartProgram(); });
    server_.on("/api/stop", HTTP_POST, [this]() { handleStop(); });
    server_.on("/api/valve/toggle", HTTP_POST, [this]() { handleToggleValve(); });
    server_.onNotFound([this]() { handleNotFound(); });
}

void WebManager::configureOta()
{
    ArduinoOTA.setHostname(AppConfig::HOSTNAME);
    if (AppConfig::OTA_PASSWORD[0] != '\0')
    {
        ArduinoOTA.setPassword(AppConfig::OTA_PASSWORD);
    }

    ArduinoOTA.onStart([]() { Serial.println("OTA-Update gestartet"); });
    ArduinoOTA.onEnd([]() { Serial.println("\nOTA-Update abgeschlossen"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        if (total > 0)
        {
            Serial.printf("OTA: %u%%\r", static_cast<unsigned>((progress * 100U) / total));
        }
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA-Fehler %u\n", static_cast<unsigned>(error));
    });
}

void WebManager::handleRoot()
{
    server_.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void WebManager::handleStatus()
{
    if (!timeManager_ || !runtimeManager_ || !valveManager_ || !scheduler_)
    {
        sendJson(503, "{\"error\":\"System nicht bereit\"}");
        return;
    }

    char timeText[8] = "--:--";
    char dateText[16] = "--.--.----";
    timeManager_->formatTime(timeText, sizeof(timeText));
    timeManager_->formatDate(dateText, sizeof(dateText));

    String body;
    body.reserve(1100);
    body += F("{\"wifi\":");
    body += timeManager_->isWifiConnected() ? F("true") : F("false");
    body += F(",\"timeValid\":");
    body += timeManager_->isValid() ? F("true") : F("false");
    body += F(",\"ssid\":\"");
    body += jsonEscape(timeManager_->wifiSsid());
    body += F("\",\"ip\":\"");
    body += jsonEscape(timeManager_->ipAddress());
    body += F("\",\"rssi\":");
    body += String(timeManager_->wifiRssi());
    body += F(",\"time\":\"");
    body += timeText;
    body += F("\",\"date\":\"");
    body += dateText;
    body += F("\",\"running\":");
    body += runtimeManager_->isRunning() ? F("true") : F("false");
    body += F(",\"programId\":");
    if (runtimeManager_->isRunning())
    {
        body += String(scheduler_->programId(static_cast<uint8_t>(runtimeManager_->runningProgramIndex())));
    }
    else
    {
        body += '0';
    }
    body += F(",\"valve\":");
    body += String(runtimeManager_->runningValveIndex());
    body += F(",\"remaining\":");
    body += String(runtimeManager_->remainingSeconds());
    body += F(",\"automatic\":");
    body += runtimeManager_->isAutomaticRun() ? F("true") : F("false");
    body += F(",\"valves\":[");

    for (uint8_t i = 0; i < AppConfig::DISPLAYED_VALVE_COUNT; ++i)
    {
        if (i > 0) body += ',';
        const auto& valve = valveManager_->channel(i);
        body += F("{\"index\":");
        body += String(i);
        body += F(",\"name\":\"");
        body += jsonEscape(String(valve.name));
        body += F("\",\"open\":");
        body += valve.assumedOpen ? F("true") : F("false");
        body += F(",\"pulseActive\":");
        body += valve.pulseActive ? F("true") : F("false");
        body += '}';
    }
    body += F("]}");
    sendJson(200, body);
}

void WebManager::handlePrograms()
{
    if (!scheduler_ || !runtimeManager_)
    {
        sendJson(503, "{\"error\":\"System nicht bereit\"}");
        return;
    }

    String body;
    body.reserve(2200);
    body += F("{\"running\":");
    body += runtimeManager_->isRunning() ? F("true") : F("false");
    body += F(",\"programs\":[");
    bool first = true;
    for (uint8_t i = 0; i < Scheduler::MAX_PROGRAMS; ++i)
    {
        if (!scheduler_->isProgramUsed(i)) continue;
        const auto& p = scheduler_->program(i);
        if (!first) body += ',';
        first = false;
        body += F("{\"index\":");
        body += String(i);
        body += F(",\"id\":");
        body += String(p.id);
        body += F(",\"valve\":");
        body += String(p.valveIndex);
        body += F(",\"hour\":");
        body += String(p.startHour);
        body += F(",\"minute\":");
        body += String(p.startMinute);
        body += F(",\"durationMinutes\":");
        body += String(p.durationSeconds / 60UL);
        body += F(",\"enabled\":");
        body += p.enabled ? F("true") : F("false");
        body += F(",\"weekdays\":");
        body += String(p.weekdays);
        body += F(",\"days\":\"");
        body += weekdayText(p.weekdays);
        body += F("\"}");
    }
    body += F("]}");
    sendJson(200, body);
}

void WebManager::handleCreateProgram()
{
    if (!scheduler_)
    {
        sendJson(503, "{\"error\":\"System nicht bereit\"}");
        return;
    }
    const int valve = server_.hasArg("valve") ? server_.arg("valve").toInt() : 0;
    const int16_t index = scheduler_->createProgram(static_cast<uint8_t>(valve));
    if (index < 0)
    {
        sendJson(409, "{\"error\":\"Kein freier Programmplatz\"}");
        return;
    }
    if (server_.hasArg("hour") && server_.hasArg("minute"))
        scheduler_->setStartTime(static_cast<uint8_t>(index), server_.arg("hour").toInt(), server_.arg("minute").toInt());
    if (server_.hasArg("duration"))
        scheduler_->setDurationMinutes(static_cast<uint8_t>(index), server_.arg("duration").toInt());
    if (server_.hasArg("days"))
    {
        const uint8_t mask = static_cast<uint8_t>(server_.arg("days").toInt()) & 0x7F;
        for (uint8_t d = 0; d < 7; ++d)
            scheduler_->setWeekday(static_cast<uint8_t>(index), static_cast<Scheduler::Weekday>(d), (mask & (1U << d)) != 0);
    }
    sendJson(200, String("{\"ok\":true,\"index\":") + String(index) + "}");
}

void WebManager::handleUpdateProgram()
{
    if (!scheduler_ || !server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    const int index = server_.arg("index").toInt();
    if (index < 0 || index >= Scheduler::MAX_PROGRAMS || !scheduler_->isProgramUsed(static_cast<uint8_t>(index)))
    {
        sendJson(404, "{\"error\":\"Programm nicht gefunden\"}");
        return;
    }
    const uint8_t i = static_cast<uint8_t>(index);
    bool ok = true;
    if (server_.hasArg("valve")) ok &= scheduler_->setValve(i, server_.arg("valve").toInt());
    if (server_.hasArg("hour") && server_.hasArg("minute")) ok &= scheduler_->setStartTime(i, server_.arg("hour").toInt(), server_.arg("minute").toInt());
    if (server_.hasArg("duration")) ok &= scheduler_->setDurationMinutes(i, server_.arg("duration").toInt());
    if (server_.hasArg("days"))
    {
        const uint8_t mask = static_cast<uint8_t>(server_.arg("days").toInt()) & 0x7F;
        for (uint8_t d = 0; d < 7; ++d)
            ok &= scheduler_->setWeekday(i, static_cast<Scheduler::Weekday>(d), (mask & (1U << d)) != 0);
    }
    if (server_.hasArg("enabled")) ok &= scheduler_->setProgramEnabled(i, server_.arg("enabled").toInt() != 0);
    if (!ok)
    {
        sendJson(400, "{\"error\":\"Ungültige Programmdaten\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleDeleteProgram()
{
    if (!scheduler_ || !server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    if (!scheduler_->deleteProgram(static_cast<uint8_t>(server_.arg("index").toInt())))
    {
        sendJson(409, "{\"error\":\"Programm konnte nicht gelöscht werden\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleCopyProgram()
{
    if (!scheduler_ || !server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    const int sourceIndex = server_.arg("index").toInt();
    if (sourceIndex < 0 || sourceIndex >= Scheduler::MAX_PROGRAMS || !scheduler_->isProgramUsed(static_cast<uint8_t>(sourceIndex)))
    {
        sendJson(404, "{\"error\":\"Programm nicht gefunden\"}");
        return;
    }
    const auto source = scheduler_->program(static_cast<uint8_t>(sourceIndex));
    const int16_t targetIndex = scheduler_->createProgram(source.valveIndex);
    if (targetIndex < 0)
    {
        sendJson(409, "{\"error\":\"Kein freier Programmplatz\"}");
        return;
    }
    const uint8_t t = static_cast<uint8_t>(targetIndex);
    scheduler_->setStartTime(t, source.startHour, source.startMinute);
    scheduler_->setDurationMinutes(t, static_cast<uint16_t>(source.durationSeconds / 60UL));
    for (uint8_t d = 0; d < 7; ++d)
        scheduler_->setWeekday(t, static_cast<Scheduler::Weekday>(d), (source.weekdays & (1U << d)) != 0);
    scheduler_->setProgramEnabled(t, source.enabled);
    sendJson(200, String("{\"ok\":true,\"index\":") + String(targetIndex) + "}");
}

void WebManager::handleToggleProgram()
{
    if (!scheduler_ || !server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    const int index = server_.arg("index").toInt();
    if (index < 0 || index >= Scheduler::MAX_PROGRAMS || !scheduler_->isProgramUsed(static_cast<uint8_t>(index)))
    {
        sendJson(404, "{\"error\":\"Programm nicht gefunden\"}");
        return;
    }
    const uint8_t i = static_cast<uint8_t>(index);
    if (!scheduler_->setProgramEnabled(i, !scheduler_->program(i).enabled))
    {
        sendJson(409, "{\"error\":\"Programmstatus konnte nicht geändert werden\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleStartProgram()
{
    if (!server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    const int index = server_.arg("index").toInt();
    if (index < 0 || index >= Scheduler::MAX_PROGRAMS || !scheduler_->isProgramUsed(static_cast<uint8_t>(index)))
    {
        sendJson(404, "{\"error\":\"Programm nicht gefunden\"}");
        return;
    }
    if (!runtimeManager_->startProgram(static_cast<uint8_t>(index), false))
    {
        sendJson(409, "{\"error\":\"Programm kann momentan nicht gestartet werden\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleStop()
{
    if (!runtimeManager_->isRunning())
    {
        sendJson(200, "{\"ok\":true,\"running\":false}");
        return;
    }
    if (!runtimeManager_->stop())
    {
        sendJson(409, "{\"error\":\"STOPP momentan nicht moeglich; Ventilimpuls laeuft\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleToggleValve()
{
    if (runtimeManager_->isRunning())
    {
        sendJson(409, "{\"error\":\"Ventilbedienung waehrend eines Programmlaufs gesperrt\"}");
        return;
    }
    if (!server_.hasArg("index"))
    {
        sendJson(400, "{\"error\":\"index fehlt\"}");
        return;
    }
    const int index = server_.arg("index").toInt();
    if (index < 0 || index >= AppConfig::DISPLAYED_VALVE_COUNT)
    {
        sendJson(404, "{\"error\":\"Ventil nicht gefunden\"}");
        return;
    }
    if (!valveManager_->toggle(static_cast<uint8_t>(index)))
    {
        sendJson(409, "{\"error\":\"Ventilimpuls momentan nicht moeglich\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleNotFound()
{
    sendJson(404, "{\"error\":\"Nicht gefunden\"}");
}

void WebManager::sendJson(int code, const String& body)
{
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(code, "application/json; charset=utf-8", body);
}

String WebManager::jsonEscape(const String& value)
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

String WebManager::weekdayText(uint8_t mask)
{
    static const char* names[7] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
    if ((mask & 0x7F) == 0x7F) return String("Taeglich");
    String result;
    for (uint8_t i = 0; i < 7; ++i)
    {
        if ((mask & (1U << i)) == 0) continue;
        if (result.length() > 0) result += ' ';
        result += names[i];
    }
    return result.isEmpty() ? String("Nie") : result;
}
