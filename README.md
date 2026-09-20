# ESP8266 Multi-WeatherTV

A multi-target weather display project for several **ESP8266-based GeekMagic / HelloCube devices** using a 240×240 ST7789V TFT.

The project is designed around a single code base with multiple PlatformIO environments. Depending on the selected target, the same firmware can run on a normal GeekMagic-style display or on a HelloCube with prism optics, where the screen image must be mirrored horizontally.

---

## Supported Targets

The available targets are defined in `platformio.ini`.

| PlatformIO Environment | Target | Flash / Layout | Display Orientation | Upload |
|---|---|---|---|---|
| `Geekmagic_Yellow_4m` | GeekMagic Yellow / standard ESP8266 display | ESP-12E/ESP-12F, 4 MB | Normal | USB |
| `Geekmagic_Yellow_ota` | GeekMagic Yellow / standard ESP8266 display | ESP-12E/ESP-12F, 4 MB | Normal | OTA |
| `Geekmagic_Clone_4m` | GeekMagic-compatible ESP8266 clone | ESP-01 layout, 1 MB linker layout | Normal | USB |
| `Geekmagic_Clone_ota` | GeekMagic-compatible ESP8266 clone | ESP-01 layout, 1 MB linker layout | Normal | OTA |
| `Geekmagic_Cube_USB` | HelloCube / prism cube | ESP-12E/ESP-12F, 4 MB | Horizontally mirrored | USB |
| `Geekmagic_Cube_OTA` | HelloCube / prism cube | ESP-12E/ESP-12F, 4 MB | Horizontally mirrored | OTA |

> Note: The environment name `Geekmagic_Clone_4m` is retained from the project, but this environment currently uses `board = esp01_1m` with a 1 MB linker layout.

---

## Main Features

- ESP8266-based
- One shared firmware source for several device variants
- ST7789V 240×240 TFT
- Hardware SPI
- SPI Mode 3
- Normal and horizontally mirrored display modes
- Prism correction for HelloCube
- OpenWeather current weather data
- Temperature in °C
- Humidity in %
- Air pressure in hPa
- Air pressure additionally shown in cmHg
- Weather icons based on OpenWeather condition codes
- Day/night weather icon support
- Large digital clock
- Date and weekday
- NTP synchronization
- Automatic CET / CEST daylight-saving handling
- Weather cache stored in EEPROM emulation
- Configuration stored in EEPROM emulation
- Automatic weather update every 15 minutes
- Integrated web configuration
- Wi-Fi network scan
- First-start access point
- Live brightness control from the browser
- PWM backlight control
- Factory reset from the web interface
- Optional boot-cycle factory reset mechanism
- OTA firmware updates
- OTA progress displayed on the TFT
- Wi-Fi watchdog with automatic restart after connection loss
- Linux-style boot screen
- Multiple custom fonts and weather icons

---

# Display

## Controller

```text
ST7789V
240 × 240 pixels
Hardware SPI
SPI_MODE3
Rotation 2
```

The display chip-select line is permanently connected to GND:

```cpp
#define TFT_CS -1
```

---

# Pin Assignment

The pin configuration is located in:

```text
include/pins.h
```

Current pin assignment:

| Function | GPIO | ESP8266 Label |
|---|---:|---|
| TFT SCK | GPIO 14 | D5 |
| TFT MOSI | GPIO 13 | D7 |
| TFT DC | GPIO 0 | D3 |
| TFT RESET | GPIO 2 | D4 |
| TFT Backlight | GPIO 5 | D1 |
| TFT CS | GND | — |
| TFT MISO | not used | — |

Hardware SPI on the ESP8266 uses GPIO13 for MOSI and GPIO14 for SCK.

---

# Normal vs. Mirrored Display

The same source code supports both standard displays and the HelloCube prism version.

The setting is controlled per PlatformIO environment with:

```text
-D USE_MIRROR=0
```

or:

```text
-D USE_MIRROR=1
```

## Normal GeekMagic / Clone

```text
USE_MIRROR=0
```

The display is rendered normally.

## HelloCube

```text
USE_MIRROR=1
```

The image is horizontally mirrored using the ST7789 MADCTL register.

This compensates for the optical prism/mirror construction used by the HelloCube.

The Cube environments already enable this automatically.

---

# No Touch / No Forecast Switching

This ESP8266 version does not use capacitive touch.

In `include/pins.h`:

```cpp
#define HAS_FORECAST 0
```

