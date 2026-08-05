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
#include "log/LogManager.h"
#include "settings/SettingsManager.h"
#include "advisor/AdvisorEngine.h"
#include "water/WaterManager.h"

namespace
{
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>GardenFlow by HK 2026 V1.0</title>
<style>
:root{font-family:system-ui,-apple-system,sans-serif;color-scheme:dark;background:#101714;color:#edf5ef}
body{margin:0;max-width:960px;padding:18px;margin:auto}.top{display:flex;justify-content:space-between;gap:12px;align-items:center;flex-wrap:wrap}
h1{margin:0;font-size:1.7rem}.muted{color:#a8b7ad}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px;margin-top:16px}
.card{background:#18231d;border:1px solid #2b3a31;border-radius:14px;padding:15px;box-shadow:0 6px 18px #0004}.big{font-size:1.55rem;font-weight:700}
button{border:0;border-radius:10px;padding:10px 14px;font-weight:700;cursor:pointer;background:#7fda98;color:#102016}button.stop{background:#ff8b84;color:#2c1110}
button.secondary{background:#33463a;color:#edf5ef}button:disabled{opacity:.45;cursor:not-allowed}.row{display:flex;gap:8px;align-items:center;justify-content:space-between;margin:8px 0}
.badge{padding:4px 9px;border-radius:999px;background:#304237;font-size:.82rem}.ok{background:#285c38}.warn{background:#745b23}.off{background:#4d3a3a}
.program{border-top:1px solid #314138;padding:11px 0}.program:first-child{border-top:0}.days{font-size:.86rem;color:#b5c3ba}.error{color:#ff9e98}
.nextProgram{margin-top:12px;border:1px solid #41604c;background:linear-gradient(135deg,#21352a,#17231d)}
.nextProgramTime{font-size:2rem;font-weight:850;margin-top:6px}.nextProgramMeta{margin-top:5px;color:#c3d1c7}
.scheduleGroup{margin-top:14px}.scheduleTitle{font-weight:850;font-size:1.05rem;margin:10px 0 6px}
.scheduleItem{display:grid;grid-template-columns:74px 1fr auto;gap:10px;align-items:center;border-top:1px solid #314138;padding:10px 0}
.scheduleItem:first-child{border-top:0}.scheduleTime{font-size:1.1rem;font-weight:850}.scheduleValve{font-size:.8rem;color:#a8b7ad}
.programInactive{opacity:.55}
.dashboardHero{margin-top:16px;background:linear-gradient(135deg,#20362a,#15221b);border:1px solid #45604d}
.dashboardGrid{display:grid;grid-template-columns:1.2fr 1fr 1fr;gap:12px;margin-top:14px}
.dashboardBlock{background:#111a15aa;border:1px solid #304237;border-radius:12px;padding:13px}
.dashboardLabel{font-size:.78rem;text-transform:uppercase;letter-spacing:.08em;color:#9fb2a5}
.dashboardValue{font-size:1.35rem;font-weight:800;margin-top:5px}
.dashboardSub{font-size:.86rem;color:#b5c3ba;margin-top:4px}
.dashboardValve{display:flex;justify-content:space-between;gap:8px;margin-top:7px}.advisorCard{grid-column:1/-1;border-color:#6b8f72;background:linear-gradient(135deg,#1e3928,#14251b)}.advisorHeadline{font-size:1.4rem;font-weight:850;margin-top:5px}.advisorReasons{margin-top:9px;display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:6px;color:#c1d1c5}.advisorNarrative{margin-top:12px;padding:11px;border-radius:10px;background:#102017;color:#d8e5da}.advisorFactors{margin-top:12px;border:1px solid #355140;border-radius:10px;overflow:hidden}.advisorFactor{display:grid;grid-template-columns:1.2fr 1fr auto;gap:10px;padding:9px 11px;border-top:1px solid #2b4033}.advisorFactor:first-child{border-top:0}.advisorConfidence{margin-top:12px;display:flex;align-items:center;gap:10px}.confidenceBar{height:9px;flex:1;background:#293a30;border-radius:99px;overflow:hidden}.confidenceFill{height:100%;background:#7fda98}.advisorDuration{margin-top:12px;font-size:1.05rem;font-weight:750}.waterGrid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:10px}.waterValue{font-size:1.2rem;font-weight:800}@media(max-width:620px){.waterGrid{grid-template-columns:repeat(2,1fr)}}
@media(max-width:760px){.dashboardGrid{grid-template-columns:1fr}}

.setupNote{margin-top:8px;color:#a8b7ad;font-size:.86rem}
.setupGrid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:12px}
@media(max-width:620px){.setupGrid{grid-template-columns:1fr}}


.modal{display:none;position:fixed;inset:0;background:#000a;align-items:center;justify-content:center;padding:16px;z-index:20}.modal.open{display:flex}.dialog{width:min(520px,100%);max-height:92vh;overflow:auto;background:#18231d;border:1px solid #3b5143;border-radius:16px;padding:18px}.formgrid{display:grid;grid-template-columns:1fr 1fr;gap:12px}.field{display:flex;flex-direction:column;gap:6px}.field.full{grid-column:1/-1}input,select{border:1px solid #46594c;border-radius:9px;background:#101714;color:#edf5ef;padding:10px;font-size:1rem}.weekdays{display:grid;grid-template-columns:repeat(7,1fr);gap:6px}.day{padding:9px 4px;background:#33463a;color:#edf5ef}.day.active{background:#7fda98;color:#102016}.actions{display:flex;justify-content:flex-end;gap:8px;margin-top:16px}.saveState{min-height:1.4em;margin-top:10px;color:#a8b7ad}.saveState.okmsg{color:#7fda98}.saveState.errmsg{color:#ff9e98}.dirtyMark{color:#ffd27a;font-weight:700}.logTools{display:flex;gap:8px;flex-wrap:wrap;margin-top:10px}.logList{margin-top:10px;max-height:420px;overflow:auto;border:1px solid #304237;border-radius:10px}.logRow{display:grid;grid-template-columns:150px 90px 1fr;gap:8px;padding:8px 10px;border-top:1px solid #2b3a31;font-size:.88rem}.logRow:first-child{border-top:0}.logRow.warning{background:#5a451f55}.logRow.error{background:#66312d66}.logCategory{color:#9fb2a5}.logMessage{word-break:break-word}@media(max-width:620px){.logRow{grid-template-columns:1fr}.logCategory{font-size:.78rem}}@media(max-width:540px){.formgrid{grid-template-columns:1fr}.field.full{grid-column:auto}.weekdays{grid-template-columns:repeat(4,1fr)}}
</style>
</head>
<body>
<div class="top"><div><h1>GardenFlow by HK 2026 V1.0</h1><div class="muted" id="address">wird verbunden …</div></div><div id="clock" class="big">--:--</div></div>
<section class="card dashboardHero">
  <div class="top">
    <div>
      <div class="muted">Dashboard</div>
      <div id="dashState" class="big">System wird geladen …</div>
    </div>
    <span id="dashStateBadge" class="badge">--</span>
  </div>
  <div class="dashboardGrid">
    <div class="dashboardBlock">
      <div class="dashboardLabel">Nächstes Programm</div>
      <div id="dashNextTime" class="dashboardValue">--:--</div>
      <div id="dashNextMeta" class="dashboardSub">Kein aktives Programm geplant</div>
    </div>
    <div class="dashboardBlock">
      <div class="dashboardLabel">Wetter</div>
      <div id="dashWeather" class="dashboardValue">--</div>
      <div id="dashWeatherMeta" class="dashboardSub">Noch keine Wetterdaten</div>
    </div>
    <div class="dashboardBlock">
      <div class="dashboardLabel">Ventile</div>
      <div id="dashValves" class="dashboardSub">Status wird geladen …</div>
    </div>
    <div class="dashboardBlock advisorCard">
      <div class="top">
        <div>
          <div class="dashboardLabel">GardenFlow Advisor</div>
          <div id="advisorHeadline" class="advisorHeadline">Wird ausgewertet …</div>
        </div>
        <span id="advisorBadge" class="badge">--</span>
      </div>
      <div id="advisorSummary" class="dashboardSub">Noch keine Empfehlung</div>
      <div id="advisorNarrative" class="advisorNarrative">Wetterdaten werden ausgewertet …</div>
      <div class="dashboardLabel" style="margin-top:12px">Warum?</div>
      <div id="advisorFactors" class="advisorFactors"></div>
      <div id="advisorDuration" class="advisorDuration"></div>
      <div class="advisorConfidence"><span id="advisorConfidenceText">Vertrauen: --</span><div class="confidenceBar"><div id="advisorConfidenceFill" class="confidenceFill" style="width:0%"></div></div></div>
      <div id="advisorReasons" class="advisorReasons"></div>
    </div>
  </div>
  <div class="dashboardSub" style="margin-top:12px">
    Letztes Ereignis: <span id="dashLastEvent">--</span>
  </div>
</section>
<div class="grid">
  <section class="card"><div class="muted">System</div><div class="row"><span>WLAN</span><span id="wifi" class="badge">--</span></div><div class="row"><span>Zeit</span><span id="timeState" class="badge">--</span></div><div class="row"><span>Automatik</span><span id="autoState" class="badge">--</span></div></section>
  <section class="card"><div class="muted">Wetter</div><div id="weatherMain" class="big">nicht eingerichtet</div><div id="weatherDetails" class="muted">API-Schluessel fehlt</div><div class="row"><span>Regenpause</span><span id="rainPause" class="badge">--</span></div><div style="margin-top:10px"><button class="secondary" onclick="post('/api/weather/refresh')">Aktualisieren</button></div></section>
  <section class="card"><div class="muted">Aktueller Lauf</div><div id="running" class="big">Kein Programm</div><div id="remaining" class="muted">--</div><div style="margin-top:12px"><button class="stop" id="stop" onclick="post('/api/stop')">STOPP</button></div></section>
  <section class="card"><div class="muted">Ventile</div><div id="valves"></div></section>
  <section class="card" style="grid-column:1/-1;border-color:#4e765b">
    <div class="top">
      <div>
        <div class="muted">Wasserbilanz</div>
        <div class="big">Verbrauch und Kosten</div>
      </div>
      <span id="waterRunBadge" class="badge off">BEREIT</span>
    </div>
    <div class="waterGrid">
      <div><div class="dashboardLabel">Heute</div><div id="waterToday" class="waterValue">0,0 l</div></div>
      <div><div class="dashboardLabel">Woche</div><div id="waterWeek" class="waterValue">0,0 l</div></div>
      <div><div class="dashboardLabel">Monat</div><div id="waterMonth" class="waterValue">0,0 l</div></div>
      <div><div class="dashboardLabel">Jahr</div><div id="waterYear" class="waterValue">0,0 l</div></div>
    </div>
    <div class="row"><span>Aktueller Lauf</span><strong id="waterCurrentRun">0,0 l</strong></div>
    <div class="row"><span>Eingespart</span><span id="waterSaved" class="badge ok">0,0 l</span></div>
    <div class="row"><span>Kosten heute</span><span id="waterTodayCost">0,00 €</span></div>
    <div class="setupNote">Die Zähler werden beim Programmende dauerhaft gespeichert. Während eines laufenden Programms wird der aktuelle Verbrauch bereits live angezeigt.</div>
  </section>
</div>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Wettersteuerung <span id="weatherDirtyMark" class="dirtyMark"></span></div><div class="big">Automatische Regenpause</div></div></div><div class="formgrid" style="margin-top:12px"><label class="field"><span>Automatik</span><select id="weatherEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label><label class="field"><span>Regenmenge 24 h (mm)</span><input id="weatherRainMm" type="number" min="0.1" max="100" step="0.1"></label><label class="field"><span>Regenwahrscheinlichkeit (%)</span><input id="weatherPop" type="number" min="1" max="100"></label><div class="field"><span>&nbsp;</span><div><button class="secondary" onclick="cancelWeatherSettings()">Abbrechen</button> <button onclick="saveWeatherSettings()">Speichern</button></div></div></div><div id="weatherSaveState" class="saveState"></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Smart Control <span id="smartDirtyMark" class="dirtyMark"></span></div><div class="big">Saison & Urlaub</div></div><span id="vacationState" class="badge">--</span></div><div class="formgrid" style="margin-top:12px"><label class="field"><span>Saisonfaktor (%)</span><input id="seasonPercent" type="number" min="10" max="200" step="5"></label><label class="field"><span>Urlaubsmodus</span><select id="vacationEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label><label class="field"><span>Start</span><input id="vacationStart" type="date"></label><label class="field"><span>Ende</span><input id="vacationEnd" type="date"></label><label class="field"><span>Bewässern alle</span><select id="vacationEvery"><option value="1">jeden Tag</option><option value="2">2 Tage</option><option value="3">3 Tage</option><option value="4">4 Tage</option><option value="5">5 Tage</option><option value="6">6 Tage</option><option value="7">7 Tage</option></select></label><label class="field"><span>Laufzeit im Urlaub (%)</span><input id="vacationPercent" type="number" min="10" max="100" step="5"></label><div class="field full"><button class="secondary" onclick="cancelSmartSettings()">Abbrechen</button> <button onclick="saveSmartSettings()">Smart-Einstellungen speichern</button></div></div><div id="smartSaveState" class="saveState"></div></section>
<section class="card nextProgram"><div class="top"><div><div class="muted">Nächstes Programm</div><div id="nextProgramTime" class="nextProgramTime">--:--</div><div id="nextProgramMeta" class="nextProgramMeta">Kein aktives Programm geplant</div></div><span id="nextProgramWhen" class="badge">--</span></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Zeitplan</div><div class="big">Heute, morgen und diese Woche</div></div><button class="secondary" onclick="loadAll()">Aktualisieren</button></div><div id="upcomingPrograms"></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Programme</div><div class="big">Alle Programme</div></div><button onclick="newProgram()">+ Neu</button></div><div id="programs"></div></section>
<section class="card" style="margin-top:12px">
<div class="top"><div><div class="muted">Wasserbilanz</div><div class="big">Durchfluss & Kosten</div></div></div>
<div class="formgrid" style="margin-top:12px">
<label class="field"><span>Ventil 1 (Liter/Minute)</span><input id="waterFlow1" type="number" min="0" max="250" step="0.1"></label>
<label class="field"><span>Ventil 2 (Liter/Minute)</span><input id="waterFlow2" type="number" min="0" max="250" step="0.1"></label>
<label class="field"><span>Wasserpreis (Euro/m³)</span><input id="waterPrice" type="number" min="0" max="100" step="0.01"></label>
<div class="field"><span>&nbsp;</span><div><button onclick="saveWaterSettings()">Speichern</button> <button class="stop" onclick="resetWaterStatistics()">Zähler löschen</button></div></div>
</div>
<div id="waterSaveState" class="saveState"></div>
</section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Setup</div><div class="big">WLAN und Standort</div></div><span id="setupState" class="badge">--</span></div><div class="setupGrid"><label class="field"><span>WLAN-Name (SSID)</span><input id="setupSsid" maxlength="32" autocomplete="off"></label><label class="field"><span>WLAN-Passwort</span><input id="setupPassword" type="password" placeholder="leer = unverändert" autocomplete="new-password"></label><label class="field"><span>Breitengrad</span><input id="setupLatitude" type="number" min="-90" max="90" step="0.00001"></label><label class="field"><span>Längengrad</span><input id="setupLongitude" type="number" min="-180" max="180" step="0.00001"></label><label class="field full"><span>Zeitzone (POSIX)</span><input id="setupTimezone" value="CET-1CEST,M3.5.0/2,M10.5.0/3"></label></div><div class="setupNote">Deutschland: Der voreingestellte Zeitzonenwert berücksichtigt Sommer- und Winterzeit automatisch. Nach dem Speichern startet GardenFlow neu.</div><div style="margin-top:12px"><button onclick="saveSetup()">WLAN und Standort speichern</button> <button class="secondary" onclick="startSetupPortal()">Setup-Portal starten</button></div><div id="setupSaveState" class="saveState"></div></section>
<section class="card" style="margin-top:12px"><div class="top"><div><div class="muted">Diagnose</div><div class="big">Ereignisprotokoll</div></div><span id="logCount" class="badge">0</span></div><div class="logTools"><select id="logFilter" onchange="renderLog()"><option value="">Alle Kategorien</option><option>System</option><option>WLAN</option><option>Zeit</option><option>Wetter</option><option>Programm</option><option>Ventil</option><option>Scheduler</option><option>Fehler</option></select><input id="logSearch" placeholder="Suchen" oninput="renderLog()"><button class="secondary" onclick="loadLog()">Aktualisieren</button><button class="stop" onclick="clearLog()">Löschen</button></div><div id="logList" class="logList"><div class="muted" style="padding:10px">Protokoll wird geladen …</div></div></section>
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
async function post(url,data){
    try{
        const opt={method:'POST'};
        if(data){
            opt.headers={'Content-Type':'application/x-www-form-urlencoded'};
            opt.body=new URLSearchParams(data);
        }
        await api(url,opt);

        /*
         * Nach Änderungen zuerst die Programmliste laden.
         * Damit verwenden Zeitplan und „Nächstes Programm“
         * niemals einen alten programCache.
         */
        if(url.startsWith('/api/program/')){
            await loadPrograms();
            await loadStatus();
            await loadLog();
        }else{
            await loadStatus();
            await loadPrograms();
            await loadLog();
        }
    }catch(e){
        alert(e.message);
    }
}
async function toggleValve(index,button){
    if(button){button.disabled=true;button.textContent='Schaltet…';}
    try{
        await api('/api/valve/toggle?index='+index,{method:'POST'});
        await loadStatus();
        setTimeout(loadStatus,1200);
    }catch(e){
        alert(e.message);
        await loadStatus();
    }
}
function badge(id,text,cls){const e=document.getElementById(id);e.textContent=text;e.className='badge '+cls}
function dateKeyToInput(v){const s=String(v||0).padStart(8,'0');return v?`${s.slice(0,4)}-${s.slice(4,6)}-${s.slice(6,8)}`:''}
let weatherDirty=false,smartDirty=false,lastStatus=null;
const weatherFields=['weatherEnabled','weatherRainMm','weatherPop'];
const smartFields=['seasonPercent','vacationEnabled','vacationStart','vacationEnd','vacationEvery','vacationPercent'];
function setSaveState(id,text,kind=''){const e=document.getElementById(id);e.textContent=text;e.className='saveState '+kind}
function markWeatherDirty(){weatherDirty=true;document.getElementById('weatherDirtyMark').textContent='• ungespeichert';setSaveState('weatherSaveState','Änderungen noch nicht gespeichert')}
function markSmartDirty(){smartDirty=true;document.getElementById('smartDirtyMark').textContent='• ungespeichert';setSaveState('smartSaveState','Änderungen noch nicht gespeichert')}
function fillWeatherForm(s){document.getElementById('weatherEnabled').value=s.weatherPauseEnabled?'1':'0';document.getElementById('weatherRainMm').value=s.weatherRainLimit;document.getElementById('weatherPop').value=s.weatherProbabilityLimit}
function fillSmartForm(s){document.getElementById('seasonPercent').value=s.seasonPercent;document.getElementById('vacationEnabled').value=s.vacationEnabled?'1':'0';document.getElementById('vacationStart').value=dateKeyToInput(s.vacationStart);document.getElementById('vacationEnd').value=dateKeyToInput(s.vacationEnd);document.getElementById('vacationEvery').value=String(s.vacationEvery);document.getElementById('vacationPercent').value=s.vacationPercent}
function cancelWeatherSettings(){if(lastStatus)fillWeatherForm(lastStatus);weatherDirty=false;document.getElementById('weatherDirtyMark').textContent='';setSaveState('weatherSaveState','Änderungen verworfen')}
function cancelSmartSettings(){if(lastStatus)fillSmartForm(lastStatus);smartDirty=false;document.getElementById('smartDirtyMark').textContent='';setSaveState('smartSaveState','Änderungen verworfen')}
function bindSettingsForms(){weatherFields.forEach(id=>{const e=document.getElementById(id);e.addEventListener('input',markWeatherDirty);e.addEventListener('change',markWeatherDirty)});smartFields.forEach(id=>{const e=document.getElementById(id);e.addEventListener('input',markSmartDirty);e.addEventListener('change',markSmartDirty)})}
window.addEventListener('beforeunload',e=>{if(weatherDirty||smartDirty){e.preventDefault();e.returnValue=''}});
function signedPercent(value){const number=Number(value||0);return (number>0?'+':'')+number+' %';}
function confidenceText(value){if(value>=85)return 'Sehr sicher';if(value>=70)return 'Sicher';if(value>=50)return 'Mittlere Sicherheit';return 'Geringe Sicherheit';}
function advisorNextDuration(s){if(!s.advisorValid||!programCache.length)return '';const now=parseControllerNow();const next=allOccurrences(now,8)[0];if(!next)return '';const base=Number(next.program.durationMinutes||0);const factor=Math.max(0,100+Number(s.advisorAdjustment||0))/100;const recommended=s.advisorAdjustment<=-60?0:Math.max(1,Math.round(base*factor));return `Empfohlene Laufzeit für Programm ${next.program.id}: ${base} min → ${recommended} min`;}
function updateAdvisor(s){
 const headline=document.getElementById('advisorHeadline'),summary=document.getElementById('advisorSummary'),narrative=document.getElementById('advisorNarrative'),badge=document.getElementById('advisorBadge'),factors=document.getElementById('advisorFactors'),reasons=document.getElementById('advisorReasons'),duration=document.getElementById('advisorDuration'),confidence=document.getElementById('advisorConfidenceText'),confidenceFill=document.getElementById('advisorConfidenceFill');
 headline.textContent=s.advisorHeadline||'Keine Empfehlung';summary.textContent=s.advisorSummary||'';narrative.textContent=s.advisorNarrative||'';
 if(!s.advisorValid){badge.textContent='WARTET';badge.className='badge off';}else if(s.advisorAdjustment<=-60){badge.textContent='PAUSE';badge.className='badge warn';}else if(s.advisorAdjustment<0){badge.textContent=s.advisorAdjustment+' %';badge.className='badge warn';}else if(s.advisorAdjustment>0){badge.textContent='+'+s.advisorAdjustment+' %';badge.className='badge ok';}else{badge.textContent='NORMAL';badge.className='badge ok';}
 factors.innerHTML=(s.advisorFactors||[]).map(f=>{const c=Number(f.contribution||0),cls=c>0?'ok':(c<0?'warn':'off');return `<div class="advisorFactor"><span>${esc(f.name)}</span><span>${esc(f.value)}</span><span class="badge ${cls}">${signedPercent(c)}</span></div>`;}).join('')||'<div class="dashboardSub" style="padding:10px">Noch keine Einzelfaktoren verfügbar</div>';
 reasons.innerHTML=(s.advisorReasons||[]).map(reason=>`<div>✓ ${esc(reason)}</div>`).join('');duration.textContent=advisorNextDuration(s);const cv=Number(s.advisorConfidence||0);confidence.textContent=`Vertrauen: ${cv} % · ${confidenceText(cv)}`;confidenceFill.style.width=Math.max(0,Math.min(100,cv))+'%';
}
function updateDashboard(s){
    const state=document.getElementById('dashState');
    const stateBadge=document.getElementById('dashStateBadge');

    if(s.running){
        state.textContent='Bewässerung läuft';
        stateBadge.textContent='AKTIV';
        stateBadge.className='badge warn';
    }else if(s.rainPause){
        state.textContent='Automatik pausiert';
        stateBadge.textContent='REGENPAUSE';
        stateBadge.className='badge warn';
    }else if(!s.wifi){
        state.textContent='WLAN getrennt';
        stateBadge.textContent='OFFLINE';
        stateBadge.className='badge off';
    }else if(!s.timeValid){
        state.textContent='Warte auf Systemzeit';
        stateBadge.textContent='WARTET';
        stateBadge.className='badge warn';
    }else{
        state.textContent='System bereit';
        stateBadge.textContent='BEREIT';
        stateBadge.className='badge ok';
    }

    document.getElementById('dashWeather').textContent=
        s.weatherValid
            ? s.temperature.toFixed(1)+' °C'
            : 'nicht verfügbar';

    document.getElementById('dashWeatherMeta').textContent=
        s.weatherValid
            ? s.weatherDescription+' · Regen '+s.rainProbability+' %'
            : (s.weatherError||'Wetterdaten fehlen');

    document.getElementById('dashValves').innerHTML=
        s.valves.map(v=>`<div class="dashboardValve"><span>${esc(v.name)}</span><span class="badge ${v.pulseActive?'warn':(v.open?'ok':'off')}">${v.pulseActive?'SCHALTET':(v.open?'OFFEN':'ZU')}</span></div>`).join('');
}
function updateWater(s){
    const liters=value=>Number(value||0).toFixed(1).replace('.',',')+' l';
    const euro=value=>Number(value||0).toFixed(2).replace('.',',')+' €';
    const live=Number(s.waterCurrentRun||0);
    const storedToday=Number(s.waterToday||0);

    const setText=(id,value)=>{
        const element=document.getElementById(id);
        if(element)element.textContent=value;
    };

    setText('waterToday',liters(storedToday+live));
    setText('waterWeek',liters(Number(s.waterWeek||0)+live));
    setText('waterMonth',liters(Number(s.waterMonth||0)+live));
    setText('waterYear',liters(Number(s.waterYear||0)+live));
    setText('waterCurrentRun',liters(live));
    setText('waterSaved',liters(s.waterSaved));
    setText('waterTodayCost',euro(Number(s.waterTodayCost||0)+Number(s.waterCurrentCost||0)));

    const runBadge=document.getElementById('waterRunBadge');
    if(runBadge){
        runBadge.textContent=s.running?'LÄUFT':'BEREIT';
        runBadge.className='badge '+(s.running?'warn':'ok');
    }

    const flow1=document.getElementById('waterFlow1');
    const flow2=document.getElementById('waterFlow2');
    const price=document.getElementById('waterPrice');

    if(flow1&&document.activeElement!==flow1){
        flow1.value=Number(s.waterFlow1||0).toFixed(1);
    }
    if(flow2&&document.activeElement!==flow2){
        flow2.value=Number(s.waterFlow2||0).toFixed(1);
    }
    if(price&&document.activeElement!==price){
        price.value=Number(s.waterPrice||0).toFixed(2);
    }
}

async function saveWaterSettings(){
    try{
        const flow1=Number(document.getElementById('waterFlow1').value);
        const flow2=Number(document.getElementById('waterFlow2').value);
        const price=Number(document.getElementById('waterPrice').value);

        if(!Number.isFinite(flow1)||flow1<0||flow1>250||
           !Number.isFinite(flow2)||flow2<0||flow2>250||
           !Number.isFinite(price)||price<0||price>100){
            setSaveState('waterSaveState','Bitte gültige Werte eingeben','errmsg');
            return;
        }

        await api('/api/water/settings',{
            method:'POST',
            headers:{'Content-Type':'application/x-www-form-urlencoded'},
            body:new URLSearchParams({flow1,flow2,price})
        });

        setSaveState('waterSaveState','Wasserdaten gespeichert','okmsg');
        await loadStatus();
    }catch(e){
        setSaveState('waterSaveState','Fehler: '+e.message,'errmsg');
    }
}

async function resetWaterStatistics(){
    if(!confirm('Alle Wasserzähler wirklich löschen?'))return;

    try{
        await api('/api/water/reset',{method:'POST'});
        setSaveState('waterSaveState','Wasserzähler gelöscht','okmsg');
        await loadStatus();
    }catch(e){
        setSaveState('waterSaveState','Fehler: '+e.message,'errmsg');
    }
}

async function loadStatus(){try{const s=await api('/api/status');document.getElementById('clock').textContent=s.date+' '+s.time;document.getElementById('address').textContent=s.ssid+' · '+s.ip+' · '+s.rssi+' dBm';badge('wifi',s.wifi?'verbunden':'getrennt',s.wifi?'ok':'off');badge('timeState',s.timeValid?'synchronisiert':'wartet',s.timeValid?'ok':'warn');badge('autoState',s.rainPause?'Regenpause':(s.timeValid?'bereit':'gesperrt'),s.rainPause?'warn':(s.timeValid?'ok':'warn'));document.getElementById('weatherMain').textContent=s.weatherValid?(s.temperature.toFixed(1)+' °C · '+s.weatherDescription):(s.weatherConfigured?'wartet auf Daten':'nicht eingerichtet');document.getElementById('weatherDetails').textContent=s.weatherValid?('Feuchte '+s.humidity+' % · Regen '+s.rainMm.toFixed(1)+' mm/24h · Risiko '+s.rainProbability+' %'):(s.weatherError||'OpenWeather API-Schluessel eintragen');badge('rainPause',s.rainPause?'AKTIV':(s.weatherPauseEnabled?'bereit':'aus'),s.rainPause?'warn':(s.weatherPauseEnabled?'ok':'off'));lastStatus=s;updateDashboard(s);updateAdvisor(s);updateWater(s);if(!weatherDirty)fillWeatherForm(s);if(!smartDirty)fillSmartForm(s);renderNextProgram();renderUpcomingPrograms();renderAllPrograms(s.running);badge('vacationState',s.vacationActive?'AKTIV':(s.vacationEnabled?'geplant':'aus'),s.vacationActive?'warn':(s.vacationEnabled?'ok':'off'));document.getElementById('running').textContent=s.running?('Programm '+s.programId+' · Ventil '+(s.valve+1)):'Kein Programm';document.getElementById('remaining').textContent=s.running?(s.remaining+' Sekunden verbleibend'):'Bereit';document.getElementById('stop').disabled=!s.running;document.getElementById('valves').innerHTML=s.valves.map(v=>`<div class="row"><span>${esc(v.name)}</span><span><span class="badge ${v.pulseActive?'warn':(v.open?'ok':'off')}">${v.pulseActive?'SCHALTET…':(v.open?'OFFEN':'GESCHLOSSEN')}</span> <button class="secondary" ${(s.running||v.pulseActive)?'disabled':''} onclick="toggleValve(${v.index},this)">Umschalten</button></span></div>`).join('')}catch(e){document.getElementById('address').innerHTML='<span class="error">Verbindung unterbrochen</span>'}}
let programCache=[];

function parseControllerNow(){
    if(!lastStatus||!lastStatus.date||!lastStatus.time)return new Date();
    const d=lastStatus.date.split('.');
    const t=lastStatus.time.split(':');
    if(d.length!==3||t.length<2)return new Date();
    return new Date(Number(d[2]),Number(d[1])-1,Number(d[0]),Number(t[0]),Number(t[1]),0,0);
}

function mondayZero(date){
    return (date.getDay()+6)%7;
}

function dayStart(date){
    return new Date(date.getFullYear(),date.getMonth(),date.getDate(),0,0,0,0);
}

function occurrenceForDay(program,baseDate,offset){
    const candidate=new Date(
        baseDate.getFullYear(),
        baseDate.getMonth(),
        baseDate.getDate()+offset,
        program.hour,
        program.minute,
        0,
        0
    );
    const bit=mondayZero(candidate);
    return (program.weekdays&(1<<bit))!==0?candidate:null;
}

function nextOccurrence(program,now){
    if(!program.enabled||!program.weekdays)return null;
    const base=dayStart(now);
    for(let offset=0;offset<8;offset++){
        const candidate=occurrenceForDay(program,base,offset);
        if(candidate&&candidate>now)return candidate;
    }
    return null;
}

function allOccurrences(now,days){
    const result=[];
    const base=dayStart(now);
    programCache.filter(p=>p.enabled).forEach(program=>{
        for(let offset=0;offset<days;offset++){
            const candidate=occurrenceForDay(program,base,offset);
            if(candidate&&candidate>now){
                result.push({program,date:candidate,offset});
            }
        }
    });
    result.sort((a,b)=>a.date-b.date||a.program.id-b.program.id);
    return result;
}

function sameCalendarDay(a,b){
    return a.getFullYear()===b.getFullYear()&&a.getMonth()===b.getMonth()&&a.getDate()===b.getDate();
}

function whenText(date,now){
    const tomorrow=new Date(now.getFullYear(),now.getMonth(),now.getDate()+1);
    if(sameCalendarDay(date,now))return 'Heute';
    if(sameCalendarDay(date,tomorrow))return 'Morgen';
    return ['So','Mo','Di','Mi','Do','Fr','Sa'][date.getDay()];
}

function formatTime(date){
    return String(date.getHours()).padStart(2,'0')+':'+String(date.getMinutes()).padStart(2,'0');
}

function durationUntil(date,now){
    let minutes=Math.max(0,Math.round((date-now)/60000));
    const days=Math.floor(minutes/1440);minutes-=days*1440;
    const hours=Math.floor(minutes/60);minutes-=hours*60;
    if(days>0)return `in ${days} T ${hours} Std`;
    if(hours>0)return `in ${hours} Std ${minutes} Min`;
    return `in ${minutes} Min`;
}

function renderNextProgram(){
    const now=parseControllerNow();
    const next=allOccurrences(now,8)[0];
    const timeEl=document.getElementById('nextProgramTime');
    const metaEl=document.getElementById('nextProgramMeta');
    const whenEl=document.getElementById('nextProgramWhen');

    if(!next){
        timeEl.textContent='--:--';
        metaEl.textContent='Kein aktives Programm geplant';
        whenEl.textContent='--';
        whenEl.className='badge off';
        document.getElementById('dashNextTime').textContent='--:--';
        document.getElementById('dashNextMeta').textContent='Kein aktives Programm geplant';
        return;
    }

    timeEl.textContent=formatTime(next.date);
    metaEl.textContent=`Programm ${next.program.id} · Ventil ${next.program.valve+1} · ${next.program.durationMinutes} min · ${durationUntil(next.date,now)}`;
    document.getElementById('dashNextTime').textContent=`${whenText(next.date,now)} ${formatTime(next.date)}`;
    document.getElementById('dashNextMeta').textContent=`Programm ${next.program.id} · Ventil ${next.program.valve+1} · ${next.program.durationMinutes} min · ${durationUntil(next.date,now)}`;
    whenEl.textContent=whenText(next.date,now);
    whenEl.className='badge ok';
}

function scheduleItemHtml(entry){
    const p=entry.program;
    return `<div class="scheduleItem"><div class="scheduleTime">${formatTime(entry.date)}</div><div><b>Programm ${p.id}</b><div class="scheduleValve">Ventil ${p.valve+1} · ${p.durationMinutes} min</div></div><button class="secondary" onclick="editProgram(${p.index})">Bearbeiten</button></div>`;
}

function renderUpcomingPrograms(){
    const target=document.getElementById('upcomingPrograms');
    if(!target)return;

    const now=parseControllerNow();
    const tomorrow=new Date(now.getFullYear(),now.getMonth(),now.getDate()+1);
    const occurrences=allOccurrences(now,7);

    const today=occurrences.filter(x=>sameCalendarDay(x.date,now));
    const tomorrowItems=occurrences.filter(x=>sameCalendarDay(x.date,tomorrow));
    const later=occurrences.filter(x=>!sameCalendarDay(x.date,now)&&!sameCalendarDay(x.date,tomorrow));

    const group=(title,items)=>items.length
        ? `<div class="scheduleGroup"><div class="scheduleTitle">${title}</div>${items.map(scheduleItemHtml).join('')}</div>`
        : '';

    target.innerHTML=
        group('Heute',today)+
        group('Morgen',tomorrowItems)+
        group('Später diese Woche',later) ||
        '<div class="muted" style="margin-top:12px">Keine aktiven Programme in den nächsten sieben Tagen</div>';
}

function renderAllPrograms(running){
    const target=document.getElementById('programs');
    if(!target)return;

    const now=parseControllerNow();
    const sorted=[...programCache].sort((a,b)=>{
        if(a.enabled!==b.enabled)return a.enabled?-1:1;
        const an=nextOccurrence(a,now);
        const bn=nextOccurrence(b,now);
        if(an&&bn)return an-bn;
        if(an)return -1;
        if(bn)return 1;
        return a.id-b.id;
    });

    target.innerHTML=sorted.length
        ? sorted.map(x=>{
            const next=nextOccurrence(x,now);
            const nextText=x.enabled&&next
                ? `Nächster Start: ${whenText(next,now)} ${formatTime(next)}`
                : (x.enabled?'Kein Termin':'Inaktiv');
            return `<div class="program ${x.enabled?'':'programInactive'}"><div class="row"><div><b>Programm ${x.id}</b> · Ventil ${x.valve+1}<div class="days">${esc(x.days)} · ${String(x.hour).padStart(2,'0')}:${String(x.minute).padStart(2,'0')} · ${x.durationMinutes} min · ${nextText}</div></div><div><button class="secondary" onclick="editProgram(${x.index})">Bearbeiten</button> <button class="secondary" onclick="post('/api/program/copy',{index:${x.index}})">Kopieren</button> <button class="secondary" onclick="post('/api/program/toggle',{index:${x.index}})">${x.enabled?'Aus':'Ein'}</button> <button class="stop" onclick="deleteProgram(${x.index})">Löschen</button> <button ${(!x.enabled||running)?'disabled':''} onclick="post('/api/program/start?index=${x.index}')">Start</button></div></div></div>`;
          }).join('')
        : '<div class="muted">Keine Programme vorhanden</div>';
}

async function loadPrograms(){
    try{
        const p=await api('/api/programs');
        programCache=p.programs;
        renderNextProgram();
        renderUpcomingPrograms();
        renderAllPrograms(p.running);
    }catch(e){
        document.getElementById('programs').innerHTML='<div class="error">Programme konnten nicht geladen werden</div>';
        document.getElementById('upcomingPrograms').innerHTML='<div class="error">Zeitplan konnte nicht geladen werden</div>';
    }
}
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
async function saveWeatherSettings(){try{setSaveState('weatherSaveState','Speichern …');await api('/api/weather/settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({enabled:Number(document.getElementById('weatherEnabled').value),rainMm:Number(document.getElementById('weatherRainMm').value),probability:Number(document.getElementById('weatherPop').value)})});weatherDirty=false;document.getElementById('weatherDirtyMark').textContent='';setSaveState('weatherSaveState','Einstellungen gespeichert','okmsg');await loadStatus()}catch(e){setSaveState('weatherSaveState','Fehler: '+e.message,'errmsg')}}
function dateInputToKey(id){
    const value=document.getElementById(id).value;
    if(!value)return 0;
    const parts=value.split('-');
    if(parts.length!==3)return 0;
    return Number(parts[0]+parts[1]+parts[2]);
}
async function saveSmartSettings(){
    try{
        const enabled=Number(document.getElementById('vacationEnabled').value);
        const start=dateInputToKey('vacationStart');
        const end=dateInputToKey('vacationEnd');

        if(enabled){
            if(!start||!end){
                setSaveState('smartSaveState','Bitte Start- und Enddatum eingeben','errmsg');
                return;
            }
            if(end<start){
                setSaveState('smartSaveState','Enddatum darf nicht vor dem Startdatum liegen','errmsg');
                return;
            }
        }

        setSaveState('smartSaveState','Speichern …');

        await api('/api/smart/settings',{
            method:'POST',
            headers:{'Content-Type':'application/x-www-form-urlencoded'},
            body:new URLSearchParams({
                season:Number(document.getElementById('seasonPercent').value),
                enabled:enabled,
                start:start,
                end:end,
                every:Number(document.getElementById('vacationEvery').value),
                percent:Number(document.getElementById('vacationPercent').value)
            })
        });

        smartDirty=false;
        document.getElementById('smartDirtyMark').textContent='';
        setSaveState('smartSaveState','Einstellungen gespeichert','okmsg');
        await loadStatus();
    }catch(e){
        setSaveState('smartSaveState','Fehler: '+e.message,'errmsg');
    }
}
let logCache=[];
async function loadLog(){
  try{
    const data=await api('/api/log');
    logCache=data.entries||[];
    document.getElementById('logCount').textContent=String(logCache.length);
    document.getElementById('dashLastEvent').textContent=
        logCache.length
            ? `${logCache[0].time} · ${logCache[0].message}`
            : 'Noch keine Ereignisse';
    renderLog();
  }catch(e){
    document.getElementById('logList').innerHTML='<div class="error" style="padding:10px">Protokoll konnte nicht geladen werden</div>';
  }
}
function renderLog(){
  const filter=document.getElementById('logFilter').value;
  const search=document.getElementById('logSearch').value.trim().toLowerCase();
  const rows=logCache.filter(x=>(!filter||x.category===filter)&&(!search||x.message.toLowerCase().includes(search)||x.category.toLowerCase().includes(search)));
  document.getElementById('logList').innerHTML=rows.length?rows.map(x=>`<div class="logRow ${x.level}"><div>${esc(x.time)}</div><div class="logCategory">${esc(x.category)}</div><div class="logMessage">${esc(x.message)}</div></div>`).join(''):'<div class="muted" style="padding:10px">Keine passenden Einträge</div>';
}
async function clearLog(){
  if(!confirm('Ereignisprotokoll wirklich löschen?'))return;
  try{await api('/api/log/clear',{method:'POST'});await loadLog()}catch(e){alert(e.message)}
}
let setupLoaded=false;
async function loadSetup(){
    try{
        const s=await api('/api/setup/settings');
        document.getElementById('setupSsid').value=s.ssid||'';
        document.getElementById('setupLatitude').value=Number(s.latitude).toFixed(5);
        document.getElementById('setupLongitude').value=Number(s.longitude).toFixed(5);
        document.getElementById('setupTimezone').value=s.timezone||'CET-1CEST,M3.5.0/2,M10.5.0/3';
        document.getElementById('setupPassword').value='';
        badge('setupState',s.passwordConfigured?'konfiguriert':'Passwort fehlt',s.passwordConfigured?'ok':'warn');
        setupLoaded=true;
    }catch(e){
        setSaveState('setupSaveState','Setup konnte nicht geladen werden: '+e.message,'errmsg');
    }
}
async function startSetupPortal(){
    if(!confirm('Setup-Portal starten? GardenFlow startet neu und wechselt in das WLAN GardenFlow-Setup.'))return;
    try{
        setSaveState('setupSaveState','Setup-Portal wird gestartet …');
        await api('/api/setup/portal/start',{method:'POST'});
        setSaveState('setupSaveState','Neustart läuft. Danach mit GardenFlow-Setup verbinden.','okmsg');
    }catch(e){
        setSaveState('setupSaveState','Fehler: '+e.message,'errmsg');
    }
}
async function saveSetup(){
    try{
        const latitude=Number(document.getElementById('setupLatitude').value);
        const longitude=Number(document.getElementById('setupLongitude').value);
        const ssid=document.getElementById('setupSsid').value.trim();
        const timezone=document.getElementById('setupTimezone').value.trim();
        if(!ssid||!Number.isFinite(latitude)||latitude<-90||latitude>90||!Number.isFinite(longitude)||longitude<-180||longitude>180||!timezone){
            setSaveState('setupSaveState','Bitte gültige WLAN- und Standortdaten eingeben','errmsg');
            return;
        }
        setSaveState('setupSaveState','Speichern; GardenFlow startet neu …');
        await api('/api/setup/save',{
            method:'POST',
            headers:{'Content-Type':'application/x-www-form-urlencoded'},
            body:new URLSearchParams({
                ssid:ssid,
                password:document.getElementById('setupPassword').value,
                latitude:latitude,
                longitude:longitude,
                timezone:timezone
            })
        });
        setSaveState('setupSaveState','Gespeichert. Neustart läuft …','okmsg');
    }catch(e){
        setSaveState('setupSaveState','Fehler: '+e.message,'errmsg');
    }
}
async function loadAll(){
    /*
     * Bewusst nacheinander statt parallel:
     * loadPrograms() aktualisiert programCache und zeichnet danach
     * Zeitplan sowie das nächste Programm neu.
     */
    await loadStatus();
    await loadPrograms();
    await loadLog();
}bindSettingsForms();loadSetup();loadAll();setInterval(loadStatus,2000);setInterval(loadPrograms,15000);setInterval(loadLog,5000);
</script>
</body></html>
)HTML";
}

void WebManager::begin(Scheduler& scheduler,
                       RuntimeManager& runtimeManager,
                       ValveManager& valveManager,
                       TimeManager& timeManager,
                       WeatherManager& weatherManager,
                       SmartControlManager& smartControlManager,
                       SettingsManager& settingsManager,
                       AdvisorEngine& advisorEngine,
                       WaterManager& waterManager)
{
    scheduler_ = &scheduler;
    runtimeManager_ = &runtimeManager;
    valveManager_ = &valveManager;
    timeManager_ = &timeManager;
    weatherManager_ = &weatherManager;
    smartControlManager_ = &smartControlManager;
    settingsManager_ = &settingsManager;
    advisorEngine_ = &advisorEngine;
    waterManager_ = &waterManager;
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

    if (restartRequestedAtMs_ != 0 &&
        static_cast<uint32_t>(millis() - restartRequestedAtMs_) >= 1200UL)
    {
        ESP.restart();
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
    server_.on("/api/log", HTTP_GET, [this]() { handleLog(); });
    server_.on("/api/log/clear", HTTP_POST, [this]() { handleLogClear(); });
    server_.on("/api/setup/settings", HTTP_GET, [this]() { handleSetupSettings(); });
    server_.on("/api/setup/save", HTTP_POST, [this]() { handleSetupSave(); });
    server_.on("/api/setup/portal/start", HTTP_POST, [this]() { handleSetupPortalStart(); });
    server_.on("/api/water/settings", HTTP_POST, [this]() { handleWaterSettings(); });
    server_.on("/api/water/reset", HTTP_POST, [this]() { handleWaterReset(); });
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
    if (!timeManager_ || !runtimeManager_ || !valveManager_ ||
        !scheduler_ || !weatherManager_ || !smartControlManager_ ||
        !advisorEngine_ || !waterManager_)
    {
        sendJson(503, "{\"error\":\"System nicht bereit\"}");
        return;
    }

    char timeText[8] = "--:--";
    char dateText[16] = "--.--.----";
    timeManager_->formatTime(timeText, sizeof(timeText));
    timeManager_->formatDate(dateText, sizeof(dateText));

    String body;
    body.reserve(3800);
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

    const auto& advisor =
        advisorEngine_->recommendation();

    body += F(",\"advisorValid\":");
    body += advisor.valid ? F("true") : F("false");
    body += F(",\"advisorAdjustment\":");
    body += String(advisor.adjustmentPercent);
    body += F(",\"advisorConfidence\":");
    body += String(advisor.confidencePercent);
    body += F(",\"advisorHeadline\":\"");
    body += jsonEscape(advisor.headline);
    body += F("\",\"advisorSummary\":\"");
    body += jsonEscape(advisor.summary);
    body += F("\",\"advisorNarrative\":\"");
    body += jsonEscape(advisor.narrative);
    body += F("\",\"advisorFactors\":[");
    for (uint8_t i = 0; i < advisor.factorCount; ++i)
    {
        if (i > 0) body += ',';
        const AdvisorFactor& factor = advisor.factors[i];
        body += F("{\"name\":\"");
        body += jsonEscape(factor.name);
        body += F("\",\"value\":\"");
        body += jsonEscape(factor.value);
        body += F("\",\"contribution\":");
        body += String(factor.contributionPercent);
        body += '}';
    }
    body += F("],\"advisorReasons\":[");

    for (uint8_t i = 0; i < advisor.reasonCount; ++i)
    {
        if (i > 0)
        {
            body += ',';
        }

        body += '\"';
        body += jsonEscape(advisor.reasons[i]);
        body += '\"';
    }

    body += F("],\"waterToday\":");
    const WaterStatistics& water = waterManager_->statistics();
    body += String(water.todayLiters, 2);
    body += F(",\"waterWeek\":");
    body += String(water.weekLiters, 2);
    body += F(",\"waterMonth\":");
    body += String(water.monthLiters, 2);
    body += F(",\"waterYear\":");
    body += String(water.yearLiters, 2);
    body += F(",\"waterSaved\":");
    body += String(water.savedLiters, 2);
    body += F(",\"waterTodayCost\":");
    body += String(water.todayCost, 3);
    body += F(",\"waterMonthCost\":");
    body += String(water.monthCost, 3);
    body += F(",\"waterYearCost\":");
    body += String(water.yearCost, 3);
    body += F(",\"waterFlow1\":");
    body += String(waterManager_->valveFlowRate(0), 2);
    body += F(",\"waterFlow2\":");
    body += String(waterManager_->valveFlowRate(1), 2);
    body += F(",\"waterPrice\":");
    body += String(waterManager_->waterPrice(), 2);

    float currentRunLiters = 0.0f;
    float currentRunCost = 0.0f;

    if (runtimeManager_->isRunning())
    {
        const uint8_t runningValve =
            runtimeManager_->runningValveIndex();

        const uint32_t elapsedSeconds =
            runtimeManager_->durationSeconds() -
            runtimeManager_->remainingSeconds();

        currentRunLiters =
            waterManager_->valveFlowRate(runningValve) *
            (
                static_cast<float>(elapsedSeconds) /
                60.0f
            );

        currentRunCost =
            currentRunLiters *
            waterManager_->waterPrice() /
            1000.0f;
    }

    body += F(",\"waterCurrentRun\":");
    body += String(currentRunLiters, 3);
    body += F(",\"waterCurrentCost\":");
    body += String(currentRunCost, 4);
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

    const bool vacationEnabled =
        server_.hasArg("enabled") &&
        server_.arg("enabled").toInt() != 0;

    const uint8_t seasonPercent =
        server_.hasArg("season")
            ? static_cast<uint8_t>(server_.arg("season").toInt())
            : smartControlManager_->seasonPercent();

    const uint8_t intervalDays =
        server_.hasArg("every")
            ? static_cast<uint8_t>(server_.arg("every").toInt())
            : smartControlManager_->vacationIntervalDays();

    const uint8_t vacationPercent =
        server_.hasArg("percent")
            ? static_cast<uint8_t>(server_.arg("percent").toInt())
            : smartControlManager_->vacationPercent();

    uint32_t startDate = smartControlManager_->vacationStartDate();
    uint32_t endDate = smartControlManager_->vacationEndDate();

    if (server_.hasArg("start") && server_.arg("start").length() > 0)
    {
        startDate =
            static_cast<uint32_t>(server_.arg("start").toInt());
    }

    if (server_.hasArg("end") && server_.arg("end").length() > 0)
    {
        endDate =
            static_cast<uint32_t>(server_.arg("end").toInt());
    }

    if (vacationEnabled)
    {
        if (!SmartControlManager::validDateKey(startDate) ||
            !SmartControlManager::validDateKey(endDate) ||
            endDate < startDate)
        {
            sendJson(
                400,
                "{\"error\":\"Ungueltiger Urlaubszeitraum\"}"
            );
            return;
        }
    }

    smartControlManager_->setSeasonPercent(seasonPercent);
    smartControlManager_->setVacationIntervalDays(intervalDays);
    smartControlManager_->setVacationPercent(vacationPercent);

    if (SmartControlManager::validDateKey(startDate) &&
        SmartControlManager::validDateKey(endDate) &&
        endDate >= startDate)
    {
        smartControlManager_->setVacationDates(
            startDate,
            endDate
        );
    }

    smartControlManager_->setVacationEnabled(
        vacationEnabled
    );

    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleLog()
{
    String body;
    body.reserve(8000);
    body += F("{\"entries\":[");

    LogManager::Entry entry;
    const uint16_t count = Log.count();
    for (uint16_t i = 0; i < count; ++i)
    {
        if (!Log.entryNewestFirst(i, entry))
        {
            continue;
        }

        if (i > 0)
        {
            body += ',';
        }

        char timestamp[24];
        LogManager::formatTimestamp(entry, timestamp, sizeof(timestamp));

        body += F("{\"sequence\":");
        body += String(entry.sequence);
        body += F(",\"time\":\"");
        body += jsonEscape(String(timestamp));
        body += F("\",\"category\":\"");
        body += LogManager::categoryName(entry.category);
        body += F("\",\"level\":\"");
        body += LogManager::levelName(entry.level);
        body += F("\",\"message\":\"");
        body += jsonEscape(String(entry.message));
        body += F("\"}");
    }

    body += F("]}");
    sendJson(200, body);
}

void WebManager::handleLogClear()
{
    Log.clear();
    Log.info(LogManager::Category::System, "Ereignisprotokoll gelöscht");
    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleSetupSettings()
{
    if (!settingsManager_)
    {
        sendJson(503, "{\"error\":\"Setup nicht bereit\"}");
        return;
    }

    String body;
    body.reserve(320);
    body += F("{\"ssid\":\"");
    body += jsonEscape(settingsManager_->wifiSsid());
    body += F("\",\"passwordConfigured\":");
    body += settingsManager_->wifiPassword().length() > 0
        ? F("true")
        : F("false");
    body += F(",\"latitude\":");
    body += String(settingsManager_->latitude(), 5);
    body += F(",\"longitude\":");
    body += String(settingsManager_->longitude(), 5);
    body += F(",\"timezone\":\"");
    body += jsonEscape(settingsManager_->timezone());
    body += F("\"}");

    sendJson(200, body);
}

void WebManager::handleSetupSave()
{
    if (!settingsManager_)
    {
        sendJson(503, "{\"error\":\"Setup nicht bereit\"}");
        return;
    }

    if (!server_.hasArg("ssid") ||
        !server_.hasArg("latitude") ||
        !server_.hasArg("longitude") ||
        !server_.hasArg("timezone"))
    {
        sendJson(400, "{\"error\":\"Setup-Daten fehlen\"}");
        return;
    }

    const String password =
        server_.hasArg("password")
            ? server_.arg("password")
            : String();

    const bool saved = settingsManager_->saveNetworkLocation(
        server_.arg("ssid"),
        password,
        server_.arg("latitude").toFloat(),
        server_.arg("longitude").toFloat(),
        server_.arg("timezone"),
        String()
    );


    if (!saved)
    {
        sendJson(400, "{\"error\":\"Ungueltige WLAN- oder Standortdaten\"}");
        return;
    }

    Log.info(
        LogManager::Category::System,
        "WLAN- und Standort-Setup gespeichert; Neustart"
    );

    restartRequestedAtMs_ = millis();
    sendJson(200, "{\"ok\":true,\"restart\":true}");
}

void WebManager::handleSetupPortalStart()
{
    if (!settingsManager_)
    {
        sendJson(503, "{\"error\":\"Setup nicht bereit\"}");
        return;
    }

    settingsManager_->requestSetupPortal(true);

    Log.info(
        LogManager::Category::System,
        "Setup-Portal manuell angefordert; Neustart"
    );

    restartRequestedAtMs_ = millis();
    sendJson(200, "{\"ok\":true,\"restart\":true}");
}

void WebManager::handleWaterSettings()
{
    if (waterManager_ == nullptr)
    {
        sendJson(
            503,
            "{\"error\":\"WaterManager nicht bereit\"}"
        );
        return;
    }

    if (!server_.hasArg("flow1") ||
        !server_.hasArg("flow2") ||
        !server_.hasArg("price"))
    {
        sendJson(
            400,
            "{\"error\":\"Wasserdaten fehlen\"}"
        );
        return;
    }

    const float flow1 =
        server_.arg("flow1").toFloat();

    const float flow2 =
        server_.arg("flow2").toFloat();

    const float price =
        server_.arg("price").toFloat();

    if (!waterManager_->
            setValveFlowRate(0, flow1) ||
        !waterManager_->
            setValveFlowRate(1, flow2) ||
        !waterManager_->
            setWaterPrice(price))
    {
        sendJson(
            400,
            "{\"error\":\"Ungültige Wasserdaten\"}"
        );
        return;
    }

    Log.info(
        LogManager::Category::System,
        "Wasserdaten gespeichert"
    );

    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleWaterReset()
{
    if (waterManager_ == nullptr)
    {
        sendJson(
            503,
            "{\"error\":\"WaterManager nicht bereit\"}"
        );
        return;
    }

    waterManager_->resetAll();

    Log.info(
        LogManager::Category::System,
        "Wasserzähler gelöscht"
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
