# GardenFlow Architektur – v0.39.0a

Die Stabilisierung veraendert keine Bewaesserungslogik.

- `hardware/ValveManager`: Ventilimpulse
- `scheduler/Scheduler`: Programme
- `runtime/RuntimeManager`: Programmausfuehrung
- `weather/WeatherManager`: Wetter und Regenpause
- `season/SeasonManager`: Saisonautomatik
- `advisor/AdvisorEngine`: Empfehlungen
- `water/WaterManager`: Wasserbilanz
- `profiles/GardenProfiles`: Pflanzenprofile
- `network/WebManager`: HTTP-Routen und API
- `network/WebUiPage`: HTML/CSS/JavaScript

## Release-Regel

1. Vom letzten getesteten Git-Commit starten.
2. Keine Dateien aus alten ZIP-Staenden uebernehmen.
3. Clean Build.
4. Warnungen beseitigen.
5. Hardwaretest, dann Commit/Tag.