The current firmware therefore shows only the live weather screen.

The forecast code is intentionally disabled because the ESP8266 hardware used here has no capacitive touch input for switching between screens.

If a hardware button is added later, forecast switching could be implemented separately.

---

# Weather Screen

The main display shows:

- Date
- Weekday
- Current time
- Weather icon
- Temperature
- Humidity
- Air pressure in hPa
- Air pressure in cmHg

Temperature and humidity are also visualized with horizontal bars.

The current weather is downloaded directly from OpenWeather.

---

# OpenWeather

The firmware uses the OpenWeather current-weather endpoint.

The required configuration is:

```text
API key
City
Country
```

Default unit and language settings:

```text
Units:    metric
Language: de
Country:  DE
```

Weather data is refreshed automatically every:

```text
15 minutes
```

After a successful download, the weather data is stored in EEPROM emulation so the previous values remain available after a reboot.

---

# First Start / Wi-Fi Setup

If no saved Wi-Fi configuration is available, the ESP8266 starts its own setup access point.

The SSID begins with:

```text
ESP8266-WeatherTV-
```

The last characters are generated from the device MAC address.

Default setup password:

```text
12345678
```

The TFT displays the setup network name and IP address.

Connect a phone, tablet, or computer to this Wi-Fi network and open the displayed IP address in a browser.

---

# Web Configuration

The ESP8266 runs an HTTP configuration server on port 80.

After connecting the device to your normal network, open:

```text
http://DEVICE-IP/
```

The configuration page allows you to set:

- Wi-Fi SSID
- Wi-Fi password
- OpenWeather API key
- OpenWeather city
- OpenWeather country
- Display brightness

The page also includes:

- Wi-Fi scan
- Live brightness adjustment
- Save and restart
- Factory reset

---

# Configuration Storage

Unlike ESP32 projects that use `Preferences` / NVS, this ESP8266 version uses **EEPROM emulation**.

The stored configuration includes:

```text
Wi-Fi SSID
Wi-Fi password
OpenWeather API key
OpenWeather city
OpenWeather country
OpenWeather units
OpenWeather language
Display brightness
```

The last valid weather data is also stored in the EEPROM emulation area.

---

# Backlight / Brightness

The display backlight is controlled through:

```text
GPIO 5
```

The hardware uses an inverted P-channel MOSFET arrangement:

```text
LOW  = backlight ON
HIGH = backlight OFF
```

Brightness is controlled through ESP8266 `analogWrite()` with a range of:

```text
0 ... 1023
```

Because the circuit is inverted, the PWM value is inverted in software.

The brightness setting is stored permanently and restored after reboot.

---

# Time / NTP

The project uses:

```text
pool.ntp.org
time.nist.gov
```

Timezone configuration:

```text
CET-1CEST,M3.5.0,M10.5.0/3
```

This automatically handles Central European standard time and daylight-saving time.

---

# OTA Firmware Update

OTA updates are handled through `ArduinoOTA`.

The OTA hostname is generated from the ESP8266 MAC address:

```text
HelloCube-XXXXXX
```

During an OTA update, the TFT shows:

- OTA update screen
- Progress bar
- Percentage
- Reboot message
- Error message if the update fails

---

## OTA Environments

### GeekMagic Yellow

```text
Geekmagic_Yellow_ota
```

### GeekMagic Clone

```text
Geekmagic_Clone_ota
```

### HelloCube

```text
Geekmagic_Cube_OTA
```

Example:

```bash
pio run -e Geekmagic_Cube_OTA -t upload
```

The target IP addresses are configured in `platformio.ini`.

---

## OTA and Flash Size

The standard ESP-12E / ESP-12F targets use 4 MB flash.

OTA is normally suitable for these targets when the partition layout provides enough application space.

The clone environment currently uses a 1 MB linker layout:

```ini
board = esp01_1m
board_build.ldscript = eagle.flash.1m64.ld
```

OTA space on a real 1 MB module can be limited. If OTA fails because the firmware does not fit into the available update slot, use a wired upload instead.

---

# PlatformIO

The common base configuration uses:

```ini
platform  = espressif8266
framework = arduino
```

Main libraries:

```text
Adafruit ST7735 and ST7789 Library
Adafruit GFX Library
ArduinoJson
```

Serial monitor speed:

```text
115200 baud
```

Default environment:

```text
Geekmagic_Yellow_4m
```

---

## Build Examples

### GeekMagic Yellow via USB

