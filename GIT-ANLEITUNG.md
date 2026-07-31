# GardenFlow mit Git

## Erster Start

Das Projekt ist bereits als Git-Repository eingerichtet. Die lokale Datei
`include/app/AppSecrets.h` enthält die WLAN-Zugangsdaten und wird durch
`.gitignore` nicht eingecheckt.

```bash
cd GardenFlow-Git
rm -rf .pio
pio run -e esp32-4827S043R
```

## Änderungen speichern

```bash
git status
git add src include data platformio.ini
git commit -m "Beschreibung der Änderung"
```

## GitHub verbinden

Auf GitHub zuerst ein leeres Repository anlegen, ohne README oder Lizenz.
Danach im Projektordner:

```bash
git remote add origin https://github.com/pdadoktor-png/GardenFlow.git
git push -u origin main
```

Falls der gewünschte Repository-Name anders lautet, die URL entsprechend ändern.

## Spätere Updates

```bash
git pull --rebase
pio run -e esp32-4827S043R
```

## WLAN-Daten auf einem neuen Rechner

```bash
cp include/app/AppSecrets.example.h include/app/AppSecrets.h
```

Dann SSID und Passwort in `include/app/AppSecrets.h` eintragen.
