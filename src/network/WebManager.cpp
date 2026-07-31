#include "network/WebManager.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "app/AppConfig.h"
#include "hardware/ValveManager.h"
#include "runtime/RuntimeManager.h"
#include "scheduler/Scheduler.h"
#include "time/TimeManager.h"
#include "weather/WeatherManager.h"
#include "smart/SmartControlManager.h"

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
.modal{display:none;position:fixed;inset:0;background:#000a;align-items:center;justify-content:center;padding:16px;z-index:20}.modal.open{display:flex}.dialog{width:min(520px,100%);max-height:92vh;overflow:auto;background:#18231d;border:1px solid #3b5143;border-radius:16px;padding:18px}.formgrid{display:grid;grid-template-columns:1fr 1fr;gap:12px}.field{display:flex;flex-direction:column;gap:6px}.field.full{grid-column:1/-1}input,select{border:1px solid #46594c;border-radius:9px;background:#101714;color:#edf5ef;padding:10px;font-size:1rem}.weekdays{display:grid;grid-template-columns:repeat(7,1fr);gap:6px}.day{padding:9px 4px;background:#33463a;color:#edf5ef}.day.active{background:#7fda98;color:#102016}.actions{display:flex;justify-content:flex-end;gap:8px;margin-top:16px}@media(max-width:540px){.formgrid{grid-template-columns:1fr}.field.full{grid-column:auto}.weekdays{grid-template-columns:repeat(4,1fr)}}
</style>
</head>
<body>
<div class="top"><div><h1>GardenFlow</h1><div class="muted" id="address">wird verbunden …</div></div><div id="clock" class="big">--:--</div></div>
<div class="grid">
  <section class="card"><div class="muted">System</div><div class="row"><span>WLAN</span><span id="wifi" class="badge">--</span></div><div class="row"><span>Zeit</span><span id="timeState" class="badge">--</span></div><div class="row"><span>Automatik</span><span id="autoState" class="badge">--</span></div></section>
  <section class="card"><div class="muted">Wetter</div><div id="weatherMain" class="big">nicht eingerichtet</div><div id="weatherDetails" class="muted">API-Schluessel fehlt</div><div class="row"><span>Regenpause</span><span id="rainPause" class="badge">--</span></div><div style="margin-top:10px"><button class="secondary" onclick="post('/api/weather/refresh')">Aktualisieren</button></div></section>
  <section class="card"><div class="muted">Aktueller Lauf</div><div id="running" class="big">Kein Programm</div><div id="remaining" class="muted">--</div><div style="margin-top:12px"><button class="stop" id="stop" onclick="post('/api/stop')">STOPP</button></div></section>
  <section class="card"><div class="muted">Ventile</div><div id="valves"></div></section>
</div>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Wettersteuerung</div><div class="big">Automatische Regenpause</div></div></div><div class="formgrid" style="margin-top:12px"><label class="field"><span>Automatik</span><select id="weatherEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label><label class="field"><span>Regenmenge 24 h (mm)</span><input id="weatherRainMm" type="number" min="0.1" max="100" step="0.1"></label><label class="field"><span>Regenwahrscheinlichkeit (%)</span><input id="weatherPop" type="number" min="1" max="100"></label><div class="field"><span>&nbsp;</span><button onclick="saveWeatherSettings()">Speichern</button></div></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Smart Control</div><div class="big">Saison & Urlaub</div></div><span id="vacationState" class="badge">--</span></div><div class="formgrid" style="margin-top:12px"><label class="field"><span>Saisonfaktor (%)</span><input id="seasonPercent" type="number" min="10" max="200" step="5"></label><label class="field"><span>Urlaubsmodus</span><select id="vacationEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label><label class="field"><span>Start</span><input id="vacationStart" type="date"></label><label class="field"><span>Ende</span><input id="vacationEnd" type="date"></label><label class="field"><span>Bewässern alle</span><select id="vacationEvery"><option value="1">jeden Tag</option><option value="2">2 Tage</option><option value="3">3 Tage</option><option value="4">4 Tage</option><option value="5">5 Tage</option><option value="6">6 Tage</option><option value="7">7 Tage</option></select></label><label class="field"><span>Laufzeit im Urlaub (%)</span><input id="vacationPercent" type="number" min="10" max="100" step="5"></label><div class="field full"><button onclick="saveSmartSettings()">Smart-Einstellungen speichern</button></div></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Programme</div><div class="big">Bewässerungsplan</div></div><div><button onclick="newProgram()">+ Neu</button> <button class="secondary" onclick="loadAll()">Aktualisieren</button></div></div><div id="programs"></div></section>
<div id="editorModal" class="modal" onclick="modalBackdrop(event)"><div class="dialog">
  <div class="top"><div><div class="muted">Programm</div><div id="editorTitle" class="big">Neu</div></div><button class="secondary" onclick="closeEditor()">Schließen</button></div>
  <div class="formgrid" style="margin-top:16px">
    <label class="field"><span>Ventil</span><select id="editValve"><option value="0">Ventil 1</option><option value="1">Ventil 2</option></select></label>
    <label class="field"><span>Startzeit</span><input id="editTime" type="time" value="06:00"></label>
    <label class="field"><span>Dauer (Minuten)</span><input id="editDuration" type="number" min="1" max="1440" value="15"></label>
    <label class="field"><span>Status</span><select id="editEnabled"><option value="1">Aktiv</option><option value="0">Inaktiv</option></select></label>
    <div class="field full"><span>Wochentage</span><div id="weekdayButtons" class="weekdays"></div></div>
  </div>
  <div class="actions"><button class="secondary" onclick="closeEditor()">Abbrechen</button><button onclick="saveEditor()">Speichern</button></div>
</div></div>
<script>
const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
async function api(url,opt){const r=await fetch(url,opt);const t=await r.text();if(!r.ok)throw new Error(t||r.status);return t?JSON.parse(t):{};}
async function post(url,data){try{const opt={method:'POST'};if(data){opt.headers={'Content-Type':'application/x-www-form-urlencoded'};opt.body=new URLSearchParams(data)}await api(url,opt);await loadAll()}catch(e){alert(e.message)}}
function badge(id,text,cls){const e=document.getElementById(id);e.textContent=text;e.className='badge '+cls}
function dateKeyToInput(v){const s=String(v||0).padStart(8,'0');return v?`${s.slice(0,4)}-${s.slice(4,6)}-${s.slice(6,8)}`:''}
async function loadStatus(){try{const s=await api('/api/status');document.getElementById('clock').textContent=s.date+' '+s.time;document.getElementById('address').textContent=s.ssid+' · '+s.ip+' · '+s.rssi+' dBm';badge('wifi',s.wifi?'verbunden':'getrennt',s.wifi?'ok':'off');badge('timeState',s.timeValid?'synchronisiert':'wartet',s.timeValid?'ok':'warn');badge('autoState',s.rainPause?'Regenpause':(s.timeValid?'bereit':'gesperrt'),s.rainPause?'warn':(s.timeValid?'ok':'warn'));document.getElementById('weatherMain').textContent=s.weatherValid?(s.temperature.toFixed(1)+' °C · '+s.weatherDescription):(s.weatherConfigured?'wartet auf Daten':'nicht eingerichtet');document.getElementById('weatherDetails').textContent=s.weatherValid?('Feuchte '+s.humidity+' % · Regen '+s.rainMm.toFixed(1)+' mm/24h · Risiko '+s.rainProbability+' %'):(s.weatherError||'OpenWeather API-Schluessel eintragen');badge('rainPause',s.rainPause?'AKTIV':(s.weatherPauseEnabled?'bereit':'aus'),s.rainPause?'warn':(s.weatherPauseEnabled?'ok':'off'));document.getElementById('weatherEnabled').value=s.weatherPauseEnabled?'1':'0';document.getElementById('weatherRainMm').value=s.weatherRainLimit;document.getElementById('weatherPop').value=s.weatherProbabilityLimit;document.getElementById('seasonPercent').value=s.seasonPercent;document.getElementById('vacationEnabled').value=s.vacationEnabled?'1':'0';document.getElementById('vacationStart').value=dateKeyToInput(s.vacationStart);document.getElementById('vacationEnd').value=dateKeyToInput(s.vacationEnd);document.getElementById('vacationEvery').value=String(s.vacationEvery);document.getElementById('vacationPercent').value=s.vacationPercent;badge('vacationState',s.vacationActive?'AKTIV':(s.vacationEnabled?'geplant':'aus'),s.vacationActive?'warn':(s.vacationEnabled?'ok':'off'));document.getElementById('running').textContent=s.running?('Programm '+s.programId+' · Ventil '+(s.valve+1)):'Kein Programm';document.getElementById('remaining').textContent=s.running?(s.remaining+' Sekunden verbleibend'):'Bereit';document.getElementById('stop').disabled=!s.running;document.getElementById('valves').innerHTML=s.valves.map(v=>`<div class="row"><span>${esc(v.name)}</span><span><span class="badge ${v.open?'ok':'off'}">${v.open?'OFFEN':'ZU'}</span> <button class="secondary" ${s.running?'disabled':''} onclick="post('/api/valve/toggle?index=${v.index}')">Impuls</button></span></div>`).join('')}catch(e){document.getElementById('address').innerHTML='<span class="error">Verbindung unterbrochen</span>'}}
let programCache=[];
async function loadPrograms(){try{const p=await api('/api/programs');programCache=p.programs;document.getElementById('programs').innerHTML=p.programs.length?p.programs.map(x=>`<div class="program"><div class="row"><div><b>Programm ${x.id}</b> · Ventil ${x.valve+1}<div class="days">${esc(x.days)} · ${String(x.hour).padStart(2,'0')}:${String(x.minute).padStart(2,'0')} · ${x.durationMinutes} min · ${x.enabled?'aktiv':'inaktiv'}</div></div><div><button class="secondary" onclick="editProgram(${x.index})">Bearbeiten</button> <button class="secondary" onclick="post('/api/program/copy',{index:${x.index}})">Kopieren</button> <button class="secondary" onclick="post('/api/program/toggle',{index:${x.index}})">${x.enabled?'Aus':'Ein'}</button> <button class="stop" onclick="deleteProgram(${x.index})">Löschen</button> <button ${(!x.enabled||p.running)?'disabled':''} onclick="post('/api/program/start?index=${x.index}')">Start</button></div></div></div>`).join(''):'<div class="muted">Keine Programme vorhanden</div>'}catch(e){document.getElementById('programs').innerHTML='<div class="error">Programme konnten nicht geladen werden</div>'}}
let editorIndex=-1,editorDays=127;
const dayNames=['Mo','Di','Mi','Do','Fr','Sa','So'];
function renderWeekdays(){document.getElementById('weekdayButtons').innerHTML=dayNames.map((n,i)=>`<button type="button" class="day ${(editorDays&(1<<i))?'active':''}" onclick="toggleEditorDay(${i})">${n}</button>`).join('')}
function toggleEditorDay(i){editorDays^=(1<<i);renderWeekdays()}
function openEditor(x){editorIndex=x?x.index:-1;editorDays=x?x.weekdays:127;document.getElementById('editorTitle').textContent=x?`Programm ${x.id} bearbeiten`:'Neues Programm';document.getElementById('editValve').value=String(x?x.valve:0);document.getElementById('editTime').value=`${String(x?x.hour:6).padStart(2,'0')}:${String(x?x.minute:0).padStart(2,'0')}`;document.getElementById('editDuration').value=String(x?x.durationMinutes:15);document.getElementById('editEnabled').value=x&&x.enabled?'1':'0';renderWeekdays();document.getElementById('editorModal').classList.add('open')}
function closeEditor(){document.getElementById('editorModal').classList.remove('open')}
function modalBackdrop(e){if(e.target.id==='editorModal')closeEditor()}
async function saveEditor(){const time=document.getElementById('editTime').value.split(':');const duration=Number(document.getElementById('editDuration').value);if(time.length!==2||duration<1||duration>1440||editorDays===0){alert(editorDays===0?'Mindestens einen Wochentag auswählen':'Bitte gültige Werte eingeben');return}const d={valve:Number(document.getElementById('editValve').value),hour:Number(time[0]),minute:Number(time[1]),duration,days:editorDays,enabled:Number(document.getElementById('editEnabled').value)};if(editorIndex<0)await post('/api/program/create',d);else{d.index=editorIndex;await post('/api/program/update',d)}closeEditor()}
function newProgram(){openEditor(null)}
function editProgram(index){openEditor(programCache.find(p=>p.index===index))}
async function deleteProgram(index){if(confirm('Programm wirklich löschen?'))await post('/api/program/delete',{index})}
async function saveWeatherSettings(){await post('/api/weather/settings',{enabled:Number(document.getElementById('weatherEnabled').value),rainMm:Number(document.getElementById('weatherRainMm').value),probability:Number(document.getElementById('weatherPop').value)})}
async function saveSmartSettings(){const start=document.getElementById('vacationStart').value.replaceAll('-','');const end=document.getElementById('vacationEnd').value.replaceAll('-','');await post('/api/smart/settings',{season:Number(document.getElementById('seasonPercent').value),enabled:Number(document.getElementById('vacationEnabled').value),start,end,every:Number(document.getElementById('vacationEvery').value),percent:Number(document.getElementById('vacationPercent').value)})}
async function loadAll(){await Promise.all([loadStatus(),loadPrograms()])}loadAll();setInterval(loadStatus,2000);setInterval(loadPrograms,15000);
</script>
</body></html>
)HTML";
}