```bash
pio run -e Geekmagic_Yellow_4m -t upload
```

### GeekMagic Yellow via OTA

```bash
pio run -e Geekmagic_Yellow_ota -t upload
```

### GeekMagic Clone via USB

```bash
pio run -e Geekmagic_Clone_4m -t upload
```

### HelloCube via USB

```bash
pio run -e Geekmagic_Cube_USB -t upload
```

### HelloCube via OTA

```bash
pio run -e Geekmagic_Cube_OTA -t upload
```

---

# Wi-Fi Watchdog

The Wi-Fi connection is checked every:

```text
60 seconds
```

If the ESP8266 loses its Wi-Fi connection, the firmware restarts the device automatically so it can reconnect cleanly.

---

# Factory Reset

A factory reset can be triggered through the web interface.

This clears the stored configuration from EEPROM emulation and restarts the ESP8266.

There is also an optional boot-cycle reset mechanism in the source code.

It is disabled by default:

```cpp
#define HW_RESET_MODE_ENABLED 0
```

---

# Project Structure

```text
src/
  main.cpp
  icons.h

include/
  config_portal.h
  credentials.h
  ota.h
  pins.h

Icons/
  weather_icons.h
  sun_icon.h
  cloud_icon.h
  fog_icon.h
  snowy_icon.h
  thunderstorm_icon.h
  heavy_rain_icon.h
  partly_cloudy_icon.h
  moon_icon.h
  tux1_icon.h
  ...

fonts/
  OCRA10pt7b.h
  OCRA24pt7b.h
  AGENCYB10pt7b.h
  AGENCYB15pt7b.h
  AGENCYB20pt7b.h
  AGENCYB25pt7b.h
  AGENCYB30pt7b.h
  AGENCYB35pt7b.h
  AGENCYB40pt7b.h
  AGENCYB45pt7b.h
  AGENCYB50pt7b.h

platformio.ini
```

---

# Security Before Publishing on GitHub

Before publishing this project publicly, check the source tree for private credentials.

Do not publish real:

- Wi-Fi passwords
- OpenWeather API keys
- OTA passwords
- private IP addresses you do not want to expose
- other API tokens or credentials

The project supports optional fallback credentials in:

```text
include/credentials.h
```

For a public repository, keep:

```cpp
#define USE_FALLBACK_CREDENTIALS 0
```

and leave the credential fields empty or replace them with placeholders.

The current project also contains an OTA password in the source and in the OTA PlatformIO upload flags. Replace it before publishing the repository.

Example:

```text
YOUR_OTA_PASSWORD
```

---

# License

This project is licensed under the **MIT License**.

See the `LICENSE` file for details.

---

# Deutsche Version

# ESP8266 Multi-WeatherTV

Ein gemeinsames Wetterdisplay-Projekt für mehrere **ESP8266-basierte GeekMagic-/HelloCube-Geräte** mit einem 240×240 ST7789V TFT.

Das Projekt verwendet eine gemeinsame Codebasis mit mehreren PlatformIO-Environments. Je nach gewähltem Target läuft dieselbe Firmware auf einem normalen GeekMagic-Display oder auf einem HelloCube mit Prisma, bei dem das Displaybild horizontal gespiegelt werden muss.

---

## Unterstützte Targets

Die verfügbaren Varianten sind in `platformio.ini` definiert.

| PlatformIO-Environment | Zielgerät | Flash / Layout | Display | Upload |
|---|---|---|---|---|
| `Geekmagic_Yellow_4m` | GeekMagic Yellow / normales ESP8266-Display | ESP-12E/ESP-12F, 4 MB | normal | USB |
| `Geekmagic_Yellow_ota` | GeekMagic Yellow / normales ESP8266-Display | ESP-12E/ESP-12F, 4 MB | normal | OTA |
| `Geekmagic_Clone_4m` | GeekMagic-kompatibler ESP8266-Clone | ESP-01-Layout, 1-MB-Linkerlayout | normal | USB |
| `Geekmagic_Clone_ota` | GeekMagic-kompatibler ESP8266-Clone | ESP-01-Layout, 1-MB-Linkerlayout | normal | OTA |
| `Geekmagic_Cube_USB` | HelloCube / Prisma-Würfel | ESP-12E/ESP-12F, 4 MB | horizontal gespiegelt | USB |
| `Geekmagic_Cube_OTA` | HelloCube / Prisma-Würfel | ESP-12E/ESP-12F, 4 MB | horizontal gespiegelt | OTA |

