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
#include "profiles/GardenProfiles.h"
#include "season/SeasonManager.h"
#include "backup/BackupManager.h"
#include "history/HistoryManager.h"
#include "garden/GardenManager.h"

namespace
{
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>GardenFlow v0.40.0</title>
<style>
:root{font-family:system-ui,-apple-system,sans-serif;color-scheme:dark;background:#101714;color:#edf5ef}
body{margin:0;max-width:1180px;padding:18px 18px 30px 238px}.top{display:flex;justify-content:space-between;gap:12px;align-items:center;flex-wrap:wrap}
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
.dashboardValve{display:flex;justify-content:space-between;gap:8px;margin-top:7px}.healthGrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:10px;margin-top:12px}.healthItem{border:1px solid #304237;border-radius:11px;padding:11px;background:#111a15aa}.healthLabel{font-size:.78rem;text-transform:uppercase;letter-spacing:.06em;color:#9fb2a5}.healthValue{font-size:1.05rem;font-weight:800;margin-top:4px}.historyList{margin-top:12px;border:1px solid #304237;border-radius:12px;overflow:hidden}.historyRow{display:grid;grid-template-columns:145px 1fr auto;gap:10px;align-items:center;padding:10px 12px;border-top:1px solid #2b3a31}.historyRow:first-child{border-top:0}.historyTitle{font-weight:800}.historyMeta{font-size:.84rem;color:#a8b7ad;margin-top:3px}.historyValue{text-align:right;font-weight:750}.historySkipped{opacity:.72}.historyEmpty{padding:14px;color:#a8b7ad}.historyTools{display:flex;gap:8px;flex-wrap:wrap;align-items:end;margin-top:12px}.historyTools .field{min-width:150px;flex:1}.historyTools button{white-space:nowrap}@media(max-width:620px){.historyRow{grid-template-columns:1fr}.historyValue{text-align:left}}.advisorCard{grid-column:1/-1;border-color:#6b8f72;background:linear-gradient(135deg,#1e3928,#14251b)}.advisorHeadline{font-size:1.4rem;font-weight:850;margin-top:5px}.advisorReasons{margin-top:9px;display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:6px;color:#c1d1c5}.advisorNarrative{margin-top:12px;padding:11px;border-radius:10px;background:#102017;color:#d8e5da}.advisorFactors{margin-top:12px;border:1px solid #355140;border-radius:10px;overflow:hidden}.advisorFactor{display:grid;grid-template-columns:1.2fr 1fr auto;gap:10px;padding:9px 11px;border-top:1px solid #2b4033}.advisorFactor:first-child{border-top:0}.advisorConfidence{margin-top:12px;display:flex;align-items:center;gap:10px}.confidenceBar{height:9px;flex:1;background:#293a30;border-radius:99px;overflow:hidden}.confidenceFill{height:100%;background:#7fda98}.advisorDuration{margin-top:12px;font-size:1.05rem;font-weight:750}.waterGrid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:10px}.waterValue{font-size:1.2rem;font-weight:800}@media(max-width:620px){.waterGrid{grid-template-columns:repeat(2,1fr)}}
@media(max-width:760px){.dashboardGrid{grid-template-columns:1fr}}

.profileGrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:12px;margin-top:12px}.profileCard{border:1px solid #36503f;border-radius:12px;padding:12px;background:#111a15aa}.profileCard h3{margin:0 0 10px}.profileFormula{margin-top:8px;color:#b9c9be;font-size:.84rem}.simGrid{display:grid;grid-template-columns:repeat(2,1fr);gap:12px;margin-top:12px}.simResult{margin-top:14px;padding:14px;border:1px solid #4b7358;border-radius:12px;background:#102017}.simLine{display:grid;grid-template-columns:1fr auto;gap:10px;padding:6px 0;border-top:1px solid #294032}.simLine:first-child{border-top:0}.simFinal{font-size:1.4rem;font-weight:850;color:#9ce4ae}.rangeRow{display:grid;grid-template-columns:1fr 72px;gap:10px;align-items:center}input[type=range]{width:100%}@media(max-width:620px){.simGrid{grid-template-columns:1fr}}.setupNote{margin-top:8px;color:#a8b7ad;font-size:.86rem}
.setupGrid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:12px}
@media(max-width:620px){.setupGrid{grid-template-columns:1fr}}


.modal{display:none;position:fixed;inset:0;background:#000a;align-items:center;justify-content:center;padding:16px;z-index:20}.modal.open{display:flex}.dialog{width:min(520px,100%);max-height:92vh;overflow:auto;background:#18231d;border:1px solid #3b5143;border-radius:16px;padding:18px}.formgrid{display:grid;grid-template-columns:1fr 1fr;gap:12px}.field{display:flex;flex-direction:column;gap:6px}.field.full{grid-column:1/-1}input,select{border:1px solid #46594c;border-radius:9px;background:#101714;color:#edf5ef;padding:10px;font-size:1rem}.weekdays{display:grid;grid-template-columns:repeat(7,1fr);gap:6px}.day{padding:9px 4px;background:#33463a;color:#edf5ef}.day.active{background:#7fda98;color:#102016}.actions{display:flex;justify-content:flex-end;gap:8px;margin-top:16px}.saveState{min-height:1.4em;margin-top:10px;color:#a8b7ad}.saveState.okmsg{color:#7fda98}.saveState.errmsg{color:#ff9e98}.dirtyMark{color:#ffd27a;font-weight:700}.logTools{display:flex;gap:8px;flex-wrap:wrap;margin-top:10px}.logList{margin-top:10px;max-height:420px;overflow:auto;border:1px solid #304237;border-radius:10px}.logRow{display:grid;grid-template-columns:150px 90px 1fr;gap:8px;padding:8px 10px;border-top:1px solid #2b3a31;font-size:.88rem}.logRow:first-child{border-top:0}.logRow.warning{background:#5a451f55}.logRow.error{background:#66312d66}.logCategory{color:#9fb2a5}.logMessage{word-break:break-word}@media(max-width:620px){.logRow{grid-template-columns:1fr}.logCategory{font-size:.78rem}}@media(max-width:540px){.formgrid{grid-template-columns:1fr}.field.full{grid-column:auto}.weekdays{grid-template-columns:repeat(4,1fr)}}

.mainMenu{position:fixed;left:0;top:0;bottom:0;width:205px;padding:18px 12px;background:#0c120f;border-right:1px solid #2b3a31;z-index:40;overflow:auto}
.menuBrand{padding:6px 10px 18px}.menuBrandTitle{font-size:1.25rem;font-weight:850}.menuBrandSub{font-size:.8rem;color:#8fa197;margin-top:3px}
.menuButton{display:flex;width:100%;align-items:center;gap:10px;border:0;background:transparent;color:#c8d5cc;text-align:left;padding:11px 12px;margin:3px 0;border-radius:10px;font-weight:750}
.menuButton:hover{background:#1a2921}.menuButton.active{background:#285c38;color:#fff}.menuIcon{width:22px;text-align:center;font-size:1.05rem}
.menuDivider{height:1px;background:#27352d;margin:12px 8px}.menuStatus{padding:9px 11px;color:#9fb2a5;font-size:.8rem}
.mobileMenuButton{display:none;position:fixed;left:12px;top:12px;z-index:60;padding:9px 12px;background:#285c38;color:#fff;border-radius:10px;box-shadow:0 4px 15px #0007}
.menuOverlay{display:none;position:fixed;inset:0;background:#0009;z-index:35}
.pageSection{display:none}.pageSection.pageActive{display:block}
.pageGrid{display:none}.pageGrid.pageActive{display:grid}
.pageHeader{display:none;margin:2px 0 14px}.pageHeader.pageActive{display:block}.pageTitle{font-size:1.65rem;font-weight:850}.pageSubtitle{color:#9fb2a5;margin-top:3px}
@media(max-width:820px){
 body{max-width:none;padding:64px 14px 26px;margin:0}
 .mainMenu{transform:translateX(-110%);transition:transform .2s ease;width:230px;box-shadow:8px 0 25px #0008}
 .mainMenu.open{transform:translateX(0)}
 .mobileMenuButton{display:block}
 .menuOverlay.open{display:block}
}


.weekPlannerWrap{overflow:auto;margin-top:12px;border:1px solid #304237;border-radius:12px;background:#101714}
.weekPlanner{display:grid;grid-template-columns:64px repeat(7,minmax(105px,1fr));min-width:850px;position:relative}
.weekCorner,.weekDayHead{position:sticky;top:0;z-index:6;background:#18231d;border-bottom:1px solid #3a4d40;min-height:42px;display:flex;align-items:center;justify-content:center;font-weight:800}
.weekCorner{left:0;z-index:8}
.weekTime{position:sticky;left:0;z-index:4;background:#121b16;border-right:1px solid #304237;border-bottom:1px solid #24342b;height:48px;padding:4px 8px;font-size:.75rem;color:#9fb2a5}
.weekCell{position:relative;height:48px;border-right:1px solid #24342b;border-bottom:1px solid #24342b;background:linear-gradient(to bottom,#17211b 0,#17211b 49%,#141d18 50%,#141d18 100%)}
.weekEvent{position:absolute;touch-action:none;user-select:none;left:4px;right:4px;z-index:3;border-radius:7px;padding:4px 6px;font-size:.75rem;font-weight:800;overflow:hidden;cursor:pointer;box-shadow:0 2px 7px #0007;border:1px solid #ffffff22}
.weekEvent:hover{filter:brightness(1.18);z-index:5}.weekEvent.dragging{opacity:.72;z-index:10;outline:2px solid #fff8;cursor:grabbing;box-shadow:0 8px 20px #000a}.weekEvent.resizing{opacity:.75;z-index:10;outline:2px solid #ffd27a;box-shadow:0 8px 20px #000a}.weekResize{position:absolute;left:3px;right:3px;bottom:1px;height:7px;border-radius:4px;cursor:ns-resize;background:#ffffff38}.weekDragHint{margin-top:8px;padding:8px 10px;border-radius:9px;background:#132019;color:#b8c9bd;font-size:.82rem}.weekGridControls{display:flex;gap:6px;align-items:center;flex-wrap:wrap}.weekGridControls .gridStep{padding:7px 10px;background:#33463a;color:#edf5ef}.weekGridControls .gridStep.active{background:#7fda98;color:#102016}.weekTargetBadge{position:fixed;z-index:80;pointer-events:none;background:#0b130f;color:#fff;border:1px solid #6c8f77;border-radius:8px;padding:5px 8px;font-size:.8rem;font-weight:800;box-shadow:0 4px 14px #0009}.weekSaveState{min-height:1.4em;margin-top:8px;color:#a8b7ad}.weekSaveState.okmsg{color:#7fda98}.weekSaveState.errmsg{color:#ff9e98}.weekEvent.v1{background:#2d7645;color:#fff}.weekEvent.v2{background:#315f91;color:#fff}
.weekEvent.disabled{opacity:.38;filter:saturate(.4)}.weekEventTitle{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.weekEventMeta{font-size:.68rem;font-weight:600;opacity:.9;white-space:nowrap}
.weekLegend{display:flex;gap:14px;flex-wrap:wrap;margin-top:10px;color:#b8c6bc;font-size:.86rem}.legendDot{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:5px}.legendV1{background:#2d7645}.legendV2{background:#315f91}
.weekNowLine{position:absolute;left:64px;right:0;height:2px;background:#ff786e;z-index:2;pointer-events:none}
.weekNowLabel{position:absolute;left:3px;transform:translateY(-50%);font-size:.68rem;color:#ff9d96;background:#101714;padding:1px 3px}
@media(max-width:820px){.weekPlannerWrap{margin-left:0}.weekPlanner{min-width:790px}}


.gardenToolbar{display:flex;gap:8px;flex-wrap:wrap;align-items:center;margin-top:12px}
.gardenCanvasWrap{margin-top:12px;border:1px solid #304237;border-radius:14px;overflow:auto;background:#0e1712}
.gardenBackgroundToolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin:10px 0}.gardenMetricToolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin:8px 0}.gardenMetricToolbar input[type=number]{width:120px;font-size:1rem}.gardenBackgroundToolbar input[type=range]{width:135px}.gardenBackgroundLayer{position:absolute;inset:0;overflow:hidden;pointer-events:none;z-index:0}.gardenBackgroundImage{position:absolute;max-width:none;max-height:none;transform-origin:0 0;user-select:none;-webkit-user-drag:none}.gardenBackgroundMoveMode{outline:2px dashed #7fc995;outline-offset:-4px}.gardenZone{z-index:2}.gardenCanvas{position:relative;width:100%;min-width:720px;aspect-ratio:16/9;background:linear-gradient(90deg,#ffffff08 1px,transparent 1px),linear-gradient(#ffffff08 1px,transparent 1px),radial-gradient(circle at 35% 30%,#264d32 0,#18351f 38%,#102619 72%);background-size:32px 32px,32px 32px,100% 100%;overflow:hidden}
.gardenZone{position:absolute;border:2px solid #ffffff55;border-radius:14px;box-shadow:0 6px 18px #0007;cursor:move;user-select:none;touch-action:none;min-width:70px;min-height:48px;overflow:hidden}
.gardenZone.selected{outline:3px solid #fff9;z-index:5}.gardenZone.watering{outline:3px solid #74d9ff;box-shadow:0 0 0 3px #74d9ff33,0 0 22px #45bde388;animation:gardenWaterPulse 1.4s ease-in-out infinite}.gardenZone.watering .gardenZoneHeader{background:#063d4ccc}.gardenZone.watering .gardenZoneMeta{background:#0a5366cc}.gardenWaterBadge{position:absolute;right:7px;top:7px;padding:3px 6px;border-radius:999px;background:#77ddff;color:#07313b;font-size:.68rem;font-weight:900;box-shadow:0 2px 8px #0007}@keyframes gardenWaterPulse{0%,100%{filter:brightness(1)}50%{filter:brightness(1.25)}}
.gardenZoneHeader{padding:7px 9px;font-weight:850;background:#0005;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.gardenZoneMeta{padding:5px 9px;font-size:.76rem;background:#0004}
.gardenResizeHandle{position:absolute;right:2px;bottom:2px;width:16px;height:16px;border-radius:4px;background:#fff7;cursor:nwse-resize}
.gardenInspector{margin-top:12px;display:none}.gardenInspector.open{display:block}
.gardenInspectorGrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:10px}.gardenStats{display:grid;grid-template-columns:repeat(auto-fit,minmax(145px,1fr));gap:9px;margin-top:12px}.gardenStat{padding:10px 12px;border:1px solid #304237;border-radius:10px;background:#111b16}.gardenStatLabel{font-size:.76rem;color:#9fb2a5}.gardenStatValue{font-size:1.05rem;font-weight:850;margin-top:3px}.gardenZoneHistory{margin-top:10px;border-top:1px solid #304237;padding-top:10px}.gardenPolygonToolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin-top:10px}.gardenVertexHandle{position:absolute;width:16px;height:16px;margin-left:-8px;margin-top:-8px;border:2px solid #fff;border-radius:50%;background:#7fda98;box-shadow:0 2px 8px #0008;cursor:grab;z-index:20;touch-action:none}.gardenVertexHandle.dragging{cursor:grabbing;transform:scale(1.18)}.gardenVertexHandle.activeVertex{background:#ffd36b;border-color:#fff;transform:scale(1.18)}.gardenPolygonToolbar select{padding:7px 9px;border-radius:8px;background:#152219;color:#e8f2eb;border:1px solid #3d5545}.gardenEdgePlus{position:absolute;width:22px;height:22px;margin-left:-11px;margin-top:-11px;border:3px solid #fff;border-radius:50%;background:#16844a;color:#fff;display:flex;align-items:center;justify-content:center;font-size:18px;line-height:18px;font-weight:900;cursor:pointer;z-index:25;box-shadow:0 2px 8px #000a;touch-action:none;pointer-events:auto;padding:0}.gardenPolygonEditing .gardenZone.selected{outline:3px dashed #d9f5df}
.gardenEmpty{position:absolute;inset:0;display:flex;align-items:center;justify-content:center;color:#9fb2a5;font-size:1.05rem;pointer-events:none}
.gardenStatus{min-height:1.4em;margin-top:8px;padding:7px 10px;border-radius:8px;color:#9fb2a5}.gardenStatus.okmsg{color:#7fda98;background:#173321}.gardenStatus.errmsg{color:#ff9e98;background:#3a1d1d}
@media(max-width:820px){.gardenCanvas{min-width:680px}.gardenToolbar button{flex:1}}

</style>
</head>
<body>
<button id="mobileMenuButton" class="mobileMenuButton" onclick="toggleMainMenu()">☰ Menü</button>
<div id="menuOverlay" class="menuOverlay" onclick="closeMainMenu()"></div>
<nav id="mainMenu" class="mainMenu">
  <div class="menuBrand">
    <div class="menuBrandTitle">GardenFlow</div>
    <div class="menuBrandSub">v0.40.0 · HK 2026</div>
  </div>
  <button class="menuButton active" data-page-target="dashboard" onclick="showPage('dashboard')"><span class="menuIcon">⌂</span>Dashboard</button>
  <button class="menuButton" data-page-target="programs" onclick="showPage('programs')"><span class="menuIcon">▤</span>Programme</button>
  <button class="menuButton" data-page-target="weekplan" onclick="showPage('weekplan')"><span class="menuIcon">▦</span>Wochenplan</button>
  <button class="menuButton" data-page-target="garden" onclick="showPage('garden')"><span class="menuIcon">⌘</span>Garten</button>
  <button class="menuButton" data-page-target="smart" onclick="showPage('smart')"><span class="menuIcon">✦</span>Smart Control</button>
  <button class="menuButton" data-page-target="water" onclick="showPage('water')"><span class="menuIcon">◉</span>Wasser</button>
  <button class="menuButton" data-page-target="history" onclick="showPage('history')"><span class="menuIcon">↺</span>Historie</button>
  <button class="menuButton" data-page-target="system" onclick="showPage('system')"><span class="menuIcon">⚙</span>System</button>
  <div class="menuDivider"></div>
  <div id="menuConnectionState" class="menuStatus">System wird geladen …</div>
</nav>
<div class="pageHeader pageActive" data-page-header="dashboard"><div class="pageTitle">Dashboard</div><div class="pageSubtitle">Aktueller Zustand und nächste Bewässerung</div></div>
<div class="pageHeader" data-page-header="programs"><div class="pageTitle">Programme</div><div class="pageSubtitle">Bewässerungsplan und manuelle Steuerung</div></div>
<div class="pageHeader" data-page-header="weekplan"><div class="pageTitle">Wochenplan</div><div class="pageSubtitle">Grafische Übersicht Montag bis Sonntag</div></div>
<div class="pageHeader" data-page-header="garden"><div class="pageTitle">Garten</div><div class="pageSubtitle">Interaktive Gartenkarte und Bewässerungszonen</div></div>
<div class="pageHeader" data-page-header="smart"><div class="pageTitle">Smart Control</div><div class="pageSubtitle">Wetter, Saison, Urlaub, Advisor und Pflanzenprofile</div></div>
<div class="pageHeader" data-page-header="water"><div class="pageTitle">Wasser</div><div class="pageSubtitle">Verbrauch, Durchfluss, Kosten und Einsparung</div></div>
<div class="pageHeader" data-page-header="history"><div class="pageTitle">Historie</div><div class="pageSubtitle">Gespeicherte Bewässerungsereignisse</div></div>
<div class="pageHeader" data-page-header="system"><div class="pageTitle">System</div><div class="pageSubtitle">Backup, WLAN, Diagnose und technische Einstellungen</div></div>
<div class="top pageSection pageActive" data-page="dashboard"><div><h1>GardenFlow by HK 2026 V1.0</h1><div class="muted" id="address">wird verbunden …</div></div><div id="clock" class="big">--:--</div></div>
<section class="card dashboardHero pageSection pageActive" data-page="dashboard">
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
<section class="card pageSection pageActive" data-page="dashboard" style="margin-top:12px">
  <div class="top">
    <div><div class="muted">Systemstatus</div><div id="healthHeadline" class="big">System wird geprüft …</div></div>
    <span id="healthBadge" class="badge">--</span>
  </div>
  <div class="healthGrid">
    <div class="healthItem"><div class="healthLabel">Firmware</div><div id="healthFirmware" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">Laufzeit</div><div id="healthUptime" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">Freier Heap</div><div id="healthHeap" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">PSRAM frei</div><div id="healthPsram" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">WLAN</div><div id="healthWifi" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">NTP</div><div id="healthNtp" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">Wetter</div><div id="healthWeather" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">OTA</div><div id="healthOta" class="healthValue">--</div></div>
    <div class="healthItem"><div class="healthLabel">Letztes Backup</div><div id="healthBackup" class="healthValue">--</div></div>
  </div>
</section>
<section class="card pageSection" data-page="history" style="margin-top:12px">
  <div class="top"><div><div class="muted">Bewässerungshistorie</div><div class="big">Letzte Ereignisse</div></div><span id="historyCount" class="badge">0</span></div>
  <div class="setupNote">Die letzten Bewässerungsereignisse aus dem persistenten HistoryManager.</div>
  <div class="historyTools">
    <label class="field"><span>Zeitraum</span><select id="historyPeriod" onchange="renderHistory()"><option value="all">Alle geladenen</option><option value="today">Heute</option><option value="week">Letzte 7 Tage</option><option value="month">Letzte 30 Tage</option></select></label>
    <label class="field"><span>Ereignis</span><select id="historyEventFilter" onchange="renderHistory()"><option value="">Alle</option><option value="start">Gestartet</option><option value="stop">Beendet / Abgebrochen</option><option value="skipped">Übersprungen</option></select></label>
    <label class="field"><span>Ventil</span><select id="historyValveFilter" onchange="renderHistory()"><option value="">Alle</option><option value="0">Ventil 1</option><option value="1">Ventil 2</option></select></label>
    <button class="secondary" onclick="downloadHistoryCsv()">CSV exportieren</button>
    <button class="secondary" onclick="downloadHistoryJson()">JSON exportieren</button>
  </div>
  <div id="historyList" class="historyList"><div class="historyEmpty">Historie wird geladen …</div></div>
</section>
<div class="grid pageGrid pageActive" data-page="dashboard">
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
<section class="card pageSection" data-page="smart" style="margin-top:12px"><div class="top"><div><div class="muted">Wettersteuerung <span id="weatherDirtyMark" class="dirtyMark"></span></div><div class="big">Automatische Regenpause</div></div></div><div class="formgrid" style="margin-top:12px"><label class="field"><span>Automatik</span><select id="weatherEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label><label class="field"><span>Regenmenge 24 h (mm)</span><input id="weatherRainMm" type="number" min="0.1" max="100" step="0.1"></label><label class="field"><span>Regenwahrscheinlichkeit (%)</span><input id="weatherPop" type="number" min="1" max="100"></label><div class="field"><span>&nbsp;</span><div><button class="secondary" onclick="cancelWeatherSettings()">Abbrechen</button> <button onclick="saveWeatherSettings()">Speichern</button></div></div></div><div id="weatherSaveState" class="saveState"></div></section>
<section class="card pageSection" data-page="smart" style="margin-top:12px">
<div class="top"><div><div class="muted">Smart Control <span id="smartDirtyMark" class="dirtyMark"></span></div><div class="big">Saison & Urlaub</div></div><span id="vacationState" class="badge">--</span></div>
<div class="formgrid" style="margin-top:12px">
<label class="field"><span>Saisonsteuerung</span><select id="seasonAutomatic"><option value="1">Automatisch aus Standort & Datum</option><option value="0">Manueller Faktor</option></select></label>
<label class="field"><span>Manueller Saisonfaktor (%)</span><input id="seasonPercent" type="number" min="10" max="200" step="5"></label>
<div class="field full"><div id="seasonAutoInfo" class="advisorNarrative">Saisonberechnung wird geladen …</div></div>
<label class="field"><span>Urlaubsmodus</span><select id="vacationEnabled"><option value="1">Ein</option><option value="0">Aus</option></select></label>
<label class="field"><span>Start</span><input id="vacationStart" type="date"></label>
<label class="field"><span>Ende</span><input id="vacationEnd" type="date"></label>
<label class="field"><span>Bewässern alle</span><select id="vacationEvery"><option value="1">jeden Tag</option><option value="2">2 Tage</option><option value="3">3 Tage</option><option value="4">4 Tage</option><option value="5">5 Tage</option><option value="6">6 Tage</option><option value="7">7 Tage</option></select></label>
<label class="field"><span>Laufzeit im Urlaub (%)</span><input id="vacationPercent" type="number" min="10" max="100" step="5"></label>
<div class="field full"><button class="secondary" onclick="cancelSmartSettings()">Abbrechen</button> <button onclick="saveSmartSettings()">Smart-Einstellungen speichern</button></div>
</div><div id="smartSaveState" class="saveState"></div></section>
<section class="card nextProgram pageSection" data-page="programs"><div class="top"><div><div class="muted">Nächstes Programm</div><div id="nextProgramTime" class="nextProgramTime">--:--</div><div id="nextProgramMeta" class="nextProgramMeta">Kein aktives Programm geplant</div></div><span id="nextProgramWhen" class="badge">--</span></div></section>
<section class="card pageSection" data-page="programs" style="margin-top:12px"><div class="top"><div><div class="muted">Zeitplan</div><div class="big">Heute, morgen und diese Woche</div></div><button class="secondary" onclick="loadAll()">Aktualisieren</button></div><div id="upcomingPrograms"></div></section>
<section class="card pageSection" data-page="programs" style="margin-top:12px"><div class="top"><div><div class="muted">Programme</div><div class="big">Alle Programme</div></div><button onclick="newProgram()">+ Neu</button></div><div id="programs"></div></section>
<section class="card pageSection" data-page="weekplan" style="margin-top:12px">
  <div class="top">
    <div><div class="muted">Kalenderansicht</div><div class="big">Bewässerungswoche</div></div>
    <div class="weekGridControls">
      <span class="muted">Raster:</span>
      <button class="gridStep" data-grid-step="5" onclick="setWeekGridStep(5)">5 min</button>
      <button class="gridStep" data-grid-step="15" onclick="setWeekGridStep(15)">15 min</button>
      <button class="gridStep" data-grid-step="30" onclick="setWeekGridStep(30)">30 min</button>
      <button class="secondary" onclick="renderWeeklyCalendar()">Aktualisieren</button>
    </div>
  </div>
  <div class="setupNote">Ein Balken zeigt Startzeit und Dauer. Klick öffnet den Programmeditor. Ziehen verschiebt das gesamte Programm; der Griff unten ändert die Laufzeit.</div>
  <div class="weekLegend"><span><span class="legendDot legendV1"></span>Ventil 1</span><span><span class="legendDot legendV2"></span>Ventil 2</span><span>Abgeblendet = Programm deaktiviert</span></div>
  <div class="weekDragHint">Das Raster kann oben auf 5, 15 oder 30 Minuten gestellt werden. Beim Ziehen wird die Zielzeit neben dem Mauszeiger angezeigt. ESC bricht die Aktion ab.</div>
  <div id="weekPlannerWrap" class="weekPlannerWrap"><div id="weekPlanner" class="weekPlanner"></div></div>
  <div id="weekSaveState" class="weekSaveState"></div>
</section>
<section class="card pageSection" data-page="garden" style="margin-top:12px">
  <div class="top">
    <div><div class="muted">Interaktive Gartenkarte</div><div class="big">Bewässerungszonen</div></div>
    <span id="gardenZoneCount" class="badge">0</span>
  </div>
  <div class="setupNote">Flächen können verschoben und vergrößert werden. Jede Zone bekommt Name, Pflanzenprofil und Ventil. In dieser ersten Version wird die Karte im Browser gespeichert.</div>
  <div class="gardenToolbar">
    <button onclick="addGardenZone()">+ Zone hinzufügen</button>
    <button class="secondary" onclick="saveGardenMap()">Karte speichern</button>
    <button class="secondary" onclick="resetGardenMap()">Karte zurücksetzen</button>
  </div>
  <div id="gardenStatus" class="gardenStatus"></div>

  <div class="card" style="margin-top:10px;padding:12px">
    <div class="top" style="margin-bottom:8px">
      <div>
        <div class="muted">Flächenberechnung</div>
        <div class="big">Reale Größe der Planfläche</div>
      </div>
      <span id="gardenMetricInfo" class="badge">Noch kein Maßstab gesetzt</span>
    </div>
    <div class="gardenMetricToolbar">
      <label class="field"><span>Gesamte Kartenbreite</span><input id="gardenMapWidthM" type="number" min="1" max="1000" step="0.1" placeholder="z. B. 20.0"> m</label>
      <label class="field"><span>Gesamte Kartenhöhe</span><input id="gardenMapHeightM" type="number" min="1" max="1000" step="0.1" placeholder="z. B. 15.0"> m</label>
      <button onclick="gardenSetMapDimensions()">Maßstab speichern</button>
    </div>
    <div class="setupNote" style="margin-top:8px">Beispiel: Planfläche 20 m × 10 m = 200 m². Die Zeichenfläche übernimmt automatisch dasselbe Seitenverhältnis 2:1. Mit „Bild an Plan anpassen“ liegt das Hintergrundbild exakt auf der Planfläche.</div>
  </div>

  <div id="gardenInspector" class="card gardenInspector">
    <div class="top">
      <div><div class="muted">Ausgewählte Zone</div><div id="gardenInspectorTitle" class="big">--</div></div>
      <div>
        <button onclick="saveSelectedGardenZone()">Zone speichern</button>
        <button class="stop" onclick="deleteSelectedGardenZone()">Zone löschen</button>
      </div>
    </div>
    <div class="gardenInspectorGrid">
      <label class="field"><span>Name</span><input id="gardenZoneName" maxlength="24" oninput="updateSelectedGardenZoneFromInspector(false)"></label>
      <label class="field"><span>Profil</span><select id="gardenZoneProfile" onchange="updateSelectedGardenZoneFromInspector(true)"></select></label>
      <label class="field"><span>Ventil</span><select id="gardenZoneValve" onchange="updateSelectedGardenZoneFromInspector(true)"><option value="0">Ventil 1</option><option value="1">Ventil 2</option></select></label>
      <label class="field"><span>Programm</span><select id="gardenZoneProgram" onchange="updateSelectedGardenZoneFromInspector(true)"></select></label>
      <label class="field"><span>Programm öffnen</span><button class="secondary" onclick="openSelectedGardenProgram()">Programmeditor öffnen</button></label>
      <label class="field"><span>Farbe</span><input id="gardenZoneColor" type="color" value="#2d7645" oninput="updateSelectedGardenZoneFromInspector(false)" onchange="updateSelectedGardenZoneFromInspector(true)"></label>
    </div>
    <div class="gardenPolygonToolbar">
      <button id="gardenPolygonEditButton" class="secondary" onclick="toggleGardenPolygonEdit()">Polygon bearbeiten</button>
      <button id="gardenDeleteVertexButton" class="secondary" onclick="gardenDeleteVertex()" disabled>Eckpunkt löschen</button>
      <select id="gardenRoundStrength" title="Rundungsgröße">
        <option value="0.15">Rundung klein</option>
        <option value="0.25" selected>Rundung mittel</option>
        <option value="0.35">Rundung groß</option>
      </select>
      <button id="gardenRoundVertexButton" class="secondary" onclick="gardenRoundSelectedVertex()" disabled>Ecke abrunden</button>
      <span id="gardenPolygonInfo" class="muted">Eckpunkte können im Bearbeitungsmodus verschoben werden.</span>
    </div>
    <div class="gardenStats">
      <div class="gardenStat"><div class="gardenStatLabel">Letzte Bewässerung</div><div id="gardenLastWatering" class="gardenStatValue">--</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Abgeschlossene Läufe</div><div id="gardenRunCount" class="gardenStatValue">0</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Aktueller Lauf</div><div id="gardenCurrentRun" class="gardenStatValue">--</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Fläche</div><div id="gardenArea" class="gardenStatValue">-- m²</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Wasser inkl. laufendem Programm</div><div id="gardenLiters" class="gardenStatValue">0,0 l</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Wasser / Fläche</div><div id="gardenLitersPerM2" class="gardenStatValue">-- l/m²</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Bewässerungsziel</div><div class="gardenStatValue"><input id="gardenTargetMm" type="number" min="0" max="50" step="0.5" value="8.0" style="width:82px" onchange="gardenSaveTargetMm()"> mm</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Benötigte Menge</div><div id="gardenRequiredLiters" class="gardenStatValue">-- l</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Durchfluss</div><div id="gardenZoneFlowRate" class="gardenStatValue">-- l/min</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Empfohlene Laufzeit</div><div id="gardenRecommendedRuntime" class="gardenStatValue">-- min</div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Programm übernehmen</div><div class="gardenStatValue"><button id="gardenApplyRuntimeButton" class="secondary" onclick="gardenApplyRecommendedRuntime()" disabled>Laufzeit übernehmen</button></div></div>
      <div class="gardenStat"><div class="gardenStatLabel">Kosten inkl. laufendem Programm</div><div id="gardenCost" class="gardenStatValue">0,00 €</div></div>
    </div>
    <div id="gardenZoneHistory" class="gardenZoneHistory setupNote">Für Historie bitte ein Programm zuordnen.</div>
  </div>

  <div class="gardenCanvasWrap">
    <div class="gardenBackgroundToolbar">
    <input id="gardenBackgroundFile" type="file" accept="image/*" onchange="gardenLoadBackgroundFile(event)">
    <label>Transparenz <input id="gardenBackgroundOpacity" type="range" min="10" max="100" value="45" oninput="gardenSetBackgroundOpacity(this.value)"></label>
    <label>Größe <input id="gardenBackgroundScale" type="range" min="25" max="300" value="100" oninput="gardenSetBackgroundScale(this.value)"></label>
    <button id="gardenBackgroundMoveButton" class="secondary" onclick="gardenToggleBackgroundMove()">Hintergrund verschieben</button><button class="secondary" onclick="gardenFitBackgroundToPlan()">Bild an Plan anpassen</button>
    <button class="secondary" onclick="gardenResetBackgroundTransform()">Bild zentrieren</button>
    <button class="stop" onclick="gardenRemoveBackground()">Hintergrund entfernen</button>
    <span id="gardenBackgroundInfo" class="muted">Kein Hintergrundbild</span>
  </div>
  <div id="gardenCanvas" class="gardenCanvas">
      <div id="gardenEmpty" class="gardenEmpty">Noch keine Gartenbereiche angelegt</div>
    </div>
  </div>
</section>
<section class="card pageSection" data-page="smart" style="margin-top:12px">
<div class="top">
  <div>
    <div class="muted">Advisor-Simulator</div>
    <div class="big">Was wäre wenn?</div><div style="margin-top:8px"><button class="secondary" onclick="resetAdvisorSimulator()">Live-Werte übernehmen</button></div>
  </div>
  <span class="badge off">NUR SIMULATION</span>
</div>
<div class="setupNote">Die Regler verändern weder Wetterdaten noch Programme. Sie zeigen ausschließlich, wie GardenFlow unter angenommenen Bedingungen empfehlen würde.</div>
<div class="simGrid">
  <label class="field"><span>Programm</span><select id="simProgram" onchange="selectSimulationProgram()"></select></label>
  <label class="field"><span>Profil</span><select id="simProfile" onchange="runAdvisorSimulation()"></select></label>
  <label class="field"><span>Temperatur</span><div class="rangeRow"><input id="simTemperature" type="range" min="-5" max="45" step="0.5" value="25" oninput="runAdvisorSimulation()"><strong id="simTemperatureValue">25 °C</strong></div></label>
  <label class="field"><span>Luftfeuchte</span><div class="rangeRow"><input id="simHumidity" type="range" min="10" max="100" step="1" value="50" oninput="runAdvisorSimulation()"><strong id="simHumidityValue">50 %</strong></div></label>
  <label class="field"><span>Regenmenge 24 h</span><div class="rangeRow"><input id="simRain" type="range" min="0" max="30" step="0.5" value="0" oninput="runAdvisorSimulation()"><strong id="simRainValue">0 mm</strong></div></label>
  <label class="field"><span>Regenwahrscheinlichkeit</span><div class="rangeRow"><input id="simProbability" type="range" min="0" max="100" step="1" value="0" oninput="runAdvisorSimulation()"><strong id="simProbabilityValue">0 %</strong></div></label>
  <label class="field"><span>Saisonfaktor</span><div class="rangeRow"><input id="simSeason" type="range" min="30" max="130" step="1" value="100" oninput="runAdvisorSimulation()"><strong id="simSeasonValue">100 %</strong></div></label>
  <label class="field"><span>Standardlaufzeit</span><input id="simDuration" type="number" min="1" max="240" value="10" oninput="runAdvisorSimulation()"></label>
</div>
<div id="simResult" class="simResult"><div class="muted">Simulation wird vorbereitet …</div></div>
</section>
<section class="card pageSection" data-page="smart" style="margin-top:12px">
<div class="top">
  <div>
    <div class="muted">Garden Profiles</div>
    <div class="big">Pflanzenprofile bearbeiten</div>
  </div>
  <button class="secondary" onclick="resetProfiles()">Standardwerte</button>
</div>
<div class="setupNote">Profilkorrektur wirkt nach dem Saisonfaktor. Die Empfindlichkeiten bestimmen, wie stark Temperatur, Luftfeuchte und Regen die Empfehlung beeinflussen.</div>
<div id="profileGrid" class="profileGrid"><div class="muted">Profile werden geladen …</div></div>
<div id="profileSaveState" class="saveState"></div>
</section>
<section class="card pageSection" data-page="water" style="margin-top:12px">
<div class="top"><div><div class="muted">Wasserbilanz</div><div class="big">Durchfluss & Kosten</div></div></div>
<div class="formgrid" style="margin-top:12px">
<label class="field"><span>Ventil 1 (Liter/Minute)</span><input id="waterFlow1" type="number" min="0" max="250" step="0.1"></label>
<label class="field"><span>Ventil 2 (Liter/Minute)</span><input id="waterFlow2" type="number" min="0" max="250" step="0.1"></label>
<label class="field"><span>Wasserpreis (Euro/m³)</span><input id="waterPrice" type="number" min="0" max="100" step="0.01"></label>
<div class="field"><span>&nbsp;</span><div><button onclick="saveWaterSettings()">Speichern</button> <button class="stop" onclick="resetWaterStatistics()">Zähler löschen</button></div></div>
</div>
<div id="waterSaveState" class="saveState"></div>
</section>
<section class="card pageSection" data-page="system" style="margin-top:12px">
<div class="top"><div><div class="muted">Datensicherung</div><div class="big">GardenFlow Backup</div></div><span class="badge ok">JSON</span></div>
<div class="setupNote">Sichert Programme, Gartenprofile, Wasserparameter, Einstellungen und Statistik. Passwörter und API-Schlüssel sind in dieser Version nicht enthalten.</div>
<div style="margin-top:12px"><button onclick="downloadGardenFlowBackup()">Backup herunterladen</button></div>
<div class="setupNote" style="margin-top:14px"><b>Wiederherstellen:</b> Programme, Profile, Standort, Wetter- und Wasserparameter werden aus einer Backup-Datei übernommen. WLAN-Passwort, API-Key und Verbrauchsstatistik bleiben unverändert.</div>
<div class="formgrid" style="margin-top:10px">
<label class="field full"><span>Backup-Datei</span><input id="backupRestoreFile" type="file" accept=".json,application/json"></label>
</div>
<div style="margin-top:10px"><button class="stop" onclick="restoreGardenFlowBackup()">Backup wiederherstellen</button></div>
<div id="backupState" class="saveState"></div>
</section>
<section class="card pageSection" data-page="system" style="margin-top:12px"><div class="top"><div><div class="muted">Setup</div><div class="big">WLAN und Standort</div></div><span id="setupState" class="badge">--</span></div><div class="setupGrid"><label class="field"><span>WLAN-Name (SSID)</span><input id="setupSsid" maxlength="32" autocomplete="off"></label><label class="field"><span>WLAN-Passwort</span><input id="setupPassword" type="password" placeholder="leer = unverändert" autocomplete="new-password"></label><label class="field"><span>Breitengrad</span><input id="setupLatitude" type="number" min="-90" max="90" step="0.00001"></label><label class="field"><span>Längengrad</span><input id="setupLongitude" type="number" min="-180" max="180" step="0.00001"></label><label class="field full"><span>Zeitzone (POSIX)</span><input id="setupTimezone" value="CET-1CEST,M3.5.0/2,M10.5.0/3"></label></div><div class="setupNote">Deutschland: Der voreingestellte Zeitzonenwert berücksichtigt Sommer- und Winterzeit automatisch. Nach dem Speichern startet GardenFlow neu.</div><div style="margin-top:12px"><button onclick="saveSetup()">WLAN und Standort speichern</button> <button class="secondary" onclick="startSetupPortal()">Setup-Portal starten</button></div><div id="setupSaveState" class="saveState"></div></section>
<section class="card pageSection" data-page="system" style="margin-top:12px"><div class="top"><div><div class="muted">Diagnose</div><div class="big">Ereignisprotokoll</div></div><span id="logCount" class="badge">0</span></div><div class="logTools"><select id="logFilter" onchange="renderLog()"><option value="">Alle Kategorien</option><option>System</option><option>WLAN</option><option>Zeit</option><option>Wetter</option><option>Programm</option><option>Ventil</option><option>Scheduler</option><option>Fehler</option></select><input id="logSearch" placeholder="Suchen" oninput="renderLog()"><button class="secondary" onclick="loadLog()">Aktualisieren</button><button class="stop" onclick="clearLog()">Löschen</button></div><div id="logList" class="logList"><div class="muted" style="padding:10px">Protokoll wird geladen …</div></div></section>
<div id="editorModal" class="modal" onclick="modalBackdrop(event)"><div class="dialog">
  <div class="top"><div><div class="muted">Programm</div><div id="editorTitle" class="big">Neu</div></div><button class="secondary" onclick="closeEditor()">Schließen</button></div>
  <div class="formgrid" style="margin-top:16px">
    <label class="field"><span>Ventil</span><select id="editValve"><option value="0">Ventil 1</option><option value="1">Ventil 2</option></select></label><label class="field"><span>Gartenprofil</span><select id="editProfile"><option value="0">Allgemein</option><option value="1">Rasen</option><option value="2">Blumen</option><option value="3">Gemüse</option><option value="4">Tomaten</option><option value="5">Hecke</option><option value="6">Bäume</option><option value="7">Mediterran</option></select></label>
    <label class="field"><span>Startzeit</span><input id="editTime" type="time" value="06:00"></label>
    <label class="field"><span>Dauer (Minuten)</span><input id="editDuration" type="number" min="1" max="1440" value="15"></label>
    <label class="field"><span>Status</span><select id="editEnabled"><option value="1">Aktiv</option><option value="0">Inaktiv</option></select></label>
    <div class="field full"><span>Wochentage</span><div id="weekdayButtons" class="weekdays"></div></div>
  </div>
  <div class="actions"><button class="secondary" onclick="closeEditor()">Abbrechen</button><button onclick="saveEditor()">Speichern</button></div>
</div></div>
<script>
const GARDENFLOW_PAGES=['dashboard','programs','weekplan','garden','smart','water','history','system'];

function showPage(page){
    if(!GARDENFLOW_PAGES.includes(page))page='dashboard';

    document.querySelectorAll('[data-page]').forEach(element=>{
        const active=element.dataset.page===page;
        element.classList.toggle('pageActive',active);
    });

    document.querySelectorAll('[data-page-header]').forEach(element=>{
        element.classList.toggle('pageActive',element.dataset.pageHeader===page);
    });

    document.querySelectorAll('[data-page-target]').forEach(button=>{
        button.classList.toggle('active',button.dataset.pageTarget===page);
    });

    try{localStorage.setItem('gardenflowPage',page);}catch(e){}
    closeMainMenu();
    window.scrollTo({top:0,behavior:'smooth'});
}

function toggleMainMenu(){
    document.getElementById('mainMenu').classList.toggle('open');
    document.getElementById('menuOverlay').classList.toggle('open');
}

function closeMainMenu(){
    document.getElementById('mainMenu').classList.remove('open');
    document.getElementById('menuOverlay').classList.remove('open');
}

function restoreSelectedPage(){
    let page='dashboard';
    try{page=localStorage.getItem('gardenflowPage')||'dashboard';}catch(e){}
    showPage(page);
}

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
const smartFields=['seasonAutomatic','seasonPercent','vacationEnabled','vacationStart','vacationEnd','vacationEvery','vacationPercent'];
function setSaveState(id,text,kind=''){const e=document.getElementById(id);e.textContent=text;e.className='saveState '+kind}
function markWeatherDirty(){weatherDirty=true;document.getElementById('weatherDirtyMark').textContent='• ungespeichert';setSaveState('weatherSaveState','Änderungen noch nicht gespeichert')}
function markSmartDirty(){smartDirty=true;document.getElementById('smartDirtyMark').textContent='• ungespeichert';setSaveState('smartSaveState','Änderungen noch nicht gespeichert')}
function fillWeatherForm(s){document.getElementById('weatherEnabled').value=s.weatherPauseEnabled?'1':'0';document.getElementById('weatherRainMm').value=s.weatherRainLimit;document.getElementById('weatherPop').value=s.weatherProbabilityLimit}
function fillSmartForm(s){document.getElementById('seasonAutomatic').value=s.seasonAutomatic?'1':'0';document.getElementById('seasonPercent').value=s.manualSeasonPercent;document.getElementById('seasonPercent').disabled=s.seasonAutomatic;document.getElementById('seasonAutoInfo').textContent=s.seasonAutomatic?(s.seasonName+' · '+s.seasonPercent+' % · '+s.seasonExplanation):('Manueller Saisonfaktor aktiv: '+s.manualSeasonPercent+' %');document.getElementById('vacationEnabled').value=s.vacationEnabled?'1':'0';document.getElementById('vacationStart').value=dateKeyToInput(s.vacationStart);document.getElementById('vacationEnd').value=dateKeyToInput(s.vacationEnd);document.getElementById('vacationEvery').value=String(s.vacationEvery);document.getElementById('vacationPercent').value=s.vacationPercent}
function cancelWeatherSettings(){if(lastStatus)fillWeatherForm(lastStatus);weatherDirty=false;document.getElementById('weatherDirtyMark').textContent='';setSaveState('weatherSaveState','Änderungen verworfen')}
function cancelSmartSettings(){if(lastStatus)fillSmartForm(lastStatus);smartDirty=false;document.getElementById('smartDirtyMark').textContent='';setSaveState('smartSaveState','Änderungen verworfen')}
function bindSettingsForms(){weatherFields.forEach(id=>{const e=document.getElementById(id);e.addEventListener('input',markWeatherDirty);e.addEventListener('change',markWeatherDirty)});smartFields.forEach(id=>{const e=document.getElementById(id);e.addEventListener('input',markSmartDirty);e.addEventListener('change',markSmartDirty)})}
window.addEventListener('beforeunload',e=>{if(weatherDirty||smartDirty){e.preventDefault();e.returnValue=''}});
function signedPercent(value){const number=Number(value||0);return (number>0?'+':'')+number+' %';}
function confidenceText(value){if(value>=85)return 'Sehr sicher';if(value>=70)return 'Sicher';if(value>=50)return 'Mittlere Sicherheit';return 'Geringe Sicherheit';}
function simulationTemperatureContribution(value){
    if(value>=35)return 30;
    if(value>=30)return 20;
    if(value>=26)return 10;
    if(value<15)return -25;
    if(value<18)return -10;
    return 0;
}

function simulationHumidityContribution(value){
    if(value<35)return 10;
    if(value>80)return -10;
    return 0;
}

function simulationRainContribution(mm,probability){
    if(mm>=8||probability>=80)return -100;
    if(mm>=2)return -25;
    if(probability>=60)return -30;
    if(probability>=40)return -15;
    return 0;
}

let advisorSimulatorInitialized=false;

function fillSimulatorSelectors(){
    const programSelect=document.getElementById('simProgram');
    const profileSelect=document.getElementById('simProfile');
    if(!programSelect||!profileSelect)return;

    const previousProgram=programSelect.value;
    const previousProfile=profileSelect.value;

    programSelect.innerHTML=programCache.map(program=>
        `<option value="${program.index}">Programm ${program.id} · ${program.durationMinutes} min</option>`
    ).join('')||'<option value="">Kein Programm</option>';

    profileSelect.innerHTML=profileCache.map(profile=>
        `<option value="${profile.id}">${esc(profile.name)}</option>`
    ).join('');

    if(programCache.some(p=>String(p.index)===previousProgram))programSelect.value=previousProgram;
    if(profileCache.some(p=>String(p.id)===previousProfile))profileSelect.value=previousProfile;

    const selected=programCache.find(
        p=>String(p.index)===programSelect.value
    )||programCache[0];

    /*
     * Nur beim ersten Laden echte Werte übernehmen.
     * Spätere Status-/Programmakualisierungen dürfen die vom
     * Benutzer eingestellten Simulationswerte nicht überschreiben.
     */
    if(!advisorSimulatorInitialized){
        if(selected){
            document.getElementById('simDuration').value=
                selected.durationMinutes;
            profileSelect.value=
                String(selected.profileId||0);
        }

        if(lastStatus){
            document.getElementById('simTemperature').value=
                Number(lastStatus.temperature||25);
            document.getElementById('simHumidity').value=
                Number(lastStatus.humidity||50);
            document.getElementById('simRain').value=
                Number(lastStatus.rainMm||0);
            document.getElementById('simProbability').value=
                Number(lastStatus.rainProbability||0);
            document.getElementById('simSeason').value=
                Number(lastStatus.seasonPercent||100);
        }

        advisorSimulatorInitialized=true;
    }

    runAdvisorSimulation();
}

function resetAdvisorSimulator(){
    advisorSimulatorInitialized=false;
    fillSimulatorSelectors();
}

function selectSimulationProgram(){
    const index=Number(document.getElementById('simProgram').value);
    const program=programCache.find(item=>Number(item.index)===index);
    if(!program)return;
    document.getElementById('simDuration').value=program.durationMinutes;
    document.getElementById('simProfile').value=String(program.profileId||0);
    runAdvisorSimulation();
}

function runAdvisorSimulation(){
    const result=document.getElementById('simResult');
    if(!result)return;

    const temperature=Number(document.getElementById('simTemperature').value);
    const humidity=Number(document.getElementById('simHumidity').value);
    const rainMm=Number(document.getElementById('simRain').value);
    const probability=Number(document.getElementById('simProbability').value);
    const season=Number(document.getElementById('simSeason').value);
    const standard=Number(document.getElementById('simDuration').value||0);
    const profile=profileById(Number(document.getElementById('simProfile').value));

    document.getElementById('simTemperatureValue').textContent=temperature.toFixed(1)+' °C';
    document.getElementById('simHumidityValue').textContent=humidity+' %';
    document.getElementById('simRainValue').textContent=rainMm.toFixed(1)+' mm';
    document.getElementById('simProbabilityValue').textContent=probability+' %';
    document.getElementById('simSeasonValue').textContent=season+' %';

    const rawTemperature=simulationTemperatureContribution(temperature);
    const rawHumidity=simulationHumidityContribution(humidity);
    const rawRain=simulationRainContribution(rainMm,probability);

    const temperatureContribution=Math.round(rawTemperature*Number(profile.temperature||100)/100);
    const humidityContribution=Math.round(rawHumidity*Number(profile.humidity||100)/100);
    const rainContribution=Math.round(rawRain*Number(profile.rain||100)/100);
    const weather=temperatureContribution+humidityContribution+rainContribution;

    const seasonal=Math.max(1,Math.round(standard*season/100));
    const profiled=Math.max(profile.minimum,Math.min(profile.maximum,
        Math.round(seasonal*(100+Number(profile.correction||0))/100)));
    const recommended=weather<=-60?0:Math.max(profile.minimum,Math.min(profile.maximum,
        Math.round(profiled*(100+weather)/100)));

    const recommendation=recommended===0?'Bewässerung aussetzen':
        recommended>profiled?'Laufzeit erhöhen':recommended<profiled?'Laufzeit reduzieren':'Keine Wetteränderung';

    const signed=value=>(value>0?'+':'')+value+' %';

    result.innerHTML=`
      <div class="simLine"><span>Profil</span><strong>${esc(profile.name)} (${signed(Number(profile.correction||0))})</strong></div>
      <div class="simLine"><span>Standardlaufzeit</span><strong>${standard} min</strong></div>
      <div class="simLine"><span>Saison ${season} %</span><strong>${seasonal} min</strong></div>
      <div class="simLine"><span>Profilkorrektur</span><strong>${profiled} min</strong></div>
      <div class="simLine"><span>Temperaturbeitrag</span><strong>${signed(temperatureContribution)}</strong></div>
      <div class="simLine"><span>Luftfeuchtebeitrag</span><strong>${signed(humidityContribution)}</strong></div>
      <div class="simLine"><span>Regenbeitrag</span><strong>${signed(rainContribution)}</strong></div>
      <div class="simLine"><span>Wetterkorrektur gesamt</span><strong>${signed(weather)}</strong></div>
      <div class="simLine"><span>${recommendation}</span><strong class="simFinal">${recommended===0?'PAUSE':recommended+' min'}</strong></div>`;
}

let profileCache=[];

function profileById(id){
    return profileCache.find(
        profile=>Number(profile.id)===Number(id)
    )||{
        id:0,
        name:'Allgemein',
        symbol:'Garten',
        correction:0,
        temperature:100,
        humidity:100,
        rain:100,
        minimum:1,
        maximum:240
    };
}

function renderProfileEditor(){
    const grid=document.getElementById('profileGrid');
    if(!grid)return;

    grid.innerHTML=profileCache.map(profile=>`
      <div class="profileCard">
        <h3>${esc(profile.symbol||'')} ${esc(profile.name)}</h3>
        <div class="formgrid">
          <label class="field"><span>Name</span><input id="profileName${profile.id}" maxlength="23" value="${esc(profile.name)}"></label>
          <label class="field"><span>Symbol/Bezeichnung</span><input id="profileSymbol${profile.id}" maxlength="15" value="${esc(profile.symbol||'')}"></label>
          <label class="field"><span>Profilkorrektur (%)</span><input id="profileCorrection${profile.id}" type="number" min="-80" max="100" step="5" value="${profile.correction}"></label>
          <label class="field"><span>Temperatur-Empfindlichkeit (%)</span><input id="profileTemperature${profile.id}" type="number" min="0" max="200" step="5" value="${profile.temperature}"></label>
          <label class="field"><span>Luftfeuchte-Empfindlichkeit (%)</span><input id="profileHumidity${profile.id}" type="number" min="0" max="200" step="5" value="${profile.humidity}"></label>
          <label class="field"><span>Regen-Empfindlichkeit (%)</span><input id="profileRain${profile.id}" type="number" min="0" max="200" step="5" value="${profile.rain}"></label>
          <label class="field"><span>Min. Laufzeit (min)</span><input id="profileMinimum${profile.id}" type="number" min="1" max="240" value="${profile.minimum}"></label>
          <label class="field"><span>Max. Laufzeit (min)</span><input id="profileMaximum${profile.id}" type="number" min="1" max="240" value="${profile.maximum}"></label>
        </div>
        <div class="profileFormula">
          Saisonlaufzeit → Profil ${Number(profile.correction)>=0?'+':''}${profile.correction} % → wetterabhängige Empfindlichkeiten
        </div>
        <div style="margin-top:10px"><button onclick="saveProfile(${profile.id})">Profil speichern</button></div>
      </div>
    `).join('');

    const select=document.getElementById('editProfile');
    if(select){
        const selected=select.value;
        select.innerHTML=profileCache.map(
            profile=>`<option value="${profile.id}">${esc(profile.name)}</option>`
        ).join('');
        select.value=selected;
    }
    fillSimulatorSelectors();
}

async function loadProfiles(){
    try{
        const data=await api('/api/profiles');
        profileCache=data.profiles||[];
        renderProfileEditor();
        renderGardenProfileSelector();
        renderGardenMap();
    }catch(e){
        setSaveState(
            'profileSaveState',
            'Profile konnten nicht geladen werden: '+e.message,
            'errmsg'
        );
    }
}

async function saveProfile(id){
    const value=name=>document.getElementById(name+id).value;

    const data={
        id,
        name:value('profileName').trim(),
        symbol:value('profileSymbol').trim(),
        correction:Number(value('profileCorrection')),
        temperature:Number(value('profileTemperature')),
        humidity:Number(value('profileHumidity')),
        rain:Number(value('profileRain')),
        minimum:Number(value('profileMinimum')),
        maximum:Number(value('profileMaximum'))
    };

    try{
        await api('/api/profile/save',{
            method:'POST',
            headers:{'Content-Type':'application/x-www-form-urlencoded'},
            body:new URLSearchParams(data)
        });

        setSaveState(
            'profileSaveState',
            'Profil gespeichert',
            'okmsg'
        );

        await loadProfiles();
        await loadPrograms();
        await loadStatus();
    }catch(e){
        setSaveState(
            'profileSaveState',
            'Fehler: '+e.message,
            'errmsg'
        );
    }
}

async function resetProfiles(){
    if(!confirm('Alle Pflanzenprofile auf Standardwerte zurücksetzen?'))return;

    try{
        await api('/api/profiles/reset',{method:'POST'});
        setSaveState(
            'profileSaveState',
            'Standardprofile wiederhergestellt',
            'okmsg'
        );
        await loadProfiles();
        await loadPrograms();
        await loadStatus();
    }catch(e){
        setSaveState(
            'profileSaveState',
            'Fehler: '+e.message,
            'errmsg'
        );
    }
}

function advisorContribution(s,name){
    const factor=(s.advisorFactors||[]).find(
        item=>String(item.name||'').toLowerCase().includes(name)
    );
    return Number(factor?factor.contribution:0);
}

function advisorNextDuration(s){
    if(!s.advisorValid||!programCache.length)return '';

    const now=parseControllerNow();
    const next=allOccurrences(now,8)[0];
    if(!next)return '';

    const profile=profileById(next.program.profileId);
    const standard=Number(next.program.durationMinutes||0);
    const seasonPercent=Number(s.advisorSeasonPercent||100);

    const seasonal=Math.max(
        1,
        Math.round(standard*seasonPercent/100)
    );

    const profiled=Math.max(
        profile.minimum,
        Math.min(
            profile.maximum,
            Math.round(
                seasonal*(100+Number(profile.correction||0))/100
            )
        )
    );

    const temperature=
        advisorContribution(s,'temperatur')*
        Number(profile.temperature||100)/100;

    const humidity=
        advisorContribution(s,'luftfeuchte')*
        Number(profile.humidity||100)/100;

    const rain=
        advisorContribution(s,'regen')*
        Number(profile.rain||100)/100;

    const weatherPercent=Math.round(
        temperature+humidity+rain
    );

    const recommended=
        weatherPercent<=-60
            ? 0
            : Math.max(
                profile.minimum,
                Math.min(
                    profile.maximum,
                    Math.round(
                        profiled*(100+weatherPercent)/100
                    )
                )
            );

    if(recommended===0){
        return `${profile.name}: Standard ${standard} min → Saison ${seasonal} min → Profil ${profiled} min → heute aussetzen`;
    }

    return `${profile.name}: Standard ${standard} min → Saison ${seasonal} min → Profil ${profiled} min → Wetter ${recommended} min`;
}

function updateAdvisor(s){
 const headline=document.getElementById('advisorHeadline'),summary=document.getElementById('advisorSummary'),narrative=document.getElementById('advisorNarrative'),badge=document.getElementById('advisorBadge'),factors=document.getElementById('advisorFactors'),reasons=document.getElementById('advisorReasons'),duration=document.getElementById('advisorDuration'),confidence=document.getElementById('advisorConfidenceText'),confidenceFill=document.getElementById('advisorConfidenceFill');
 headline.textContent=s.advisorHeadline||'Keine Empfehlung';summary.textContent=s.advisorSummary||'';narrative.textContent=s.advisorNarrative||'';
 if(!s.advisorValid){badge.textContent='WARTET';badge.className='badge off';}else if(s.advisorAdjustment<=-60){badge.textContent='PAUSE';badge.className='badge warn';}else if(s.advisorAdjustment<0){badge.textContent=s.advisorAdjustment+' %';badge.className='badge warn';}else if(s.advisorAdjustment>0){badge.textContent='+'+s.advisorAdjustment+' %';badge.className='badge ok';}else{badge.textContent='NORMAL';badge.className='badge ok';}
 factors.innerHTML=(s.advisorFactors||[]).map(f=>{const c=Number(f.contribution||0),cls=c>0?'ok':(c<0?'warn':'off');return `<div class="advisorFactor"><span>${esc(f.name)}</span><span>${esc(f.value)}</span><span class="badge ${cls}">${signedPercent(c)}</span></div>`;}).join('')||'<div class="dashboardSub" style="padding:10px">Noch keine Einzelfaktoren verfügbar</div>';
 reasons.innerHTML=(s.advisorReasons||[]).map(reason=>`<div>✓ ${esc(reason)}</div>`).join('');duration.textContent=advisorNextDuration(s);const cv=Number(s.advisorConfidence||0);confidence.textContent=`Vertrauen: ${cv} % · ${confidenceText(cv)}`;confidenceFill.style.width=Math.max(0,Math.min(100,cv))+'%';
}
function formatUptime(seconds){
    seconds=Math.max(0,Number(seconds||0));
    const days=Math.floor(seconds/86400);
    const hours=Math.floor((seconds%86400)/3600);
    const minutes=Math.floor((seconds%3600)/60);
    if(days>0)return `${days} T ${hours} h ${minutes} min`;
    if(hours>0)return `${hours} h ${minutes} min`;
    return `${minutes} min`;
}
function formatBytes(value){
    const bytes=Number(value||0);
    if(bytes>=1048576)return (bytes/1048576).toFixed(1)+' MB';
    return Math.round(bytes/1024)+' kB';
}
function formatBackupEpoch(epoch){
    const value=Number(epoch||0);
    if(value<=0)return 'noch keines seit Start';
    return new Date(value*1000).toLocaleString('de-DE',{
        day:'2-digit',month:'2-digit',year:'numeric',
        hour:'2-digit',minute:'2-digit'
    });
}
function updateSystemHealth(s){
    const warnings=[];
    const critical=[];

    const freeHeap=Number(s.freeHeap||0);
    const psramTotal=Number(s.psramTotal||0);
    const freePsram=Number(s.freePsram||0);

    if(!s.wifi)warnings.push('WLAN');
    if(!s.timeValid)warnings.push('NTP');
    if(!s.weatherValid)warnings.push('Wetter');

    // ESP32-S3 mit PSRAM: interner Heap wird getrennt bewertet.
    if(freeHeap<20000){
        critical.push('Speicher');
    }else if(freeHeap<30000){
        warnings.push('Speicher');
    }

    // PSRAM ist auf diesem Board vorhanden. Nur wirklich niedrige
    // Restwerte sollen den Gesamtstatus beeinflussen.
    if(psramTotal>0){
        if(freePsram<131072){
            critical.push('PSRAM');
        }else if(freePsram<262144){
            warnings.push('PSRAM');
        }
    }

    const headline=document.getElementById('healthHeadline');
    const healthBadge=document.getElementById('healthBadge');

    if(critical.length>0){
        headline.textContent='Kritisch: '+critical.join(', ');
        healthBadge.textContent='KRITISCH';
        healthBadge.className='badge off';
    }else if(warnings.length>0){
        headline.textContent='Prüfen: '+warnings.join(', ');
        healthBadge.textContent='WARNUNG';
        healthBadge.className='badge warn';
    }else{
        headline.textContent='GardenFlow betriebsbereit';
        healthBadge.textContent='OK';
        healthBadge.className='badge ok';
    }

    document.getElementById('healthFirmware').textContent=s.firmwareVersion+' · '+s.buildDate;
    document.getElementById('healthUptime').textContent=formatUptime(s.uptimeSeconds);

    const heapState=
        freeHeap<20000?' · kritisch':
        freeHeap<30000?' · wenig':
        ' · OK';
    document.getElementById('healthHeap').textContent=
        formatBytes(freeHeap)+heapState;

    if(psramTotal>0){
        const psramState=
            freePsram<131072?' · kritisch':
            freePsram<262144?' · wenig':
            ' · OK';
        document.getElementById('healthPsram').textContent=
            formatBytes(freePsram)+' / '+formatBytes(psramTotal)+psramState;
    }else{
        document.getElementById('healthPsram').textContent='nicht vorhanden';
    }

    document.getElementById('healthWifi').textContent=
        s.wifi ? `${s.ssid} · ${s.rssi} dBm` : 'getrennt';
    document.getElementById('healthNtp').textContent=s.timeValid?'synchronisiert':'wartet';
    document.getElementById('healthWeather').textContent=
        s.weatherValid?`${Number(s.temperature||0).toFixed(1)} °C · ${s.humidity} %`:'nicht verfügbar';
    document.getElementById('healthOta').textContent=s.otaReady?'bereit':'nicht bereit';
    document.getElementById('healthBackup').textContent=formatBackupEpoch(s.lastBackupEpoch);
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

async function loadStatus(){try{const s=await api('/api/status');document.getElementById('clock').textContent=s.date+' '+s.time;document.getElementById('address').textContent=s.ssid+' · '+s.ip+' · '+s.rssi+' dBm';badge('wifi',s.wifi?'verbunden':'getrennt',s.wifi?'ok':'off');badge('timeState',s.timeValid?'synchronisiert':'wartet',s.timeValid?'ok':'warn');badge('autoState',s.rainPause?'Regenpause':(s.timeValid?'bereit':'gesperrt'),s.rainPause?'warn':(s.timeValid?'ok':'warn'));document.getElementById('weatherMain').textContent=s.weatherValid?(s.temperature.toFixed(1)+' °C · '+s.weatherDescription):(s.weatherConfigured?'wartet auf Daten':'nicht eingerichtet');document.getElementById('weatherDetails').textContent=s.weatherValid?('Feuchte '+s.humidity+' % · Regen '+s.rainMm.toFixed(1)+' mm/24h · Risiko '+s.rainProbability+' %'):(s.weatherError||'OpenWeather API-Schluessel eintragen');badge('rainPause',s.rainPause?'AKTIV':(s.weatherPauseEnabled?'bereit':'aus'),s.rainPause?'warn':(s.weatherPauseEnabled?'ok':'off'));const previousGardenRunActive=
    !!lastStatus &&
    (
        !!lastStatus.running ||
        !!lastStatus.manualRun
    );

lastStatus=s;

const currentGardenRunActive=
    !!s.running ||
    !!s.manualRun;

if(previousGardenRunActive &&
   !currentGardenRunActive){
    void loadHistory();
}

const menuState=document.getElementById('menuConnectionState');
if(menuState){
    menuState.textContent=s.wifi
        ? `● Online · ${s.rssi} dBm`
        : '● WLAN getrennt';
}
updateDashboard(s);updateSystemHealth(s);updateAdvisor(s);updateWater(s);updateGardenLiveState();renderGardenZoneStatistics();if(document.querySelector('[data-page="weekplan"].pageActive'))renderWeeklyCalendar();if(!weatherDirty)fillWeatherForm(s);if(!smartDirty)fillSmartForm(s);renderNextProgram();renderUpcomingPrograms();renderAllPrograms(s.running);fillSimulatorSelectors();badge('vacationState',s.vacationActive?'AKTIV':(s.vacationEnabled?'geplant':'aus'),s.vacationActive?'warn':(s.vacationEnabled?'ok':'off'));document.getElementById('running').textContent=s.running?('Programm '+s.programId+' · Ventil '+(s.valve+1)):'Kein Programm';document.getElementById('remaining').textContent=s.running?(s.remaining+' Sekunden verbleibend'):'Bereit';document.getElementById('stop').disabled=!s.running;document.getElementById('valves').innerHTML=s.valves.map(v=>`<div class="row"><span>${esc(v.name)}</span><span><span class="badge ${v.pulseActive?'warn':(v.open?'ok':'off')}">${v.pulseActive?'SCHALTET…':(v.open?'OFFEN':'GESCHLOSSEN')}</span> <button class="secondary" ${(s.running||v.pulseActive)?'disabled':''} onclick="toggleValve(${v.index},this)">Umschalten</button></span></div>`).join('')}catch(e){document.getElementById('address').innerHTML='<span class="error">Verbindung unterbrochen</span>'}}
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
    metaEl.textContent=`Programm ${next.program.id} · ${next.program.profileName||'Allgemein'} · Ventil ${next.program.valve+1} · ${next.program.durationMinutes} min · ${durationUntil(next.date,now)}`;
    document.getElementById('dashNextTime').textContent=`${whenText(next.date,now)} ${formatTime(next.date)}`;
    document.getElementById('dashNextMeta').textContent=`Programm ${next.program.id} · ${next.program.profileName||'Allgemein'} · Ventil ${next.program.valve+1} · ${next.program.durationMinutes} min · ${durationUntil(next.date,now)}`;
    whenEl.textContent=whenText(next.date,now);
    whenEl.className='badge ok';
}

function scheduleItemHtml(entry){
    const p=entry.program;
    return `<div class="scheduleItem"><div class="scheduleTime">${formatTime(entry.date)}</div><div><b>Programm ${p.id} · ${esc(p.profileName||'Allgemein')}</b><div class="scheduleValve">Ventil ${p.valve+1} · ${p.durationMinutes} min</div></div><button class="secondary" onclick="editProgram(${p.index})">Bearbeiten</button></div>`;
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
            return `<div class="program ${x.enabled?'':'programInactive'}"><div class="row"><div><b>Programm ${x.id}</b> · ${esc(x.profileName||'Allgemein')} · Ventil ${x.valve+1}<div class="days">${esc(x.days)} · ${String(x.hour).padStart(2,'0')}:${String(x.minute).padStart(2,'0')} · ${x.durationMinutes} min · ${nextText}</div></div><div><button class="secondary" onclick="editProgram(${x.index})">Bearbeiten</button> <button class="secondary" onclick="post('/api/program/copy',{index:${x.index}})">Kopieren</button> <button class="secondary" onclick="post('/api/program/toggle',{index:${x.index}})">${x.enabled?'Aus':'Ein'}</button> <button class="stop" onclick="deleteProgram(${x.index})">Löschen</button> <button ${(!x.enabled||running)?'disabled':''} onclick="post('/api/program/start?index=${x.index}')">Start</button></div></div></div>`;
          }).join('')
        : '<div class="muted">Keine Programme vorhanden</div>';
}


let weekGridStepMinutes=5;

function loadWeekGridStep(){
    try{
        const stored=Number(localStorage.getItem('gardenflowWeekGridStep')||5);
        weekGridStepMinutes=[5,15,30].includes(stored)?stored:5;
    }catch(e){
        weekGridStepMinutes=5;
    }
    updateWeekGridButtons();
}

function setWeekGridStep(step){
    const value=Number(step);
    if(![5,15,30].includes(value))return;
    weekGridStepMinutes=value;
    try{localStorage.setItem('gardenflowWeekGridStep',String(value));}catch(e){}
    updateWeekGridButtons();
    renderWeeklyCalendar();
}

function updateWeekGridButtons(){
    document.querySelectorAll('[data-grid-step]').forEach(button=>{
        button.classList.toggle('active',Number(button.dataset.gridStep)===weekGridStepMinutes);
    });
}

function snapWeekMinutes(value){
    return Math.round(Number(value)/weekGridStepMinutes)*weekGridStepMinutes;
}

let weekInteraction=null;

function weekSetState(text,type=''){
    const el=document.getElementById('weekSaveState');
    if(!el)return;
    el.textContent=text||'';
    el.className='weekSaveState'+(type?' '+type:'');
}

function weekShiftMask(mask,delta){
    let result=0;
    for(let day=0;day<7;day++){
        if(mask&(1<<day)){
            const shifted=((day+delta)%7+7)%7;
            result|=(1<<shifted);
        }
    }
    return result&0x7f;
}

function weekPointerToSlot(clientX,clientY,grabOffsetMinutes=0){
    const planner=document.getElementById('weekPlanner');
    if(!planner)return null;
    const rect=planner.getBoundingClientRect();
    const headerHeight=42;
    const timeWidth=64;
    const usableWidth=Math.max(1,rect.width-timeWidth);
    const dayWidth=usableWidth/7;
    const x=clientX-rect.left-timeWidth;
    const y=clientY-rect.top-headerHeight;

    const day=Math.max(0,Math.min(6,Math.floor(x/dayWidth)));
    const pointerMinutes=(y/48)*60;
    const rawStartMinutes=pointerMinutes-grabOffsetMinutes;
    const totalMinutes=Math.max(0,Math.min(1435,snapWeekMinutes(rawStartMinutes)));

    return {
        day,
        hour:Math.floor(totalMinutes/60),
        minute:totalMinutes%60,
        totalMinutes
    };
}

function cancelWeekInteraction(){
    if(!weekInteraction)return;
    const state=weekInteraction;
    weekInteraction=null;
    window.removeEventListener('pointermove',weekEventPointerMove);
    state.element.classList.remove('dragging','resizing');
    if(state.targetBadge){
        state.targetBadge.remove();
        state.targetBadge=null;
    }
    weekSetState('Änderung abgebrochen');
    renderWeeklyCalendar();
}

function weekKeyDown(event){
    if(event.key==='Escape' && weekInteraction){
        event.preventDefault();
        cancelWeekInteraction();
    }
}

window.addEventListener('keydown',weekKeyDown);

function weekEventPointerDown(event){
    if(event.button!==undefined&&event.button!==0)return;

    const element=event.currentTarget;
    const programIndex=Number(element.dataset.programIndex);
    const occurrenceDay=Number(element.dataset.day);
    const program=programCache.find(p=>Number(p.index)===programIndex);
    if(!program)return;

    const resize=event.target.classList.contains('weekResize');

    const elementRect=element.getBoundingClientRect();
    const elementHeight=Math.max(1,elementRect.height);
    const grabRatio=Math.max(0,Math.min(1,(event.clientY-elementRect.top)/elementHeight));
    const grabOffsetMinutes=grabRatio*Number(program.durationMinutes||1);

    weekInteraction={
        mode:resize?'resize':'move',
        element,
        program,
        occurrenceDay,
        startX:event.clientX,
        startY:event.clientY,
        originalHour:Number(program.hour),
        originalMinute:Number(program.minute),
        originalDuration:Number(program.durationMinutes),
        originalDays:Number(program.weekdays),
        grabOffsetMinutes,
        changed:false,
        targetBadge:null
    };

    element.setPointerCapture?.(event.pointerId);

    window.addEventListener('pointermove',weekEventPointerMove,{passive:false});
    window.addEventListener('pointerup',weekEventPointerUp,{once:true});
    event.preventDefault();
    event.stopPropagation();
}

function weekEventPointerMove(event){
    if(!weekInteraction)return;

    const state=weekInteraction;
    const distance=Math.hypot(
        event.clientX-state.startX,
        event.clientY-state.startY
    );

    /*
     * Erst ab 5 Pixel Bewegung wird aus einem normalen Klick
     * wirklich eine Drag-/Resize-Aktion.
     */
    if(!state.changed && distance<5){
        return;
    }

    if(!state.changed){
        state.changed=true;
        state.element.classList.add(
            state.mode==='resize'?'resizing':'dragging'
        );
        if(state.mode==='move'){
            const badge=document.createElement('div');
            badge.className='weekTargetBadge';
            badge.textContent='--:--';
            document.body.appendChild(badge);
            state.targetBadge=badge;
        }
    }

    event.preventDefault();

    if(state.mode==='resize'){
        const rawDelta=((event.clientY-state.startY)/48)*60;
        const deltaMinutes=snapWeekMinutes(rawDelta);
        const duration=Math.max(1,Math.min(240,state.originalDuration+deltaMinutes));
        const snapped=Math.max(1,snapWeekMinutes(duration));
        state.previewDuration=snapped;
        state.element.style.height=`${Math.max(18,(snapped/60)*48)}px`;
        const meta=state.element.querySelector('.weekEventMeta');
        if(meta)meta.textContent=`V${Number(state.program.valve)+1} · ${snapped} min`;
    }else{
        const slot=weekPointerToSlot(event.clientX,event.clientY,state.grabOffsetMinutes);
        if(!slot)return;

        state.previewSlot=slot;

        const title=state.element.querySelector('.weekEventTitle');
        const time=`${String(slot.hour).padStart(2,'0')}:${String(slot.minute).padStart(2,'0')}`;
        if(title){
            title.textContent=`${time} · ${state.program.profileName||'Allgemein'}`;
        }
        if(state.targetBadge){
            state.targetBadge.textContent=`${['Mo','Di','Mi','Do','Fr','Sa','So'][slot.day]} ${time} · ${weekGridStepMinutes} min`;
            state.targetBadge.style.left=`${event.clientX+14}px`;
            state.targetBadge.style.top=`${event.clientY+14}px`;
        }
    }
}

async function weekEventPointerUp(event){
    if(!weekInteraction)return;

    const state=weekInteraction;
    weekInteraction=null;

    window.removeEventListener('pointermove',weekEventPointerMove);

    state.element.classList.remove('dragging','resizing');

    if(state.targetBadge){state.targetBadge.remove();state.targetBadge=null;}

    /*
     * Keine echte Bewegung:
     * - normaler Balken -> Programmeditor öffnen
     * - Resize-Griff -> nichts ändern
     */
    if(!state.changed){
        if(state.mode==='move'){
            editProgram(state.program.index);
        }else{
            renderWeeklyCalendar();
        }
        return;
    }

    try{
        weekSetState('Änderung wird gespeichert …');

        if(state.mode==='resize'){
            const duration=Number(state.previewDuration||state.originalDuration);
            await post('/api/program/update',{
                index:state.program.index,
                duration
            });
        }else{
            const slot=state.previewSlot||weekPointerToSlot(event.clientX,event.clientY,state.grabOffsetMinutes);
            if(!slot){
                renderWeeklyCalendar();
                return;
            }

            const dayDelta=slot.day-state.occurrenceDay;
            const days=weekShiftMask(state.originalDays,dayDelta);

            await post('/api/program/update',{
                index:state.program.index,
                hour:slot.hour,
                minute:slot.minute,
                days
            });
        }

        await loadPrograms();
        weekSetState('Wochenplan gespeichert','okmsg');
    }catch(error){
        renderWeeklyCalendar();
        weekSetState('Speichern fehlgeschlagen: '+error.message,'errmsg');
    }
}

function renderWeeklyCalendar(){
    updateWeekGridButtons();
    const planner=document.getElementById('weekPlanner');
    if(!planner)return;

    const dayLabels=['Mo','Di','Mi','Do','Fr','Sa','So'];
    const cellHeight=48;
    let html='<div class="weekCorner">Zeit</div>';
    dayLabels.forEach(day=>html+=`<div class="weekDayHead">${day}</div>`);

    for(let hour=0;hour<24;hour++){
        html+=`<div class="weekTime">${String(hour).padStart(2,'0')}:00</div>`;
        for(let day=0;day<7;day++){
            html+=`<div class="weekCell" data-week-day="${day}" data-week-hour="${hour}"></div>`;
        }
    }

    planner.innerHTML=html;

    const cells=[...planner.querySelectorAll('.weekCell')];

    programCache.forEach(program=>{
        if(!program.weekdays)return;

        for(let day=0;day<7;day++){
            if((program.weekdays&(1<<day))===0)continue;

            const hour=Number(program.hour||0);
            const minute=Number(program.minute||0);
            const duration=Math.max(1,Number(program.durationMinutes||1));

            const cell=cells.find(c=>
                Number(c.dataset.weekDay)===day &&
                Number(c.dataset.weekHour)===hour
            );
            if(!cell)continue;

            const event=document.createElement('div');
            event.className=`weekEvent v${Number(program.valve)+1}${program.enabled?'':' disabled'}`;

            const top=(minute/60)*cellHeight;
            const height=Math.max(18,(duration/60)*cellHeight);

            event.style.top=`${top}px`;
            event.style.height=`${height}px`;

            const start=`${String(hour).padStart(2,'0')}:${String(minute).padStart(2,'0')}`;
            const profile=program.profileName||'Allgemein';
            event.title=`Programm ${program.id}\nVentil ${Number(program.valve)+1}\nProfil ${profile}\nStart ${start}\nDauer ${duration} min\n${program.enabled?'Aktiv':'Inaktiv'}`;
            event.innerHTML=`<div class="weekEventTitle">${esc(start)} · ${esc(profile)}</div><div class="weekEventMeta">V${Number(program.valve)+1} · ${duration} min</div><div class="weekResize" title="Laufzeit ändern"></div>`;
            event.dataset.programIndex=String(program.index);
            event.dataset.day=String(day);
            event.addEventListener('pointerdown',weekEventPointerDown);
            event.addEventListener('dblclick',e=>{
                e.preventDefault();
                e.stopPropagation();
                editProgram(program.index);
            });
            cell.appendChild(event);
        }
    });

    // Aktuelle Uhrzeit als Orientierungslinie, wenn Controllerzeit verfügbar ist.
    if(lastStatus&&lastStatus.time){
        const parts=String(lastStatus.time).split(':');
        if(parts.length>=2){
            const hours=Number(parts[0]),minutes=Number(parts[1]);
            if(Number.isFinite(hours)&&Number.isFinite(minutes)){
                const line=document.createElement('div');
                line.className='weekNowLine';
                line.style.top=`${42 + (hours + minutes/60)*cellHeight}px`;
                line.innerHTML=`<span class="weekNowLabel">${String(hours).padStart(2,'0')}:${String(minutes).padStart(2,'0')}</span>`;
                planner.appendChild(line);
            }
        }
    }
}

async function loadPrograms(){
    try{
        const p=await api('/api/programs');
        programCache=p.programs;
        renderNextProgram();
        renderUpcomingPrograms();
        renderAllPrograms(p.running);
        renderWeeklyCalendar();
        renderGardenProgramSelector();
        renderGardenMap();
        renderGardenZoneStatistics();
    }catch(e){
        document.getElementById('programs').innerHTML='<div class="error">Programme konnten nicht geladen werden</div>';
        document.getElementById('upcomingPrograms').innerHTML='<div class="error">Zeitplan konnte nicht geladen werden</div>';
    }
}
let editorIndex=-1,editorDays=127;
const dayNames=['Mo','Di','Mi','Do','Fr','Sa','So'];
function renderWeekdays(){document.getElementById('weekdayButtons').innerHTML=dayNames.map((n,i)=>`<button type="button" class="day ${(editorDays&(1<<i))?'active':''}" onclick="toggleEditorDay(${i})">${n}</button>`).join('')}
function toggleEditorDay(i){editorDays^=(1<<i);renderWeekdays()}
function openEditor(x){editorIndex=x?x.index:-1;editorDays=x?x.weekdays:127;document.getElementById('editorTitle').textContent=x?`Programm ${x.id} bearbeiten`:'Neues Programm';document.getElementById('editValve').value=String(x?x.valve:0);document.getElementById('editProfile').value=String(x?x.profileId:0);document.getElementById('editTime').value=`${String(x?x.hour:6).padStart(2,'0')}:${String(x?x.minute:0).padStart(2,'0')}`;document.getElementById('editDuration').value=String(x?x.durationMinutes:15);document.getElementById('editEnabled').value=x&&x.enabled?'1':'0';renderWeekdays();document.getElementById('editorModal').classList.add('open')}
function closeEditor(){document.getElementById('editorModal').classList.remove('open')}
function modalBackdrop(e){if(e.target.id==='editorModal')closeEditor()}
async function saveEditor(){const time=document.getElementById('editTime').value.split(':');const duration=Number(document.getElementById('editDuration').value);if(time.length!==2||duration<1||duration>1440||editorDays===0){alert(editorDays===0?'Mindestens einen Wochentag auswählen':'Bitte gültige Werte eingeben');return}const d={valve:Number(document.getElementById('editValve').value),profile:Number(document.getElementById('editProfile').value),hour:Number(time[0]),minute:Number(time[1]),duration,days:editorDays,enabled:Number(document.getElementById('editEnabled').value)};if(editorIndex<0)await post('/api/program/create',d);else{d.index=editorIndex;await post('/api/program/update',d)}closeEditor()}
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
                seasonAuto:Number(document.getElementById('seasonAutomatic').value),
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
async function downloadGardenFlowBackup(){
    const state=document.getElementById('backupState');
    try{
        if(state){state.textContent='Backup wird erstellt …';state.className='saveState';}
        const response=await fetch('/api/backup',{cache:'no-store'});
        if(!response.ok)throw new Error(await response.text()||('HTTP '+response.status));
        const backup=await response.json();
        const now=new Date();
        const pad=value=>String(value).padStart(2,'0');
        const filename=`gardenflow_backup_${now.getFullYear()}-${pad(now.getMonth()+1)}-${pad(now.getDate())}_${pad(now.getHours())}${pad(now.getMinutes())}${pad(now.getSeconds())}.json`;
        const blob=new Blob([JSON.stringify(backup,null,2)],{type:'application/json'});
        const url=URL.createObjectURL(blob);
        const link=document.createElement('a');
        link.href=url;
        link.download=filename;
        document.body.appendChild(link);
        link.click();
        link.remove();
        URL.revokeObjectURL(url);
        if(state){state.textContent='Backup heruntergeladen: '+filename;state.className='saveState okmsg';}
    }catch(e){
        if(state){state.textContent='Backup fehlgeschlagen: '+e.message;state.className='saveState errmsg';}
    }
}


async function restoreGardenFlowBackup(){
    const state=document.getElementById('backupState');
    const input=document.getElementById('backupRestoreFile');
    const file=input&&input.files?input.files[0]:null;

    if(!file){
        if(state){
            state.textContent='Bitte zuerst eine Backup-Datei auswählen.';
            state.className='saveState errmsg';
        }
        return;
    }

    if(!confirm(
        'Backup wirklich wiederherstellen?\\n\\n'+
        'Programme, Profile und Einstellungen werden überschrieben. '+
        'WLAN-Passwort, API-Key und Verbrauchsstatistik bleiben erhalten.'
    )){
        return;
    }

    try{
        if(state){
            state.textContent='Backup wird geprüft und wiederhergestellt …';
            state.className='saveState';
        }

        const text=await file.text();
        JSON.parse(text);

        const response=await fetch('/api/backup/restore',{
            method:'POST',
            headers:{'Content-Type':'application/json'},
            body:text
        });

        const result=await response.json();

        if(!response.ok){
            throw new Error(result.error||('HTTP '+response.status));
        }

        if(state){
            state.textContent=
                'Wiederherstellung erfolgreich: '+
                (result.message||'OK')+
                '. GardenFlow startet neu …';
            state.className='saveState okmsg';
        }

        setTimeout(()=>location.reload(),3500);
    }catch(e){
        if(state){
            state.textContent='Restore fehlgeschlagen: '+e.message;
            state.className='saveState errmsg';
        }
    }
}


let historyCache=[];

function historyEventLabel(entry){
    if(entry.event==='start')return entry.automatic?'Automatik gestartet':'Manuell gestartet';
    if(entry.event==='stop')return entry.reason==='user_stop'?'Bewässerung abgebrochen':'Bewässerung beendet';
    if(entry.event==='skipped'){
        if(entry.reason==='weather_pause')return 'Wegen Regenpause übersprungen';
        if(entry.reason==='vacation')return 'Wegen Urlaubsmodus übersprungen';
        return 'Bewässerung übersprungen';
    }
    return entry.event||'Ereignis';
}
function historyTimestamp(epoch){
    const value=Number(epoch||0);
    if(value<=0)return 'Zeit unbekannt';
    return new Date(value*1000).toLocaleString('de-DE',{day:'2-digit',month:'2-digit',year:'numeric',hour:'2-digit',minute:'2-digit'});
}
function historyDuration(seconds){
    const total=Math.max(0,Number(seconds||0));
    const minutes=Math.floor(total/60);
    const rest=total%60;
    return rest?`${minutes} min ${rest} s`:`${minutes} min`;
}
function filteredHistory(){
    const period=document.getElementById('historyPeriod')?.value||'all';
    const eventFilter=document.getElementById('historyEventFilter')?.value||'';
    const valveFilter=document.getElementById('historyValveFilter')?.value??'';
    const now=Date.now()/1000;
    let minEpoch=0;
    if(period==='today'){const d=new Date();d.setHours(0,0,0,0);minEpoch=d.getTime()/1000;}
    else if(period==='week')minEpoch=now-7*86400;
    else if(period==='month')minEpoch=now-30*86400;
    return historyCache.filter(entry=>{
        if(minEpoch>0&&Number(entry.timestamp||0)<minEpoch)return false;
        if(eventFilter&&entry.event!==eventFilter)return false;
        if(valveFilter!==''&&String(entry.valve)!==String(valveFilter))return false;
        return true;
    });
}
function renderHistory(){
    const list=document.getElementById('historyList');
    if(!list)return;
    const entries=filteredHistory();
    const total=document.getElementById('historyCount');
    if(total)total.textContent=`${entries.length} sichtbar · ${historyCache.length} geladen`;
    if(!entries.length){list.innerHTML='<div class="historyEmpty">Keine passenden Historieneinträge.</div>';return;}
    list.innerHTML=entries.map(entry=>{
        const stopped=entry.event==='stop';
        const skipped=entry.event==='skipped';
        const plan=entry.event==='start'?`Geplant ${historyDuration(entry.plannedSeconds)}`:stopped?`Ist ${historyDuration(entry.actualSeconds)} · Soll ${historyDuration(entry.plannedSeconds)}`:`Geplant ${historyDuration(entry.plannedSeconds)}`;
        const amount=stopped&&Number(entry.liters)>0?`${Number(entry.liters).toFixed(1)} l · ${Number(entry.costEuro).toFixed(2)} €`:(skipped?'nicht ausgeführt':'');
        return `<div class="historyRow ${skipped?'historySkipped':''}"><div>${esc(historyTimestamp(entry.timestamp))}</div><div><div class="historyTitle">${esc(historyEventLabel(entry))}</div><div class="historyMeta">Programm ${entry.programId} · Ventil ${entry.valve+1} · Profil ${entry.profile} · ${esc(plan)} · Advisor ${entry.advisorPercent>=0?'+':''}${entry.advisorPercent}% · Saison ${entry.seasonPercent}%</div></div><div class="historyValue">${esc(amount)}</div></div>`;
    }).join('');
}
async function loadHistory(){
    const list=document.getElementById('historyList');
    try{
        const data=await api('/api/history?limit=200');
        historyCache=Array.isArray(data.entries)?data.entries:[];
        renderHistory();
        renderGardenZoneStatistics();
    }catch(e){
        historyCache=[];
        if(list)list.innerHTML='<div class="historyEmpty error">Historie konnte nicht geladen werden.</div>';
    }
}
function csvEscape(value){const text=String(value??'');return `"${text.replace(/"/g,'""')}"`;}
function downloadTextFile(filename,text,mime){
    const blob=new Blob([text],{type:mime});
    const url=URL.createObjectURL(blob);
    const link=document.createElement('a');
    link.href=url;link.download=filename;document.body.appendChild(link);link.click();link.remove();URL.revokeObjectURL(url);
}
function historyExportFilename(ext){
    const d=new Date(),pad=v=>String(v).padStart(2,'0');
    return `gardenflow_history_${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}_${pad(d.getHours())}${pad(d.getMinutes())}.${ext}`;
}
function downloadHistoryCsv(){
    const rows=filteredHistory();
    const header=['ID','Datum','Ereignis','Grund','Programm','Ventil','Profil','Soll Sekunden','Ist Sekunden','Liter','Kosten EUR','Advisor %','Saison %','Automatisch','Firmware'];
    const lines=[header.map(csvEscape).join(';')];
    rows.forEach(entry=>lines.push([entry.id,historyTimestamp(entry.timestamp),historyEventLabel(entry),entry.reason,entry.programId,Number(entry.valve)+1,entry.profile,entry.plannedSeconds,entry.actualSeconds,Number(entry.liters||0).toFixed(2),Number(entry.costEuro||0).toFixed(3),entry.advisorPercent,entry.seasonPercent,entry.automatic?'ja':'nein',entry.firmware].map(csvEscape).join(';')));
    downloadTextFile(historyExportFilename('csv'),'\ufeff'+lines.join('\n'),'text/csv;charset=utf-8');
}
function downloadHistoryJson(){
    const rows=filteredHistory();
    downloadTextFile(historyExportFilename('json'),JSON.stringify({exportedAt:new Date().toISOString(),count:rows.length,entries:rows},null,2),'application/json');
}


let gardenZones=[];
let selectedGardenZoneId=null;
let gardenInteraction=null;
let gardenPolygonEditMode=false;
let gardenVertexInteraction=null;
let gardenSelectedVertex=-1;
let gardenBackground={dataUrl:'',opacity:0.45,scale:1,x:0,y:0,fitToPlan:false};
let gardenBackgroundMoveMode=false;
let gardenBackgroundDrag=null;
let gardenMetric={widthM:0,heightM:0};


function gardenBackgroundStorageKey(){return 'gardenflowBackgroundV1';}
function gardenSaveBackgroundLocal(){
    try{localStorage.setItem(gardenBackgroundStorageKey(),JSON.stringify(gardenBackground));return true;}
    catch(error){setGardenStatus('Hintergrund konnte nicht gespeichert werden: '+error.message,'errmsg');return false;}
}
function gardenLoadBackgroundLocal(){
    try{
        const raw=localStorage.getItem(gardenBackgroundStorageKey());
        if(raw){
            const v=JSON.parse(raw);
            gardenBackground={dataUrl:String(v.dataUrl||''),opacity:Math.max(.1,Math.min(1,Number(v.opacity??.45))),scale:Math.max(.25,Math.min(3,Number(v.scale??1))),x:Number(v.x||0),y:Number(v.y||0),fitToPlan:Boolean(v.fitToPlan)};
        }
    }catch(e){}
    renderGardenBackground();
}
function renderGardenBackground(){
    const canvas=document.getElementById('gardenCanvas');if(!canvas)return;
    let layer=document.getElementById('gardenBackgroundLayer');
    if(!layer){layer=document.createElement('div');layer.id='gardenBackgroundLayer';layer.className='gardenBackgroundLayer';canvas.prepend(layer);}
    layer.innerHTML='';
    if(gardenBackground.dataUrl){
        const img=document.createElement('img');img.className='gardenBackgroundImage';img.src=gardenBackground.dataUrl;img.alt='Garten-Hintergrund';
        img.style.opacity=String(gardenBackground.opacity);if(gardenBackground.fitToPlan){img.style.left='0';img.style.top='0';img.style.width='100%';img.style.height='100%';img.style.transform='none';}else{img.style.left=gardenBackground.x+'px';img.style.top=gardenBackground.y+'px';img.style.width='';img.style.height='';img.style.transform='scale('+gardenBackground.scale+')';}layer.appendChild(img);
    }
    const op=document.getElementById('gardenBackgroundOpacity');if(op)op.value=String(Math.round(gardenBackground.opacity*100));
    const sc=document.getElementById('gardenBackgroundScale');if(sc)sc.value=String(Math.round(gardenBackground.scale*100));
    const info=document.getElementById('gardenBackgroundInfo');if(info)info.textContent=gardenBackground.dataUrl?'Hintergrund aktiv · '+Math.round(gardenBackground.opacity*100)+' % · '+Math.round(gardenBackground.scale*100)+' %':'Kein Hintergrundbild';
    const btn=document.getElementById('gardenBackgroundMoveButton');if(btn)btn.textContent=gardenBackgroundMoveMode?'Verschieben beenden':'Hintergrund verschieben';
    canvas.classList.toggle('gardenBackgroundMoveMode',gardenBackgroundMoveMode);
}
function gardenLoadBackgroundFile(event){
    const file=event.target.files?.[0];if(!file)return;
    if(file.size>1500*1024){setGardenStatus('Bild zu groß. Bitte maximal 1,5 MB verwenden.','errmsg');event.target.value='';return;}
    const reader=new FileReader();
    reader.onload=()=>{gardenBackground={dataUrl:String(reader.result||''),opacity:.45,scale:1,x:0,y:0};if(gardenSaveBackgroundLocal()){renderGardenBackground();setGardenStatus('Hintergrundbild im Browser gespeichert','okmsg');}};
    reader.onerror=()=>setGardenStatus('Hintergrundbild konnte nicht gelesen werden','errmsg');reader.readAsDataURL(file);
}
function gardenSetBackgroundOpacity(value){gardenBackground.opacity=Math.max(.1,Math.min(1,Number(value)/100));gardenSaveBackgroundLocal();renderGardenBackground();}
function gardenSetBackgroundScale(value){gardenBackground.fitToPlan=false;gardenBackground.scale=Math.max(.25,Math.min(3,Number(value)/100));gardenSaveBackgroundLocal();renderGardenBackground();}
function gardenFitBackgroundToPlan(){if(!gardenBackground.dataUrl){setGardenStatus('Bitte zuerst ein Hintergrundbild laden','errmsg');return;}gardenBackgroundMoveMode=false;gardenBackgroundDrag=null;gardenBackground.fitToPlan=true;gardenBackground.x=0;gardenBackground.y=0;gardenBackground.scale=1;gardenSaveBackgroundLocal();renderGardenBackground();renderGardenZoneStatistics();setGardenStatus('Hintergrund entspricht jetzt exakt der Planfläche','okmsg');}
function gardenToggleBackgroundMove(){if(!gardenBackground.dataUrl){setGardenStatus('Bitte zuerst ein Hintergrundbild laden','errmsg');return;}gardenBackgroundMoveMode=!gardenBackgroundMoveMode;if(gardenBackgroundMoveMode)gardenBackground.fitToPlan=false;gardenBackgroundDrag=null;renderGardenBackground();}
function gardenResetBackgroundTransform(){gardenBackground.x=0;gardenBackground.y=0;gardenBackground.fitToPlan=false;gardenBackground.scale=1;gardenSaveBackgroundLocal();renderGardenBackground();}
function gardenRemoveBackground(){if(!gardenBackground.dataUrl)return;if(!confirm('Hintergrundbild wirklich entfernen?'))return;gardenBackground={dataUrl:'',opacity:.45,scale:1,x:0,y:0,fitToPlan:false};gardenBackgroundMoveMode=false;gardenBackgroundDrag=null;try{localStorage.removeItem(gardenBackgroundStorageKey());}catch(e){};const f=document.getElementById('gardenBackgroundFile');if(f)f.value='';renderGardenBackground();setGardenStatus('Hintergrundbild entfernt','okmsg');}
function gardenBackgroundPointerDown(event){
    if(!gardenBackgroundMoveMode||!gardenBackground.dataUrl)return false;
    gardenBackgroundDrag={pointerId:event.pointerId,startX:event.clientX,startY:event.clientY,x:gardenBackground.x,y:gardenBackground.y};
    event.currentTarget.setPointerCapture?.(event.pointerId);event.preventDefault();event.stopPropagation();return true;
}
function gardenBackgroundPointerMove(event){if(!gardenBackgroundDrag||event.pointerId!==gardenBackgroundDrag.pointerId)return;gardenBackground.x=gardenBackgroundDrag.x+event.clientX-gardenBackgroundDrag.startX;gardenBackground.y=gardenBackgroundDrag.y+event.clientY-gardenBackgroundDrag.startY;renderGardenBackground();event.preventDefault();}
function gardenBackgroundPointerUp(event){if(!gardenBackgroundDrag||event.pointerId!==gardenBackgroundDrag.pointerId)return;gardenBackgroundDrag=null;gardenSaveBackgroundLocal();renderGardenBackground();event.preventDefault();}

function gardenMapStorageKey(){return 'gardenflowGardenMapV1';}

async function loadGardenMap(){
    let loadedFromEsp=false;
    try{
        const data=await api('/api/garden');
        gardenZones=Array.isArray(data.zones)?data.zones.map(zone=>({
            id:Number(zone.id||0),
            name:String(zone.name||'Zone'),
            profileId:Number(zone.profile||0),
            valve:Number(zone.valve||0),
            programIndex:Number(zone.program??-1),
            shape:String(zone.shape||'polygon'),
            points:Array.isArray(zone.points)
                ? zone.points.map(p=>[
                    Number(p?.[0]??0),
                    Number(p?.[1]??0)
                  ])
                : [],
            color:String(zone.color||'#2d7645'),
            x:Number(zone.x??10),
            y:Number(zone.y??10),
            w:Number(zone.width??24),
            h:Number(zone.height??18)
        })):[];
        loadedFromEsp=true;
        try{localStorage.setItem(gardenMapStorageKey(),JSON.stringify(gardenZones));}catch(e){}
    }catch(error){
        try{
            const raw=localStorage.getItem(gardenMapStorageKey());
            gardenZones=raw?JSON.parse(raw):[];
            if(!Array.isArray(gardenZones))gardenZones=[];
            setGardenStatus('ESP-Gartenkarte nicht erreichbar – Browser-Sicherung geladen','errmsg');
        }catch(e){gardenZones=[];}
    }
    normalizeGardenZones();
    renderGardenMap();
    if(loadedFromEsp)setGardenStatus(`Gartenkarte vom ESP geladen · ${gardenZones.length} Zone(n)`,'okmsg');
}

function gardenMetricStorageKey(){return 'gardenflowMetricV1';}
function gardenSaveMetricLocal(){
    try{localStorage.setItem(gardenMetricStorageKey(),JSON.stringify(gardenMetric));return true;}
    catch(error){setGardenStatus('Maßstab konnte nicht gespeichert werden: '+error.message,'errmsg');return false;}
}
function gardenLoadMetricLocal(){
    try{
        const raw=localStorage.getItem(gardenMetricStorageKey());
        if(raw){
            const v=JSON.parse(raw);
            gardenMetric={
                widthM:Math.max(0,Number(v.widthM||0)),
                heightM:Math.max(0,Number(v.heightM||0))
            };
        }
    }catch(e){}
    gardenRenderMetricControls();
}
function gardenRenderMetricControls(){
    const w=document.getElementById('gardenMapWidthM');
    const h=document.getElementById('gardenMapHeightM');
    const info=document.getElementById('gardenMetricInfo');
    if(w)w.value=gardenMetric.widthM>0?String(gardenMetric.widthM):'';
    if(h)h.value=gardenMetric.heightM>0?String(gardenMetric.heightM):'';
    if(info){
        info.textContent=(gardenMetric.widthM>0&&gardenMetric.heightM>0)
            ? `Karte ${gardenMetric.widthM.toFixed(1).replace('.',',')} × ${gardenMetric.heightM.toFixed(1).replace('.',',')} m`
            : 'Noch kein Maßstab gesetzt';
    }
}
function gardenApplyPlanAspectRatio(){const canvas=document.getElementById('gardenCanvas');if(!canvas)return;if(gardenMetric.widthM>0&&gardenMetric.heightM>0)canvas.style.aspectRatio=String(gardenMetric.widthM)+' / '+String(gardenMetric.heightM);else canvas.style.aspectRatio='';}
function gardenSetMapDimensions(){
    const w=Number(document.getElementById('gardenMapWidthM')?.value||0);
    const h=Number(document.getElementById('gardenMapHeightM')?.value||0);
    if(!(w>0&&h>0)){
        setGardenStatus('Bitte Kartenbreite und Kartenhöhe in Metern eingeben','errmsg');
        return;
    }
    gardenMetric={widthM:w,heightM:h};
    gardenSaveMetricLocal();
    gardenRenderMetricControls();
    renderGardenZoneStatistics();
    setGardenStatus('Maßstab gespeichert','okmsg');
gardenApplyPlanAspectRatio();}
function gardenZoneAreaM2(zone){
    if(!zone || !(gardenMetric.widthM>0) || !(gardenMetric.heightM>0))return 0;
    gardenEnsurePolygon(zone);
    if(zone.points.length<3)return 0;
    let sum=0;
    for(let i=0;i<zone.points.length;i++){
        const a=zone.points[i];
        const b=zone.points[(i+1)%zone.points.length];
        const ax=(Number(a[0])/100)*gardenMetric.widthM;
        const ay=(Number(a[1])/100)*gardenMetric.heightM;
        const bx=(Number(b[0])/100)*gardenMetric.widthM;
        const by=(Number(b[1])/100)*gardenMetric.heightM;
        sum+=ax*by-bx*ay;
    }
    return Math.abs(sum)/2;
}

function gardenEnsurePolygon(zone){
    if(Array.isArray(zone.points) && zone.points.length>=3){
        return;
    }

    const x=Number(zone.x||0);
    const y=Number(zone.y||0);
    const w=Number(zone.w||24);
    const h=Number(zone.h||18);

    zone.points=[
        [x,y],
        [x+w,y],
        [x+w,y+h],
        [x,y+h]
    ];
    zone.shape='polygon';
}

function gardenUpdateBoundsFromPoints(zone){
    gardenEnsurePolygon(zone);

    const xs=zone.points.map(p=>Number(p[0]));
    const ys=zone.points.map(p=>Number(p[1]));

    zone.x=Math.min(...xs);
    zone.y=Math.min(...ys);
    zone.w=Math.max(8,Math.max(...xs)-zone.x);
    zone.h=Math.max(8,Math.max(...ys)-zone.y);
}

function gardenSyncPolygonFromRect(zone){
    const x=Number(zone.x||0);
    const y=Number(zone.y||0);
    const w=Number(zone.w||8);
    const h=Number(zone.h||8);

    zone.points=[
        [x,y],
        [x+w,y],
        [x+w,y+h],
        [x,y+h]
    ];
    zone.shape='polygon';
}

function gardenClonePoints(zone){
    gardenEnsurePolygon(zone);
    return zone.points.map(p=>[
        Number(p[0]),
        Number(p[1])
    ]);
}

function gardenTranslatePolygon(zone,originalPoints,dx,dy){
    zone.points=originalPoints.map(point=>[
        Math.max(0,Math.min(100,Number(point[0])+dx)),
        Math.max(0,Math.min(100,Number(point[1])+dy))
    ]);
    gardenUpdateBoundsFromPoints(zone);
}

function gardenScalePolygon(zone,originalPoints,oldBounds,newBounds){
    const oldW=Math.max(0.001,Number(oldBounds.w));
    const oldH=Math.max(0.001,Number(oldBounds.h));
    const sx=Math.max(0.001,Number(newBounds.w))/oldW;
    const sy=Math.max(0.001,Number(newBounds.h))/oldH;

    zone.points=originalPoints.map(point=>[
        Math.max(
            0,
            Math.min(
                100,
                Number(oldBounds.x)+
                (Number(point[0])-Number(oldBounds.x))*sx
            )
        ),
        Math.max(
            0,
            Math.min(
                100,
                Number(oldBounds.y)+
                (Number(point[1])-Number(oldBounds.y))*sy
            )
        )
    ]);

    gardenUpdateBoundsFromPoints(zone);
}

function toggleGardenPolygonEdit(){
    if(selectedGardenZoneId===null){
        setGardenStatus('Bitte zuerst eine Zone auswählen','errmsg');
        return;
    }

    gardenPolygonEditMode=!gardenPolygonEditMode;
    gardenSelectedVertex=-1;
    renderGardenMap();
    renderGardenInspector();

    setGardenStatus(
        gardenPolygonEditMode
            ? 'Polygon-Bearbeitung aktiv · Eckpunkte ziehen'
            : 'Polygon-Bearbeitung beendet',
        'okmsg'
    );
}

function gardenAddVertex(zoneId,edgeIndex,event){
    if(event){
        event.preventDefault();
        event.stopPropagation();
    }

    const zone=gardenZones.find(z=>z.id===Number(zoneId));
    if(!zone)return;

    gardenEnsurePolygon(zone);

    if(zone.points.length>=16){
        setGardenStatus('Maximal 16 Eckpunkte pro Zone','errmsg');
        return;
    }

    const a=zone.points[edgeIndex];
    const b=zone.points[(edgeIndex+1)%zone.points.length];

    zone.points.splice(edgeIndex+1,0,[
        (Number(a[0])+Number(b[0]))/2,
        (Number(a[1])+Number(b[1]))/2
    ]);

    gardenSelectedVertex=edgeIndex+1;
    gardenUpdateBoundsFromPoints(zone);
    renderGardenMap();
    void saveGardenMap();

    setGardenStatus('Eckpunkt hinzugefügt','okmsg');
}

function gardenDeleteVertex(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);

    if(!zone || gardenSelectedVertex<0){
        setGardenStatus('Bitte zuerst einen Eckpunkt auswählen','errmsg');
        return;
    }

    gardenEnsurePolygon(zone);

    if(zone.points.length<=3){
        setGardenStatus('Mindestens 3 Eckpunkte erforderlich','errmsg');
        return;
    }

    zone.points.splice(gardenSelectedVertex,1);
    gardenSelectedVertex=-1;
    gardenUpdateBoundsFromPoints(zone);
    renderGardenMap();
    void saveGardenMap();

    setGardenStatus('Eckpunkt gelöscht','okmsg');
}

function gardenCoord(value){
    const n=Number(value);
    if(!Number.isFinite(n))return 0;
    return Math.round(
        Math.max(0,Math.min(100,n))*1000
    )/1000;
}

async function gardenRoundSelectedVertex(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);

    if(!zone || gardenSelectedVertex<0){
        setGardenStatus('Bitte zuerst einen Eckpunkt auswählen','errmsg');
        return;
    }

    gardenEnsurePolygon(zone);

    const count=zone.points.length;

    if(count+4>16){
        setGardenStatus('Für diese Rundung sind zu viele Eckpunkte vorhanden','errmsg');
        return;
    }

    const index=gardenSelectedVertex;
    const prev=zone.points[(index-1+count)%count];
    const corner=zone.points[index];
    const next=zone.points[(index+1)%count];

    const strength=Math.max(
        0.05,
        Math.min(
            0.45,
            Number(document.getElementById('gardenRoundStrength')?.value||0.25)
        )
    );

    const start=[
        Number(corner[0])+(Number(prev[0])-Number(corner[0]))*strength,
        Number(corner[1])+(Number(prev[1])-Number(corner[1]))*strength
    ];

    const end=[
        Number(corner[0])+(Number(next[0])-Number(corner[0]))*strength,
        Number(corner[1])+(Number(next[1])-Number(corner[1]))*strength
    ];

    const curve=[];

    for(let step=0;step<5;step++){
        const t=step/4;
        const u=1-t;

        curve.push([
            gardenCoord(
                u*u*start[0]+
                2*u*t*Number(corner[0])+
                t*t*end[0]
            ),
            gardenCoord(
                u*u*start[1]+
                2*u*t*Number(corner[1])+
                t*t*end[1]
            )
        ]);
    }

    zone.points.splice(index,1,...curve);
    gardenSelectedVertex=index+2;
    gardenUpdateBoundsFromPoints(zone);
    renderGardenMap();

    const ok=await saveGardenMap();

    if(ok){
        setGardenStatus(
            `Ecke abgerundet · ${zone.points.length} Eckpunkte · gespeichert`,
            'okmsg'
        );
    }
}

function renderGardenEdgePluses(){
    const canvas=document.getElementById('gardenCanvas');
    if(!canvas)return;

    canvas.querySelectorAll('.gardenEdgePlus').forEach(e=>e.remove());

    if(!gardenPolygonEditMode || selectedGardenZoneId===null)return;

    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone)return;

    gardenEnsurePolygon(zone);

    zone.points.forEach((a,index)=>{
        const b=zone.points[(index+1)%zone.points.length];

        const plus=document.createElement('button');
        plus.type='button';
        plus.className='gardenEdgePlus';
        plus.textContent='+';
        plus.title='Eckpunkt hinzufügen';
        plus.style.left=`${(Number(a[0])+Number(b[0]))/2}%`;
        plus.style.top=`${(Number(a[1])+Number(b[1]))/2}%`;

        plus.addEventListener('pointerdown',event=>{
            event.preventDefault();
            event.stopPropagation();
        });

        plus.addEventListener('click',event=>{
            gardenAddVertex(zone.id,index,event);
        });

        canvas.appendChild(plus);
    });
}

function renderGardenVertexHandles(){
    const canvas=document.getElementById('gardenCanvas');
    if(!canvas)return;

    canvas.querySelectorAll('.gardenVertexHandle').forEach(handle=>handle.remove());
    canvas.classList.toggle('gardenPolygonEditing',gardenPolygonEditMode);

    if(!gardenPolygonEditMode || selectedGardenZoneId===null)return;

    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone)return;

    gardenEnsurePolygon(zone);

    zone.points.forEach((point,index)=>{
        const handle=document.createElement('div');
        handle.className='gardenVertexHandle'+(index===gardenSelectedVertex?' activeVertex':'');
        handle.dataset.zoneId=String(zone.id);
        handle.dataset.pointIndex=String(index);
        handle.style.left=`${Number(point[0])}%`;
        handle.style.top=`${Number(point[1])}%`;
        handle.title=`Eckpunkt ${index+1}`;
        handle.addEventListener('pointerdown',gardenVertexPointerDown);
        handle.addEventListener('click',event=>{
            event.preventDefault();
            event.stopPropagation();
            gardenSelectedVertex=index;
            renderGardenMap();
        });
        canvas.appendChild(handle);
    });
}

function gardenVertexPointerDown(event){
    if(!gardenPolygonEditMode)return;

    const handle=event.currentTarget;
    const zoneId=Number(handle.dataset.zoneId);
    const pointIndex=Number(handle.dataset.pointIndex);
    const zone=gardenZones.find(z=>z.id===zoneId);
    const canvas=document.getElementById('gardenCanvas');

    if(!zone || !canvas || !zone.points?.[pointIndex])return;

    gardenSelectedVertex=pointIndex;
    const rect=canvas.getBoundingClientRect();

    gardenVertexInteraction={
        zoneId,
        pointIndex,
        canvasRect:rect
    };

    handle.classList.add('dragging');
    handle.setPointerCapture?.(event.pointerId);

    window.addEventListener(
        'pointermove',
        gardenVertexPointerMove,
        {passive:false}
    );

    window.addEventListener(
        'pointerup',
        gardenVertexPointerUp,
        {once:true}
    );

    event.preventDefault();
    event.stopPropagation();
}

function gardenVertexPointerMove(event){
    if(!gardenVertexInteraction)return;

    event.preventDefault();

    const state=gardenVertexInteraction;
    const zone=gardenZones.find(z=>z.id===state.zoneId);
    if(!zone)return;

    const rect=state.canvasRect;
    const x=((event.clientX-rect.left)/rect.width)*100;
    const y=((event.clientY-rect.top)/rect.height)*100;

    zone.points[state.pointIndex]=[
        Math.max(0,Math.min(100,x)),
        Math.max(0,Math.min(100,y))
    ];

    gardenUpdateBoundsFromPoints(zone);
    renderGardenMap();
}

function gardenVertexPointerUp(){
    if(!gardenVertexInteraction)return;

    gardenVertexInteraction=null;
    window.removeEventListener('pointermove',gardenVertexPointerMove);
    void saveGardenMap();
    renderGardenMap();
}

function gardenClipPath(zone){
    gardenEnsurePolygon(zone);
    gardenUpdateBoundsFromPoints(zone);

    const w=Math.max(0.001,Number(zone.w||1));
    const h=Math.max(0.001,Number(zone.h||1));

    return 'polygon('+
        zone.points.map(point=>{
            const px=((Number(point[0])-zone.x)/w)*100;
            const py=((Number(point[1])-zone.y)/h)*100;
            return `${px}% ${py}%`;
        }).join(',')+
        ')';
}

function normalizeGardenZones(){
    gardenZones=gardenZones.map((zone,index)=>({
        id:Number(zone.id||Date.now()+index),
        name:String(zone.name||`Zone ${index+1}`),
        profileId:Number(zone.profileId||0),
        valve:Number(zone.valve||0),
        programIndex:Number(zone.programIndex??-1),
        shape:'polygon',
        points:Array.isArray(zone.points)
            ? zone.points
                .filter(p=>Array.isArray(p)&&p.length>=2)
                .slice(0,16)
                .map(p=>[
                    Math.max(0,Math.min(100,Number(p[0]||0))),
                    Math.max(0,Math.min(100,Number(p[1]||0)))
                ])
            : [],
        color:String(zone.color||'#2d7645'),
        x:Math.max(0,Math.min(90,Number(zone.x??10))),
        y:Math.max(0,Math.min(85,Number(zone.y??10))),
        w:Math.max(8,Math.min(80,Number(zone.w??24))),
        h:Math.max(8,Math.min(80,Number(zone.h??18)))
    }));

    gardenZones.forEach(zone=>{
        gardenEnsurePolygon(zone);
        gardenUpdateBoundsFromPoints(zone);
    });
}

async function saveGardenMap(){
    const payload={
        version:1,
        zones:gardenZones.map(zone=>({
            id:Number(zone.id||0),
            name:String(zone.name||'Zone'),
            profile:Number(zone.profileId||0),
            valve:Number(zone.valve||0),
            program:Number(zone.programIndex??-1),
            shape:'polygon',
            x:Number(zone.x||0),
            y:Number(zone.y||0),
            width:Number(zone.w||8),
            height:Number(zone.h||8),
            points:(zone.points||[]).map(p=>[
                gardenCoord(p[0]),
                gardenCoord(p[1])
            ]),
            color:String(zone.color||'#2d7645')
        }))
    };
    let espSaved=false;
    let espError='';
    try{
        await api('/api/garden/save',{
            method:'POST',
            headers:{'Content-Type':'application/json'},
            body:JSON.stringify(payload)
        });
        espSaved=true;
    }catch(error){
        espError=error.message||String(error);
    }

    let browserSaved=false;
    try{
        localStorage.setItem(gardenMapStorageKey(),JSON.stringify(gardenZones));
        browserSaved=true;
    }catch(e){}

    if(espSaved){
        setGardenStatus(`Gartenkarte gespeichert · ${gardenZones.length} Zone(n) · ESP + Browser-Sicherung`,'okmsg');
        return true;
    }
    if(browserSaved){
        setGardenStatus(`ESP-Speicherung fehlgeschlagen (${espError}) · Browser-Sicherung gespeichert`,'errmsg');
        return false;
    }
    setGardenStatus(`Speichern fehlgeschlagen: ${espError||'unbekannter Fehler'}`,'errmsg');
    return false;
}

async function resetGardenMap(){
    if(!confirm('Gartenkarte wirklich zurücksetzen?'))return;
    let espOk=false;
    try{
        await api('/api/garden/reset',{method:'POST'});
        espOk=true;
    }catch(e){}
    gardenZones=[];selectedGardenZoneId=null;
    try{localStorage.removeItem(gardenMapStorageKey());}catch(e){}
    renderGardenMap();
    setGardenStatus(
        espOk?'Gartenkarte im ESP und Browser zurückgesetzt':'Browser-Gartenkarte zurückgesetzt · ESP nicht erreichbar',
        espOk?'okmsg':'errmsg'
    );
}

function addGardenZone(){
    const id=Date.now(),index=gardenZones.length;
    gardenZones.push({id,name:`Zone ${index+1}`,profileId:0,valve:index%2,programIndex:-1,shape:'polygon',points:[],color:index%2?'#315f91':'#2d7645',x:8+(index%4)*8,y:8+(index%5)*7,w:24,h:18});
    gardenSyncPolygonFromRect(gardenZones[gardenZones.length-1]);
    selectedGardenZoneId=id;renderGardenMap();void saveGardenMap();
}

function setGardenStatus(text,type=''){
    const state=document.getElementById('gardenStatus');
    if(!state)return;
    state.textContent=text||'';state.className='gardenStatus'+(type?' '+type:'');
}

function gardenProfileName(profileId){
    const profile=profileCache.find(p=>Number(p.id)===Number(profileId));
    return profile?profile.name:'Allgemein';
}

function renderGardenProfileSelector(){
    const select=document.getElementById('gardenZoneProfile');
    if(!select)return;
    const selected=select.value;
    select.innerHTML=(profileCache||[]).map(p=>`<option value="${p.id}">${esc(p.name)}</option>`).join('')||'<option value="0">Allgemein</option>';
    if([...select.options].some(o=>o.value===selected))select.value=selected;
}

function gardenValveIsOpen(valveIndex){
    if(!lastStatus || !Array.isArray(lastStatus.valves))return false;
    const valve=lastStatus.valves[Number(valveIndex)];
    return !!(valve && valve.open);
}

function updateGardenLiveState(){
    const canvas=document.getElementById('gardenCanvas');
    if(!canvas)return;
    gardenZones.forEach(zone=>{
        const element=canvas.querySelector(`[data-zone-id="${zone.id}"]`);
        if(!element)return;
        const active=gardenValveIsOpen(zone.valve);
        element.classList.toggle('watering',active);
        let badge=element.querySelector('.gardenWaterBadge');
        if(active && !badge){
            badge=document.createElement('div');
            badge.className='gardenWaterBadge';
            badge.textContent='💧 AKTIV';
            element.appendChild(badge);
        }else if(!active && badge){
            badge.remove();
        }
        const meta=element.querySelector('.gardenZoneMeta');
        if(meta)meta.textContent=`${gardenProfileName(zone.profileId)} · Ventil ${Number(zone.valve)+1}${gardenProgramLabel(zone.programIndex)?' · '+gardenProgramLabel(zone.programIndex):''}${active?' · Bewässerung läuft':''}`;
    });
}

function gardenProgramLabel(programIndex){
    const program=programCache.find(p=>Number(p.index)===Number(programIndex));
    if(!program)return '';
    const hh=String(program.hour).padStart(2,'0');
    const mm=String(program.minute).padStart(2,'0');
    return `Programm ${program.id} · ${hh}:${mm} · ${program.durationMinutes} min`;
}

function renderGardenProgramSelector(){
    const select=document.getElementById('gardenZoneProgram');
    if(!select)return;

    const current=select.value;

    let html='<option value="-1">Kein Programm</option>';

    programCache.forEach(program=>{
        const hh=String(program.hour).padStart(2,'0');
        const mm=String(program.minute).padStart(2,'0');
        const profile=program.profileName||'Allgemein';
        html+=`<option value="${program.index}">P${program.id} · ${hh}:${mm} · V${Number(program.valve)+1} · ${esc(profile)}</option>`;
    });

    select.innerHTML=html;

    if([...select.options].some(o=>o.value===current)){
        select.value=current;
    }
}

function openSelectedGardenProgram(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);

    if(!zone || Number(zone.programIndex)<0){
        setGardenStatus('Dieser Zone ist noch kein Programm zugeordnet','errmsg');
        return;
    }

    const program=programCache.find(p=>Number(p.index)===Number(zone.programIndex));

    if(!program){
        setGardenStatus('Das zugeordnete Programm existiert nicht mehr','errmsg');
        return;
    }

    editProgram(program.index);
}

function renderGardenMap(){
    const canvas=document.getElementById('gardenCanvas');
    if(!canvas)return;
    renderGardenBackground();
    canvas.querySelectorAll('.gardenZone').forEach(z=>z.remove());
    const empty=document.getElementById('gardenEmpty');
    if(empty)empty.style.display=gardenZones.length?'none':'flex';
    const count=document.getElementById('gardenZoneCount');
    if(count)count.textContent=String(gardenZones.length);

    gardenZones.forEach(zone=>{
        const element=document.createElement('div');
        element.className='gardenZone'+(zone.id===selectedGardenZoneId?' selected':'')+(gardenValveIsOpen(zone.valve)?' watering':'');
        element.dataset.zoneId=String(zone.id);
        gardenEnsurePolygon(zone);
        gardenUpdateBoundsFromPoints(zone);
        element.style.left=`${zone.x}%`;element.style.top=`${zone.y}%`;element.style.width=`${zone.w}%`;element.style.height=`${zone.h}%`;element.style.background=zone.color+'cc';
        element.style.clipPath=gardenClipPath(zone);
        element.style.webkitClipPath=gardenClipPath(zone);
        const watering=gardenValveIsOpen(zone.valve);
        const programLabel=gardenProgramLabel(zone.programIndex);
        element.innerHTML=`<div class="gardenZoneHeader">${esc(zone.name)}</div><div class="gardenZoneMeta">${esc(gardenProfileName(zone.profileId))} · Ventil ${Number(zone.valve)+1}${programLabel?' · '+esc(programLabel):''}${watering?' · Bewässerung läuft':''}</div>${watering?'<div class="gardenWaterBadge">💧 AKTIV</div>':''}<div class="gardenResizeHandle"></div>`;
        element.addEventListener('pointerdown',gardenZonePointerDown);
        element.addEventListener('click',e=>{e.stopPropagation();selectGardenZone(zone.id);});
        element.addEventListener('dblclick',e=>{
            e.preventDefault();
            e.stopPropagation();
            selectedGardenZoneId=zone.id;
            openSelectedGardenProgram();
        });
        canvas.appendChild(element);
    });
    canvas.onclick=()=>{if(gardenBackgroundMoveMode)return;selectedGardenZoneId=null;gardenPolygonEditMode=false;gardenSelectedVertex=-1;renderGardenMap();renderGardenInspector();};
    if(!canvas.dataset.backgroundHandlers){
        canvas.dataset.backgroundHandlers='1';
        canvas.addEventListener('pointerdown',gardenBackgroundPointerDown,true);
        canvas.addEventListener('pointermove',gardenBackgroundPointerMove,true);
        canvas.addEventListener('pointerup',gardenBackgroundPointerUp,true);
        canvas.addEventListener('pointercancel',gardenBackgroundPointerUp,true);
    }
    renderGardenInspector();
    renderGardenEdgePluses();
    renderGardenVertexHandles();
}

function selectGardenZone(id){
    const nextId=Number(id);
    if(selectedGardenZoneId!==nextId){
        gardenPolygonEditMode=false;
        gardenSelectedVertex=-1;
    }
    selectedGardenZoneId=nextId;
    renderGardenMap();
    renderGardenInspector();
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(zone)setGardenStatus(`Zone „${zone.name}“ ausgewählt – Einstellungen oberhalb der Karte`,'okmsg');
}

function gardenLinkedProgram(zone){
    if(!zone || Number(zone.programIndex)<0)return null;
    return programCache.find(p=>Number(p.index)===Number(zone.programIndex))||null;
}

function gardenZoneHistoryEntries(zone){
    if(!zone)return [];

    const program=gardenLinkedProgram(zone);

    return (historyCache||[]).filter(entry=>{
        const linkedProgram=
            program &&
            Number(entry.programId)===Number(program.id);

        const manualSameValve=
            Number(entry.programId)===0 &&
            Number(entry.valve)===Number(zone.valve);

        return linkedProgram || manualSameValve;
    });
}

function gardenZoneIsRunning(zone){
    if(!zone || !lastStatus)return false;

    const program=gardenLinkedProgram(zone);

    const programRun=
        !!program &&
        lastStatus.running &&
        Number(lastStatus.programId)===Number(program.id);

    const manualRun=
        !!lastStatus.manualRun &&
        Number(lastStatus.manualValve)===Number(zone.valve);

    return programRun || manualRun;
}

function gardenCurrentRunText(zone){
    if(!gardenZoneIsRunning(zone))return '--';

    if(lastStatus.manualRun &&
       Number(lastStatus.manualValve)===Number(zone.valve)){
        const elapsed=Math.max(
            0,
            Number(lastStatus.currentRunSeconds||0)
        );

        const minutes=Math.floor(elapsed/60);
        const seconds=elapsed%60;

        return `MANUELL · ${minutes}:${String(seconds).padStart(2,'0')}`;
    }

    const remaining=Math.max(
        0,
        Number(lastStatus.remaining||0)
    );

    const minutes=Math.floor(remaining/60);
    const seconds=remaining%60;

    return `LÄUFT · Rest ${minutes}:${String(seconds).padStart(2,'0')}`;
}

function gardenRunStartTimestamp(stopEntry,entries){
    if(!stopEntry)return 0;

    const stopTime=Number(stopEntry.timestamp||0);
    const actualSeconds=Math.max(0,Number(stopEntry.actualSeconds||0));

    if(stopTime>0 && actualSeconds>0){
        return stopTime-actualSeconds;
    }

    const candidates=(entries||[])
        .filter(entry=>
            entry.event==='start' &&
            Number(entry.programId)===Number(stopEntry.programId) &&
            Number(entry.valve)===Number(stopEntry.valve) &&
            Number(entry.timestamp||0)<=stopTime
        )
        .sort((a,b)=>Number(b.timestamp||0)-Number(a.timestamp||0));

    return candidates.length
        ? Number(candidates[0].timestamp||0)
        : stopTime;
}

function gardenTargetMmStorageKey(zoneId){
    return `gardenflowTargetMm_${zoneId}`;
}

function gardenGetTargetMm(zone){
    if(!zone)return 8.0;
    try{
        const raw=localStorage.getItem(gardenTargetMmStorageKey(zone.id));
        if(raw!==null){
            const v=Number(raw);
            if(Number.isFinite(v))return Math.max(0,Math.min(50,v));
        }
    }catch(e){}
    return 8.0;
}

function gardenSaveTargetMm(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    const input=document.getElementById('gardenTargetMm');
    if(!zone || !input)return;

    const value=Math.max(0,Math.min(50,Number(input.value)||0));
    input.value=value.toFixed(1);

    try{
        localStorage.setItem(gardenTargetMmStorageKey(zone.id),String(value));
    }catch(e){}

    renderGardenZoneStatistics();
}

function gardenZoneFlowRate(zone){
    if(!zone || !lastStatus)return 0;
    const valve=Number(zone.valve||0);
    const flow=valve===0
        ? Number(lastStatus.waterFlow1||0)
        : Number(lastStatus.waterFlow2||0);
    return Number.isFinite(flow)&&flow>0?flow:0;
}

function gardenRecommendedMinutes(zone){
    const area=gardenZoneAreaM2(zone);
    const mm=gardenGetTargetMm(zone);
    const liters=area*mm;
    const flow=gardenZoneFlowRate(zone);
    return flow>0?liters/flow:0;
}

async function gardenApplyRecommendedRuntime(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone){
        setGardenStatus('Bitte zuerst eine Zone auswählen','errmsg');
        return;
    }

    const program=gardenLinkedProgram(zone);
    if(!program){
        setGardenStatus('Dieser Zone ist kein Programm zugeordnet','errmsg');
        return;
    }

    const recommended=gardenRecommendedMinutes(zone);
    if(!(recommended>0)){
        setGardenStatus('Keine gültige Laufzeit berechnet','errmsg');
        return;
    }

    const duration=Math.max(1,Math.min(1440,Math.round(recommended)));

    if(!confirm(
        `Programm ${program.id}: Laufzeit von ${program.durationMinutes} auf ${duration} Minuten ändern?`
    ))return;

    try{
        await api('/api/program/update',{
            method:'POST',
            headers:{'Content-Type':'application/x-www-form-urlencoded'},
            body:new URLSearchParams({
                index:program.index,
                valve:program.valve,
                profile:program.profileId,
                hour:program.hour,
                minute:program.minute,
                duration,
                days:program.weekdays,
                enabled:Number(!!program.enabled)
            })
        });

        await loadPrograms();
        await loadStatus();
        setGardenStatus(
            `Programm ${program.id}: Laufzeit auf ${duration} min übernommen`,
            'okmsg'
        );
    }catch(error){
        setGardenStatus('Laufzeit konnte nicht übernommen werden: '+error.message,'errmsg');
    }
}

function renderGardenZoneStatistics(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone)return;

    const program=gardenLinkedProgram(zone);
    const allEntries=gardenZoneHistoryEntries(zone);
    const completed=allEntries.filter(entry=>entry.event==='stop');

    const historyLiters=completed.reduce(
        (sum,entry)=>sum+Number(entry.liters||0),
        0
    );

    const historyCost=completed.reduce(
        (sum,entry)=>sum+Number(entry.costEuro||0),
        0
    );

    const newestCompleted=completed
        .slice()
        .sort((a,b)=>Number(b.timestamp||0)-Number(a.timestamp||0))[0];

    const running=gardenZoneIsRunning(zone);

    const liveLiters=running
        ? Number(lastStatus.waterCurrentRun||0)
        : 0;

    const liveCost=running
        ? Number(lastStatus.waterCurrentCost||0)
        : 0;

    const totalLiters=historyLiters+liveLiters;
    const areaM2=gardenZoneAreaM2(zone);
    const litersPerM2=areaM2>0?totalLiters/areaM2:0;
    const targetMm=gardenGetTargetMm(zone);
    const requiredLiters=areaM2*targetMm;
    const zoneFlowRate=gardenZoneFlowRate(zone);
    const recommendedMinutes=zoneFlowRate>0?requiredLiters/zoneFlowRate:0;

    const setText=(id,value)=>{
        const el=document.getElementById(id);
        if(el)el.textContent=value;
    };

    const newestStartTimestamp=
        newestCompleted
            ? gardenRunStartTimestamp(
                newestCompleted,
                allEntries
              )
            : 0;

    setText(
        'gardenLastWatering',
        running
            ? (
                lastStatus.manualRun
                    ? 'MANUELL · LÄUFT JETZT'
                    : 'LÄUFT JETZT'
              )
            : (
                newestCompleted
                    ? historyTimestamp(newestStartTimestamp)
                    : '--'
              )
    );

    setText(
        'gardenRunCount',
        running
            ? `${completed.length} · +1 läuft`
            : String(completed.length)
    );

    setText(
        'gardenCurrentRun',
        gardenCurrentRunText(zone)
    );

    setText(
        'gardenArea',
        areaM2>0
            ? areaM2.toFixed(1).replace('.',',')+' m²'
            : '-- m²'
    );

    const targetInput=document.getElementById('gardenTargetMm');
    if(targetInput && document.activeElement!==targetInput){
        targetInput.value=targetMm.toFixed(1);
    }

    setText(
        'gardenRequiredLiters',
        areaM2>0
            ? requiredLiters.toFixed(1).replace('.',',')+' l'
            : '-- l'
    );

    setText(
        'gardenZoneFlowRate',
        zoneFlowRate>0
            ? zoneFlowRate.toFixed(1).replace('.',',')+' l/min'
            : '-- l/min'
    );

    setText(
        'gardenRecommendedRuntime',
        recommendedMinutes>0
            ? recommendedMinutes.toFixed(1).replace('.',',')+' min'
            : '-- min'
    );

    const applyButton=document.getElementById('gardenApplyRuntimeButton');
    if(applyButton){
        applyButton.disabled=
            !gardenLinkedProgram(zone) ||
            !(recommendedMinutes>0);
    }


    setText(
        'gardenLiters',
        totalLiters
            .toFixed(1)
            .replace('.',',')+' l'
    );

    setText(
        'gardenLitersPerM2',
        areaM2>0
            ? litersPerM2.toFixed(2).replace('.',',')+' l/m²'
            : '-- l/m²'
    );

    setText(
        'gardenCost',
        (historyCost+liveCost)
            .toFixed(2)
            .replace('.',',')+' €'
    );

    const info=document.getElementById('gardenZoneHistory');
    if(!info)return;

    if(!program){
        info.textContent='Für Historie bitte ein Programm zuordnen.';
        return;
    }

    if(!allEntries.length){
        info.textContent=`${gardenProgramLabel(zone.programIndex)} · Noch keine passenden Historieneinträge in den geladenen Daten.`;
        return;
    }

    const skipped=allEntries.filter(entry=>entry.event==='skipped').length;
    const started=allEntries.filter(entry=>entry.event==='start').length;

    if(gardenZoneIsRunning(zone)){
        const source=
            lastStatus.manualRun
                ? `Manuell · Ventil ${Number(zone.valve)+1}`
                : gardenProgramLabel(zone.programIndex);

        info.textContent=
            `${source} · `+
            `LÄUFT JETZT · `+
            `${Number(lastStatus.waterCurrentRun||0).toFixed(1).replace('.',',')} l · `+
            `${Number(lastStatus.waterCurrentCost||0).toFixed(2).replace('.',',')} €`;
    }else{
        info.textContent=
            `${gardenProgramLabel(zone.programIndex)} · `+
            `${started} Start(s), ${completed.length} Abschluss/Abbruch, `+
            `${skipped} übersprungen`;
    }
}

function renderGardenInspector(){
    const inspector=document.getElementById('gardenInspector');
    if(!inspector)return;
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    inspector.classList.toggle('open',!!zone);
    if(!zone)return;
    renderGardenProfileSelector();
    renderGardenProgramSelector();
    document.getElementById('gardenInspectorTitle').textContent=zone.name;
    document.getElementById('gardenZoneName').value=zone.name;
    document.getElementById('gardenZoneProfile').value=String(zone.profileId);
    document.getElementById('gardenZoneValve').value=String(zone.valve);
    document.getElementById('gardenZoneProgram').value=String(zone.programIndex??-1);
    document.getElementById('gardenZoneColor').value=zone.color;

    const polygonButton=document.getElementById('gardenPolygonEditButton');
    if(polygonButton){
        polygonButton.textContent=
            gardenPolygonEditMode
                ? 'Polygon fertig'
                : 'Polygon bearbeiten';
        polygonButton.className=
            gardenPolygonEditMode
                ? ''
                : 'secondary';
    }

    const polygonInfo=document.getElementById('gardenPolygonInfo');
    if(polygonInfo){
        polygonInfo.textContent=
            gardenPolygonEditMode
                ? `${zone.points?.length||0} Eckpunkte · + Punkt · gelben Punkt abrunden`
                : `${zone.points?.length||0} Eckpunkte`;
    }

    const deleteButton=document.getElementById('gardenDeleteVertexButton');
    if(deleteButton){
        deleteButton.disabled=
            !gardenPolygonEditMode ||
            gardenSelectedVertex<0 ||
            (zone.points?.length||0)<=3;
    }

    const roundButton=document.getElementById('gardenRoundVertexButton');
    if(roundButton){
        roundButton.disabled=
            !gardenPolygonEditMode ||
            gardenSelectedVertex<0 ||
            (zone.points?.length||0)+4>16;
    }

    renderGardenZoneStatistics();
}

function updateSelectedGardenZoneFromInspector(showConfirmation=false){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone)return;
    zone.name=document.getElementById('gardenZoneName').value||'Zone';
    zone.profileId=Number(document.getElementById('gardenZoneProfile').value||0);
    zone.valve=Number(document.getElementById('gardenZoneValve').value||0);
    zone.programIndex=Number(document.getElementById('gardenZoneProgram').value??-1);
    zone.color=document.getElementById('gardenZoneColor').value||'#2d7645';

    const canvas=document.getElementById('gardenCanvas');
    const element=canvas?.querySelector(`[data-zone-id="${zone.id}"]`);
    if(element){
        element.style.background=zone.color+'cc';
        const header=element.querySelector('.gardenZoneHeader');
        const meta=element.querySelector('.gardenZoneMeta');
        if(header)header.textContent=zone.name;
        if(meta)meta.textContent=`${gardenProfileName(zone.profileId)} · Ventil ${Number(zone.valve)+1}${gardenProgramLabel(zone.programIndex)?' · '+gardenProgramLabel(zone.programIndex):''}${gardenValveIsOpen(zone.valve)?' · Bewässerung läuft':''}`;
    }

    document.getElementById('gardenInspectorTitle').textContent=zone.name;
    renderGardenZoneStatistics();
    void saveGardenMap();
    if(showConfirmation)setGardenStatus(`Zone „${zone.name}“ aktualisiert`,'okmsg');
}

async function saveSelectedGardenZone(){
    const zone=gardenZones.find(z=>z.id===selectedGardenZoneId);
    if(!zone){
        setGardenStatus('Bitte zuerst eine Zone auswählen','errmsg');
        return;
    }
    updateSelectedGardenZoneFromInspector(false);
    const ok=await saveGardenMap();
    if(ok)setGardenStatus(`Zone „${zone.name}“ im ESP gespeichert`,'okmsg');
}

function deleteSelectedGardenZone(){
    if(selectedGardenZoneId===null)return;
    if(!confirm('Ausgewählte Zone löschen?'))return;
    gardenZones=gardenZones.filter(z=>z.id!==selectedGardenZoneId);
    selectedGardenZoneId=null;renderGardenMap();void saveGardenMap();
}

function gardenZonePointerDown(event){
    if(gardenPolygonEditMode)return;
    if(event.button!==undefined&&event.button!==0)return;
    const element=event.currentTarget,id=Number(element.dataset.zoneId),zone=gardenZones.find(z=>z.id===id);
    if(!zone)return;
    selectedGardenZoneId=id;renderGardenInspector();
    const canvas=document.getElementById('gardenCanvas'),canvasRect=canvas.getBoundingClientRect();
    const resize=event.target.classList.contains('gardenResizeHandle');
    gardenInteraction={
        id,
        mode:resize?'resize':'move',
        startX:event.clientX,
        startY:event.clientY,
        canvasWidth:canvasRect.width,
        canvasHeight:canvasRect.height,
        x:zone.x,
        y:zone.y,
        w:zone.w,
        h:zone.h,
        points:gardenClonePoints(zone)
    };
    element.setPointerCapture?.(event.pointerId);
    window.addEventListener('pointermove',gardenZonePointerMove,{passive:false});
    window.addEventListener('pointerup',gardenZonePointerUp,{once:true});
    event.preventDefault();event.stopPropagation();
}

function gardenZonePointerMove(event){
    if(!gardenInteraction)return;
    event.preventDefault();
    const state=gardenInteraction,zone=gardenZones.find(z=>z.id===state.id);
    if(!zone)return;
    const dx=(event.clientX-state.startX)/state.canvasWidth*100,dy=(event.clientY-state.startY)/state.canvasHeight*100;
    if(state.mode==='resize'){
        const newW=Math.max(
            8,
            Math.min(
                100-state.x,
                state.w+dx
            )
        );

        const newH=Math.max(
            8,
            Math.min(
                100-state.y,
                state.h+dy
            )
        );

        gardenScalePolygon(
            zone,
            state.points,
            {
                x:state.x,
                y:state.y,
                w:state.w,
                h:state.h
            },
            {
                x:state.x,
                y:state.y,
                w:newW,
                h:newH
            }
        );
    }else{
        const requestedX=Math.max(
            0,
            Math.min(
                100-state.w,
                state.x+dx
            )
        );

        const requestedY=Math.max(
            0,
            Math.min(
                100-state.h,
                state.y+dy
            )
        );

        gardenTranslatePolygon(
            zone,
            state.points,
            requestedX-state.x,
            requestedY-state.y
        );
    }

    renderGardenMap();
}

function gardenZonePointerUp(){
    if(!gardenInteraction)return;
    gardenInteraction=null;
    window.removeEventListener('pointermove',gardenZonePointerMove);
    saveGardenMap();
}

async function loadAll(){
    /*
     * Bewusst nacheinander statt parallel:
     * loadPrograms() aktualisiert programCache und zeichnet danach
     * Zeitplan sowie das nächste Programm neu.
     */
    await loadProfiles();
    await loadPrograms();
    await loadStatus();
    await loadLog();
    await loadHistory();
}bindSettingsForms();loadWeekGridStep();gardenLoadBackgroundLocal();gardenLoadMetricLocal();void loadGardenMap();restoreSelectedPage();loadSetup();loadAll();setInterval(loadStatus,2000);setInterval(loadPrograms,15000);setInterval(loadLog,5000);setInterval(loadHistory,10000);
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
                       WaterManager& waterManager,
                       SeasonManager& seasonManager,
                       BackupManager& backupManager,
                       HistoryManager& historyManager,
                       GardenManager& gardenManager)
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
    seasonManager_ = &seasonManager;
    backupManager_ = &backupManager;
    historyManager_ = &historyManager;
    gardenManager_ = &gardenManager;
    configureRoutes();
    Serial.println("WebManager initialisiert");
}

void WebManager::updateManualValveTracking()
{
    if (valveManager_ == nullptr || runtimeManager_ == nullptr)
    {
        return;
    }

    if (!valveObservationInitialized_)
    {
        for (uint8_t i = 0; i < AppConfig::DISPLAYED_VALVE_COUNT && i < 2; ++i)
        {
            lastObservedValveOpen_[i] = valveManager_->channel(i).assumedOpen;
        }
        valveObservationInitialized_ = true;
        return;
    }

    for (uint8_t i = 0; i < AppConfig::DISPLAYED_VALVE_COUNT && i < 2; ++i)
    {
        const bool openNow = valveManager_->channel(i).assumedOpen;
        const bool openBefore = lastObservedValveOpen_[i];

        if (openNow == openBefore)
        {
            continue;
        }

        lastObservedValveOpen_[i] = openNow;

        if (openNow)
        {
            if (runtimeManager_->isRunning())
            {
                continue;
            }

            manualValveStartedAtMs_[i] = millis();
            manualValveRunActive_[i] = true;

            if (historyManager_ != nullptr && historyManager_->isReady())
            {
                historyManager_->recordStart(0, i, 0, 0, false);
            }
        }
        else if (manualValveRunActive_[i])
        {
            const uint32_t actualSeconds =
                static_cast<uint32_t>(millis() - manualValveStartedAtMs_[i]) / 1000UL;

            if (actualSeconds > 0)
            {
                if (waterManager_ != nullptr)
                {
                    waterManager_->addRuntime(i, actualSeconds);
                }

                if (historyManager_ != nullptr && historyManager_->isReady())
                {
                    historyManager_->recordStop(
                        0, i, 0, 0, actualSeconds, false, false
                    );
                }
            }

            manualValveRunActive_[i] = false;
            manualValveStartedAtMs_[i] = 0;
        }
    }
}

void WebManager::update()
{
    updateManualValveTracking();

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
    server_.on("/api/backup", HTTP_GET, [this]() {
        if (backupManager_ == nullptr)
        {
            sendJson(503, "{\"error\":\"BackupManager nicht bereit\"}");
            return;
        }
        sendJson(200, backupManager_->createBackupJson());
    });
    server_.on("/api/backup/restore", HTTP_POST, [this]() {
        handleBackupRestore();
    });
    server_.on("/api/history", HTTP_GET, [this]() { handleHistory(); });
    server_.on("/api/garden", HTTP_GET, [this]() { handleGarden(); });
    server_.on("/api/garden/save", HTTP_POST, [this]() { handleGardenSave(); });
    server_.on("/api/garden/reset", HTTP_POST, [this]() { handleGardenReset(); });
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
    server_.on("/api/profiles", HTTP_GET, [this]() { handleProfiles(); });
    server_.on("/api/profile/save", HTTP_POST, [this]() { handleProfileSave(); });
    server_.on("/api/profiles/reset", HTTP_POST, [this]() { handleProfilesReset(); });
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
        !advisorEngine_ || !waterManager_ || !seasonManager_)
    {
        sendJson(503, "{\"error\":\"System nicht bereit\"}");
        return;
    }

    char timeText[8] = "--:--";
    char dateText[16] = "--.--.----";
    timeManager_->formatTime(timeText, sizeof(timeText));
    timeManager_->formatDate(dateText, sizeof(dateText));

    String body;
    body.reserve(4300);
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
    body += F("\",\"firmwareVersion\":\"0.39.2");
    body += F("\",\"buildDate\":\"");
    body += __DATE__;
    body += ' ';
    body += __TIME__;
    body += F("\",\"uptimeSeconds\":");
    body += String(millis() / 1000UL);
    body += F(",\"freeHeap\":");
    body += String(ESP.getFreeHeap());
    body += F(",\"psramTotal\":");
    body += String(ESP.getPsramSize());
    body += F(",\"freePsram\":");
    body += String(ESP.getFreePsram());
    body += F(",\"otaReady\":");
    body += otaStarted_ ? F("true") : F("false");
    body += F(",\"lastBackupEpoch\":");
    body += String(
        backupManager_ != nullptr
            ? static_cast<unsigned long>(backupManager_->lastBackupEpoch())
            : 0UL
    );
    body += F(",\"running\":");
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
    body += F(",\"manualSeasonPercent\":");
    body += String(smartControlManager_->manualSeasonPercent());
    body += F(",\"seasonAutomatic\":");
    body += smartControlManager_->seasonAutomatic() ? F("true") : F("false");
    body += F(",\"seasonValid\":");
    body += seasonManager_->isValid() ? F("true") : F("false");
    body += F(",\"seasonName\":\"");
    body += jsonEscape(seasonManager_->seasonName());
    body += F("\",\"seasonDayLength\":");
    body += String(seasonManager_->dayLengthHours(), 2);
    body += F(",\"seasonSunrise\":");
    body += String(seasonManager_->sunriseMinutes());
    body += F(",\"seasonSunset\":");
    body += String(seasonManager_->sunsetMinutes());
    body += F(",\"seasonExplanation\":\"");
    body += jsonEscape(seasonManager_->explanation());
    body += '"';
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
    body += F(",\"advisorWeatherAdjustment\":");
    body += String(advisor.weatherAdjustmentPercent);
    body += F(",\"advisorSeasonPercent\":");
    body += String(advisor.seasonPercent);
    body += F(",\"advisorCombinedPercent\":");
    body += String(advisor.combinedPercent);
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
    bool manualRun = false;
    int8_t manualValve = -1;
    uint32_t currentRunSeconds = 0;

    if (runtimeManager_->isRunning())
    {
        const uint8_t runningValve =
            runtimeManager_->runningValveIndex();

        currentRunSeconds =
            runtimeManager_->durationSeconds() -
            runtimeManager_->remainingSeconds();

        currentRunLiters =
            waterManager_->valveFlowRate(runningValve) *
            (
                static_cast<float>(currentRunSeconds) /
                60.0f
            );

        currentRunCost =
            currentRunLiters *
            waterManager_->waterPrice() /
            1000.0f;
    }
    else
    {
        for (uint8_t i = 0;
             i < AppConfig::DISPLAYED_VALVE_COUNT;
             ++i)
        {
            if (!manualValveRunActive_[i])
            {
                continue;
            }

            manualRun = true;
            manualValve = static_cast<int8_t>(i);

            currentRunSeconds =
                static_cast<uint32_t>(
                    millis() - manualValveStartedAtMs_[i]
                ) / 1000UL;

            currentRunLiters =
                waterManager_->valveFlowRate(i) *
                (
                    static_cast<float>(currentRunSeconds) /
                    60.0f
                );

            currentRunCost =
                currentRunLiters *
                waterManager_->waterPrice() /
                1000.0f;

            break;
        }
    }

    body += F(",\"waterCurrentRun\":");
    body += String(currentRunLiters, 3);
    body += F(",\"waterCurrentCost\":");
    body += String(currentRunCost, 4);
    body += F(",\"currentRunSeconds\":");
    body += String(currentRunSeconds);
    body += F(",\"manualRun\":");
    body += manualRun ? F("true") : F("false");
    body += F(",\"manualValve\":");
    body += String(manualValve);
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
        body += F(",\"profileId\":");
        body += String(p.profileId);
        body += F(",\"profileName\":\"");
        body += jsonEscape(String(GardenProfiles::name(p.profileId)));
        body += F("\"");
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
    const uint8_t profileId = server_.hasArg("profile") ? static_cast<uint8_t>(server_.arg("profile").toInt()) : 0;
    const int16_t index = scheduler_->createProgram(static_cast<uint8_t>(valve));
    if (index < 0)
    {
        sendJson(409, "{\"error\":\"Kein freier Programmplatz\"}");
        return;
    }
    scheduler_->setProfile(static_cast<uint8_t>(index), profileId);
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
    if (server_.hasArg("profile")) ok &= scheduler_->setProfile(i, server_.arg("profile").toInt());
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
    scheduler_->setProfile(t, source.profileId);
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

    const bool seasonAutomatic =
        server_.hasArg("seasonAuto")
            ? server_.arg("seasonAuto").toInt() != 0
            : smartControlManager_->seasonAutomatic();

    const bool vacationEnabled =
        server_.hasArg("enabled") &&
        server_.arg("enabled").toInt() != 0;

    const uint8_t seasonPercent =
        server_.hasArg("season")
            ? static_cast<uint8_t>(server_.arg("season").toInt())
            : smartControlManager_->manualSeasonPercent();

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

    smartControlManager_->setSeasonAutomatic(seasonAutomatic);
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

void WebManager::handleProfiles()
{
    String body;
    body.reserve(2200);
    body += F("{\"profiles\":[");

    for (uint8_t i = 0;
         i < GardenProfiles::PROFILE_COUNT;
         ++i)
    {
        if (i > 0)
        {
            body += ',';
        }

        const auto& profile =
            GardenProfiles::profileByIndex(i);

        body += F("{\"id\":");
        body += String(i);
        body += F(",\"name\":\"");
        body += jsonEscape(profile.name);
        body += F("\",\"symbol\":\"");
        body += jsonEscape(profile.symbol);
        body += F("\",\"correction\":");
        body += String(profile.correctionPercent);
        body += F(",\"temperature\":");
        body += String(
            profile.temperatureSensitivityPercent
        );
        body += F(",\"humidity\":");
        body += String(
            profile.humiditySensitivityPercent
        );
        body += F(",\"rain\":");
        body += String(
            profile.rainSensitivityPercent
        );
        body += F(",\"minimum\":");
        body += String(profile.minimumMinutes);
        body += F(",\"maximum\":");
        body += String(profile.maximumMinutes);
        body += '}';
    }

    body += F("]}");
    sendJson(200, body);
}

void WebManager::handleProfileSave()
{
    const char* required[] =
    {
        "id",
        "name",
        "correction",
        "temperature",
        "humidity",
        "rain",
        "minimum",
        "maximum"
    };

    for (const char* key : required)
    {
        if (!server_.hasArg(key))
        {
            sendJson(
                400,
                "{\"error\":\"Profildaten fehlen\"}"
            );
            return;
        }
    }

    const uint8_t id =
        static_cast<uint8_t>(
            server_.arg("id").toInt()
        );

    const String symbol =
        server_.hasArg("symbol")
            ? server_.arg("symbol")
            : String();

    const bool saved = GardenProfiles::update(
        id,
        server_.arg("name"),
        symbol,
        static_cast<int16_t>(
            server_.arg("correction").toInt()
        ),
        static_cast<uint8_t>(
            server_.arg("temperature").toInt()
        ),
        static_cast<uint8_t>(
            server_.arg("humidity").toInt()
        ),
        static_cast<uint8_t>(
            server_.arg("rain").toInt()
        ),
        static_cast<uint16_t>(
            server_.arg("minimum").toInt()
        ),
        static_cast<uint16_t>(
            server_.arg("maximum").toInt()
        )
    );

    if (!saved)
    {
        sendJson(
            400,
            "{\"error\":\"Ungültige Profilwerte\"}"
        );
        return;
    }

    Log.addf(
        LogManager::Category::System,
        LogManager::Level::Info,
        "Gartenprofil %u gespeichert",
        static_cast<unsigned>(id)
    );

    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleProfilesReset()
{
    GardenProfiles::resetDefaults();

    Log.info(
        LogManager::Category::System,
        "Gartenprofile auf Standardwerte gesetzt"
    );

    sendJson(200, "{\"ok\":true}");
}

void WebManager::handleHistory()
{
    if (historyManager_ == nullptr ||
        !historyManager_->isReady())
    {
        sendJson(
            503,
            "{\"error\":\"HistoryManager nicht bereit\"}"
        );
        return;
    }

    constexpr uint16_t DEFAULT_WEB_ENTRIES = 30;
    constexpr uint16_t MAX_WEB_ENTRIES = 200;

    uint16_t requested = DEFAULT_WEB_ENTRIES;

    if (server_.hasArg("limit"))
    {
        const int parsed = server_.arg("limit").toInt();

        if (parsed > 0)
        {
            requested = static_cast<uint16_t>(
                constrain(parsed, 1, static_cast<int>(MAX_WEB_ENTRIES))
            );
        }
    }

    const uint16_t total =
        historyManager_->count();

    const uint16_t visible =
        total < requested
            ? total
            : requested;

    String body;
    body.reserve(
        256U +
        static_cast<size_t>(visible) *
        300U
    );

    body += F("{\"total\":");
    body += String(total);
    body += F(",\"entries\":[");

    bool first = true;

    for (uint16_t i = 0;
         i < visible;
         ++i)
    {
        HistoryManager::HistoryEntry entry;

        if (!historyManager_->readNewest(
                i,
                entry
            ))
        {
            continue;
        }

        if (!first)
        {
            body += ',';
        }
        first = false;

        body += F("{\"id\":");
        body += String(entry.eventId);
        body += F(",\"timestamp\":");
        body += String(
            static_cast<long long>(
                entry.timestamp
            )
        );
        body += F(",\"programId\":");
        body += String(entry.programId);
        body += F(",\"valve\":");
        body += String(entry.valveIndex);
        body += F(",\"profile\":");
        body += String(entry.profileId);
        body += F(",\"plannedSeconds\":");
        body += String(entry.plannedSeconds);
        body += F(",\"actualSeconds\":");
        body += String(entry.actualSeconds);
        body += F(",\"liters\":");
        body += String(entry.liters, 2);
        body += F(",\"costEuro\":");
        body += String(entry.costEuro, 3);
        body += F(",\"advisorPercent\":");
        body += String(entry.advisorPercent);
        body += F(",\"seasonPercent\":");
        body += String(entry.seasonPercent);
        body += F(",\"automatic\":");
        body += entry.automatic ? F("true") : F("false");
        body += F(",\"event\":\"");
        body += jsonEscape(String(entry.event));
        body += F("\",\"reason\":\"");
        body += jsonEscape(String(entry.reason));
        body += F("\",\"firmware\":\"");
        body += jsonEscape(String(entry.firmware));
        body += F("\"}");
    }

    body += F("]}");

    sendJson(200, body);
}

void WebManager::handleGarden()
{
    if (gardenManager_ == nullptr ||
        !gardenManager_->isReady())
    {
        sendJson(
            503,
            "{\"error\":\"GardenManager nicht bereit\"}"
        );
        return;
    }

    sendJson(
        200,
        gardenManager_->exportJson()
    );
}

void WebManager::handleGardenSave()
{
    if (gardenManager_ == nullptr ||
        !gardenManager_->isReady())
    {
        sendJson(
            503,
            "{\"error\":\"GardenManager nicht bereit\"}"
        );
        return;
    }

    if (!server_.hasArg("plain"))
    {
        sendJson(
            400,
            "{\"error\":\"Keine JSON-Daten empfangen\"}"
        );
        return;
    }

    const String body =
        server_.arg("plain");

    if (body.length() == 0 ||
        body.length() > 16000)
    {
        sendJson(
            400,
            "{\"error\":\"Gartenkarte leer oder zu groß\"}"
        );
        return;
    }

    String message;

    if (!gardenManager_->
            importJson(
                body,
                message
            ))
    {
        sendJson(
            400,
            String("{\"error\":\"") +
            jsonEscape(message) +
            "\"}"
        );
        return;
    }

    sendJson(
        200,
        String("{\"ok\":true,\"zones\":") +
        String(gardenManager_->count()) +
        ",\"message\":\"" +
        jsonEscape(message) +
        "\"}"
    );
}

void WebManager::handleGardenReset()
{
    if (gardenManager_ == nullptr ||
        !gardenManager_->isReady())
    {
        sendJson(
            503,
            "{\"error\":\"GardenManager nicht bereit\"}"
        );
        return;
    }

    if (!gardenManager_->reset())
    {
        sendJson(
            500,
            "{\"error\":\"Gartenkarte konnte nicht zurückgesetzt werden\"}"
        );
        return;
    }

    sendJson(
        200,
        "{\"ok\":true,\"zones\":0}"
    );
}

void WebManager::handleBackupRestore()
{
    if (backupManager_ == nullptr)
    {
        sendJson(
            503,
            "{\"error\":\"BackupManager nicht bereit\"}"
        );
        return;
    }

    if (!server_.hasArg("plain"))
    {
        sendJson(
            400,
            "{\"error\":\"Keine JSON-Daten empfangen\"}"
        );
        return;
    }

    const String body =
        server_.arg("plain");

    if (body.length() == 0 ||
        body.length() > 30000)
    {
        sendJson(
            400,
            "{\"error\":\"Backup-Datei leer oder zu groß\"}"
        );
        return;
    }

    String message;

    if (!backupManager_->
            restoreBackupJson(
                body,
                message
            ))
    {
        sendJson(
            400,
            String("{\"error\":\"") +
            jsonEscape(message) +
            "\"}"
        );
        return;
    }

    const String logMessage =
        String("Backup wiederhergestellt: ") +
        message;

    Log.info(
        LogManager::Category::System,
        logMessage.c_str()
    );

    sendJson(
        200,
        String("{\"ok\":true,\"message\":\"") +
        jsonEscape(message) +
        "\"}"
    );

    restartRequestedAtMs_ = millis();
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