void WebManager::begin(Scheduler& scheduler,
                       RuntimeManager& runtimeManager,
                       ValveManager& valveManager,
                       TimeManager& timeManager,
                       WeatherManager& weatherManager,
                       SmartControlManager& smartControlManager)
{
    scheduler_ = &scheduler;
    runtimeManager_ = &runtimeManager;
    valveManager_ = &valveManager;
    timeManager_ = &timeManager;
    weatherManager_ = &weatherManager;
    smartControlManager_ = &smartControlManager;
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
    server_.on("/api/weather/refresh", HTTP_POST, [this]() { handleWeatherRefresh(); });
    server_.on("/api/weather/settings", HTTP_POST, [this]() { handleWeatherSettings(); });
    server_.on("/api/smart/settings", HTTP_POST, [this]() { handleSmartSettings(); });
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
    if (!timeManager_ || !runtimeManager_ || !valveManager_ || !scheduler_ || !weatherManager_ || !smartControlManager_)
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
    body += F(",\"weatherConfigured\":");
    body += weatherManager_->isConfigured() ? F("true") : F("false");
    body += F(",\"weatherValid\":");
    body += weatherManager_->isValid() ? F("true") : F("false");
    body += F(",\"temperature\":");
    body += String(weatherManager_->temperatureC(), 1);
    body += F(",\"humidity\":");
    body += String(weatherManager_->humidityPercent());
    body += F(",\"rainMm\":");
    body += String(weatherManager_->rainMmNext24Hours(), 1);
    body += F(",\"rainProbability\":");
    body += String(weatherManager_->maxRainProbabilityPercent());
    body += F(",\"weatherDescription\":\"");
    body += jsonEscape(weatherManager_->description());
    body += F("\",\"weatherError\":\"");
    body += jsonEscape(weatherManager_->lastError());
    body += F("\",\"rainPause\":");
    body += weatherManager_->automaticPauseActive() ? F("true") : F("false");
    body += F(",\"weatherPauseEnabled\":");
    body += weatherManager_->automaticPauseEnabled() ? F("true") : F("false");
    body += F(",\"weatherRainLimit\":");
    body += String(weatherManager_->rainLimitMm(), 1);
    body += F(",\"weatherProbabilityLimit\":");
    body += String(weatherManager_->probabilityLimitPercent());
    struct tm smartLocal = {};
    timeManager_->getLocalTime(smartLocal);
    body += F(",\"seasonPercent\":");
    body += String(smartControlManager_->seasonPercent());
    body += F(",\"vacationEnabled\":");
    body += smartControlManager_->vacationEnabled() ? F("true") : F("false");
    body += F(",\"vacationActive\":");
    body += smartControlManager_->vacationActive(smartLocal) ? F("true") : F("false");
    body += F(",\"vacationStart\":");
    body += String(smartControlManager_->vacationStartDate());
    body += F(",\"vacationEnd\":");
    body += String(smartControlManager_->vacationEndDate());
    body += F(",\"vacationEvery\":");
    body += String(smartControlManager_->vacationIntervalDays());
    body += F(",\"vacationPercent\":");
    body += String(smartControlManager_->vacationPercent());
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

void WebManager::handleWeatherRefresh()
{
    if (!weatherManager_)
    {
        sendJson(503, "{\"error\":\"Wettermodul nicht bereit\"}");
        return;
    }
    if (!weatherManager_->isConfigured())
    {
        sendJson(409, "{\"error\":\"OpenWeather API-Schluessel fehlt\"}");
        return;
    }
    if (!weatherManager_->refreshNow())
    {
        sendJson(409, String("{\"error\":\"") + jsonEscape(weatherManager_->lastError()) + "\"}");
        return;
    }
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleWeatherSettings()
{
    if (!weatherManager_)
    {
        sendJson(503, "{\"error\":\"Wettermodul nicht bereit\"}");
        return;
    }

    if (server_.hasArg("enabled"))
        weatherManager_->setAutomaticPauseEnabled(server_.arg("enabled").toInt() != 0);
    if (server_.hasArg("rainMm"))
        weatherManager_->setRainLimitMm(server_.arg("rainMm").toFloat());
    if (server_.hasArg("probability"))
        weatherManager_->setProbabilityLimitPercent(static_cast<uint8_t>(server_.arg("probability").toInt()));

    sendJson(200, "{\"ok\":true}");
}


void WebManager::handleSmartSettings()
{
    if (!smartControlManager_)
    {
        sendJson(503, "{\"error\":\"Smart-Control nicht bereit\"}");
        return;
    }

    if (server_.hasArg("season"))
        smartControlManager_->setSeasonPercent(
            static_cast<uint8_t>(server_.arg("season").toInt())
        );

    if (server_.hasArg("enabled"))
        smartControlManager_->setVacationEnabled(
            server_.arg("enabled").toInt() != 0
        );

    if (server_.hasArg("start") && server_.hasArg("end"))
    {
        const uint32_t startDate =
            static_cast<uint32_t>(server_.arg("start").toInt());
        const uint32_t endDate =
            static_cast<uint32_t>(server_.arg("end").toInt());

        if (SmartControlManager::validDateKey(startDate) &&
            SmartControlManager::validDateKey(endDate) &&
            endDate >= startDate)
        {
            smartControlManager_->setVacationDates(startDate, endDate);
        }
        else if (smartControlManager_->vacationEnabled())
        {
            sendJson(400, "{\"error\":\"Ungueltiger Urlaubszeitraum\"}");
            return;
        }
    }

    if (server_.hasArg("every"))
        smartControlManager_->setVacationIntervalDays(
            static_cast<uint8_t>(server_.arg("every").toInt())
        );

    if (server_.hasArg("percent"))
        smartControlManager_->setVacationPercent(
            static_cast<uint8_t>(server_.arg("percent").toInt())
        );

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