> Hinweis: Der Environment-Name `Geekmagic_Clone_4m` stammt aus dem Projekt. Technisch verwendet dieses Environment derzeit `board = esp01_1m` mit einem 1-MB-Linkerlayout.

---

## Funktionen

- ESP8266
- eine gemeinsame Firmware für mehrere Gerätevarianten
- ST7789V 240×240 TFT
- Hardware-SPI
- SPI Mode 3
- normale und horizontal gespiegelte Darstellung
- Prisma-Korrektur für den HelloCube
- aktuelle Wetterdaten über OpenWeather
- Temperatur in °C
- Luftfeuchtigkeit in %
- Luftdruck in hPa
- zusätzliche Luftdruckanzeige in cmHg
- Wettersymbole entsprechend dem OpenWeather-Wettercode
- Tag-/Nacht-Wettersymbole
- große Digitaluhr
- Datum und Wochentag
- NTP-Zeitsynchronisation
- automatische CET-/CEST-Sommerzeitumschaltung
- Wetter-Cache über EEPROM-Emulation
- Konfigurationsspeicherung über EEPROM-Emulation
- automatische Wetteraktualisierung alle 15 Minuten
- integrierte Web-Konfiguration
- WLAN-Scan
- Setup-Access-Point für den ersten Start
- Live-Helligkeitsregelung über den Browser
- PWM-Hintergrundbeleuchtung
- Werksreset über das Webinterface
- optionaler Werksreset über mehrere Bootvorgänge
- OTA-Firmwareupdates
- OTA-Fortschritt direkt auf dem TFT
- WLAN-Watchdog mit automatischem Neustart bei Verbindungsverlust
- Linux-artiger Bootscreen
- mehrere eigene Schriften und Wettergrafiken

---

# Display

## Controller

```text
ST7789V
240 × 240 Pixel
Hardware-SPI
SPI_MODE3
Rotation 2
```

Die Chip-Select-Leitung des Displays ist dauerhaft mit GND verbunden:

```cpp
#define TFT_CS -1
```

---

# Pinbelegung

Die Pinbelegung befindet sich in:

```text
include/pins.h
```

Aktuelle Belegung:

| Funktion | GPIO | ESP8266-Bezeichnung |
|---|---:|---|
| TFT SCK | GPIO 14 | D5 |
| TFT MOSI | GPIO 13 | D7 |
| TFT DC | GPIO 0 | D3 |
| TFT RESET | GPIO 2 | D4 |
| TFT Backlight | GPIO 5 | D1 |
| TFT CS | GND | — |
| TFT MISO | nicht verwendet | — |

Beim ESP8266 verwendet Hardware-SPI GPIO13 für MOSI und GPIO14 für SCK.

---

# Normale und gespiegelte Darstellung

Mit derselben Codebasis werden sowohl normale Displays als auch die Prisma-Version des HelloCube unterstützt.

Die Einstellung wird pro PlatformIO-Environment über folgendes Build-Flag vorgenommen:

```text
-D USE_MIRROR=0
```

oder:

```text
-D USE_MIRROR=1
```

## Normales GeekMagic / Clone

```text
USE_MIRROR=0
```

Das Bild wird normal dargestellt.

## HelloCube

```text
USE_MIRROR=1
```

Das Bild wird über das MADCTL-Register des ST7789 horizontal gespiegelt.

Dadurch wird die Spiegelwirkung des Prismas im HelloCube ausgeglichen.

Die Cube-Environments aktivieren diese Einstellung bereits automatisch.

---

# Kein Touch / keine Vorhersage-Umschaltung

Diese ESP8266-Version verwendet keinen kapazitiven Touch.

In `include/pins.h` steht:

```cpp
#define HAS_FORECAST 0
```

Die aktuelle Firmware zeigt deshalb nur die Live-Wetteransicht.

Die 3-Tage-Vorhersage ist bewusst deaktiviert, weil die hier verwendete ESP8266-Hardware keinen kapazitiven Touch-Eingang für die Bildschirmumschaltung besitzt.

Falls später ein Hardware-Taster ergänzt wird, könnte eine Umschaltung separat nachgerüstet werden.

---

# Wetteranzeige

Die Hauptansicht zeigt:

- Datum
- Wochentag
- aktuelle Uhrzeit
- Wettersymbol
- Temperatur
- Luftfeuchtigkeit
- Luftdruck in hPa
- Luftdruck in cmHg

Temperatur und Luftfeuchtigkeit werden zusätzlich als horizontale Balken dargestellt.

Die aktuellen Wetterdaten werden direkt von OpenWeather geladen.

---

# OpenWeather

Die Firmware verwendet die aktuelle Wetterabfrage von OpenWeather.

Benötigt werden:

```text
API-Key
Ort
Land
```

Standardwerte für Einheit und Sprache:

```text
Einheiten: metric
Sprache:   de
Land:      DE
```

Die Wetterdaten werden automatisch alle:

```text
15 Minuten
```

aktualisiert.

Nach einem erfolgreichen Abruf werden die Wetterdaten über die EEPROM-Emulation gespeichert, sodass nach einem Neustart zunächst die letzten gültigen Werte verfügbar bleiben.

---

# Erster Start / WLAN-Einrichtung

Wenn noch keine gespeicherte WLAN-Konfiguration vorhanden ist, startet der ESP8266 automatisch einen eigenen Setup-Access-Point.

Die SSID beginnt mit:

```text
ESP8266-WeatherTV-
```

Die letzten Zeichen werden aus der MAC-Adresse des Geräts erzeugt.

Standardpasswort:

```text
12345678
```

Auf dem TFT werden der Name des Setup-WLANs und die IP-Adresse angezeigt.

Mit Smartphone, Tablet oder PC mit diesem WLAN verbinden und anschließend die angezeigte IP-Adresse im Browser öffnen.

---

# Web-Konfiguration

Der ESP8266 stellt auf Port 80 einen HTTP-Konfigurationsserver bereit.

Nach der Verbindung mit dem normalen WLAN:

```text
http://GERAETE-IP/
```

Im Webinterface können eingestellt werden:

- WLAN-SSID
- WLAN-Passwort
- OpenWeather-API-Key
- OpenWeather-Ort
- OpenWeather-Land
- Displayhelligkeit

Zusätzlich stehen zur Verfügung:

- WLAN-Scan
- Live-Helligkeitsänderung
- Speichern und Neustart
- Werksreset

---

# Speicherung der Konfiguration

Anders als bei ESP32-Projekten mit `Preferences` / NVS verwendet diese ESP8266-Version die **EEPROM-Emulation**.

Gespeichert werden:

```text
WLAN-SSID
WLAN-Passwort
OpenWeather-API-Key
OpenWeather-Ort
OpenWeather-Land
OpenWeather-Einheiten
OpenWeather-Sprache
Displayhelligkeit
```

Auch die letzten gültigen Wetterdaten werden im EEPROM-Emulationsbereich gespeichert.

---

# Hintergrundbeleuchtung / Helligkeit

Die Hintergrundbeleuchtung wird über:

```text
GPIO 5
```

angesteuert.

Die Hardware verwendet eine invertierte P-MOSFET-Schaltung:

```text
LOW  = Beleuchtung EIN
HIGH = Beleuchtung AUS
```

Die Helligkeit wird über ESP8266 `analogWrite()` mit folgendem Bereich gesteuert:

```text
0 ... 1023
```

Da die Schaltung invertiert arbeitet, wird auch der PWM-Wert in der Software invertiert.

Die eingestellte Helligkeit wird dauerhaft gespeichert und nach dem Neustart wiederhergestellt.

---

# Uhrzeit / NTP

Verwendete Zeitserver:

```text
pool.ntp.org
time.nist.gov
```

Zeitzoneneinstellung:

```text
CET-1CEST,M3.5.0,M10.5.0/3
```

Damit werden mitteleuropäische Winter- und Sommerzeit automatisch berücksichtigt.

---

# OTA-Firmwareupdate

OTA-Updates werden über `ArduinoOTA` durchgeführt.

Der OTA-Hostname wird aus der MAC-Adresse erzeugt:

```text
HelloCube-XXXXXX
```

Während eines OTA-Updates zeigt das TFT:

- OTA-Bildschirm
- Fortschrittsbalken
- Prozentanzeige
- Neustartmeldung
- Fehlermeldung bei Problemen

---

## OTA-Environments

### GeekMagic Yellow

```text
Geekmagic_Yellow_ota
```

### GeekMagic Clone

```text
Geekmagic_Clone_ota
```

### HelloCube

```text
Geekmagic_Cube_OTA
```

Beispiel:

```bash
pio run -e Geekmagic_Cube_OTA -t upload
```

Die jeweiligen Ziel-IP-Adressen sind in `platformio.ini` eingetragen.

---

## OTA und Flashgröße

Die normalen ESP-12E-/ESP-12F-Targets verwenden 4 MB Flash.

OTA funktioniert dort normalerweise, wenn das verwendete Flashlayout ausreichend Platz für den zweiten Firmware-Slot bereitstellt.

Das Clone-Environment verwendet derzeit ein 1-MB-Linkerlayout:

```ini
board = esp01_1m
board_build.ldscript = eagle.flash.1m64.ld
```

Bei einem echten 1-MB-Modul kann der Platz für OTA knapp werden. Falls die Firmware nicht in den verfügbaren Update-Slot passt, sollte per Kabel geflasht werden.

---

# PlatformIO

Gemeinsame Basiskonfiguration:

```ini
platform  = espressif8266
framework = arduino
```

Wichtige Bibliotheken:

```text
Adafruit ST7735 and ST7789 Library
Adafruit GFX Library
ArduinoJson
```

Serieller Monitor:

```text
115200 Baud
```

Standard-Environment:

```text
Geekmagic_Yellow_4m
```

---

## Build-Beispiele

### GeekMagic Yellow per USB

```bash
pio run -e Geekmagic_Yellow_4m -t upload
```

### GeekMagic Yellow per OTA

```bash
pio run -e Geekmagic_Yellow_ota -t upload
```

### GeekMagic Clone per USB

```bash
pio run -e Geekmagic_Clone_4m -t upload
```

### HelloCube per USB

```bash
pio run -e Geekmagic_Cube_USB -t upload
```

### HelloCube per OTA

```bash
pio run -e Geekmagic_Cube_OTA -t upload
```

---

# WLAN-Watchdog

Die WLAN-Verbindung wird alle:

```text
60 Sekunden
```

überprüft.

Falls die Verbindung verloren geht, startet die Firmware den ESP8266 automatisch neu, damit eine saubere Wiederverbindung erfolgen kann.

---

# Werksreset

Ein Werksreset kann über das Webinterface ausgelöst werden.

Dabei werden die gespeicherten Einstellungen aus der EEPROM-Emulation gelöscht und der ESP8266 neu gestartet.

Zusätzlich existiert im Quelltext ein optionaler Resetmechanismus über mehrere schnelle Bootvorgänge.

Dieser ist standardmäßig deaktiviert:

```cpp
#define HW_RESET_MODE_ENABLED 0
```

---

# Projektstruktur

```text
src/
  main.cpp
  icons.h

include/
  config_portal.h
  credentials.h
  ota.h
  pins.h

Icons/
  weather_icons.h
  sun_icon.h
  cloud_icon.h
  fog_icon.h
  snowy_icon.h
  thunderstorm_icon.h
  heavy_rain_icon.h
  partly_cloudy_icon.h
  moon_icon.h
  tux1_icon.h
  ...

fonts/
  OCRA10pt7b.h
  OCRA24pt7b.h
  AGENCYB10pt7b.h
  AGENCYB15pt7b.h
  AGENCYB20pt7b.h
  AGENCYB25pt7b.h
  AGENCYB30pt7b.h
  AGENCYB35pt7b.h
  AGENCYB40pt7b.h
  AGENCYB45pt7b.h
  AGENCYB50pt7b.h

platformio.ini
```

---

# Sicherheit vor dem GitHub-Upload

Vor einem öffentlichen GitHub-Upload sollte das gesamte Projekt auf private Zugangsdaten geprüft werden.

Nicht veröffentlichen:

- echte WLAN-Passwörter
- echte OpenWeather-API-Keys
- OTA-Passwörter
- private IP-Adressen, die nicht öffentlich sichtbar sein sollen
- andere API-Tokens oder Zugangsdaten

Das Projekt unterstützt optionale Fallback-Zugangsdaten in:

```text
include/credentials.h
```

Für ein öffentliches Repository sollte:

```cpp
#define USE_FALLBACK_CREDENTIALS 0
```

aktiv bleiben und die Zugangsdaten leer bleiben oder durch Platzhalter ersetzt werden.

Im aktuellen Projekt ist außerdem ein OTA-Passwort im Quelltext und in den OTA-Upload-Flags von PlatformIO eingetragen. Dieses sollte vor dem öffentlichen Upload ersetzt werden.

Beispiel:

```text
YOUR_OTA_PASSWORD
```

---

# Lizenz

Dieses Projekt steht unter der **MIT-Lizenz**.

Weitere Informationen findest du in der Datei `LICENSE`.
