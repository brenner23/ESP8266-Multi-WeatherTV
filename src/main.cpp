#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <SPI.h>
#include <time.h>

// ── Grafik ────────────────────────────────────────────────
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ── Pinout & Schalter (USE_MIRROR, HAS_FORECAST) ──────────
#include "pins.h"

// ── Instanzen (muessen VOR config_portal.h stehen) ────────
Adafruit_ST7789   tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ESP8266WebServer  server(80);

// ── Hardware-Dimmer (P-MOSFET Low-Aktiv, analogWrite invertiert 0..1023) ─
void applyBrightness(uint8_t percent) {
  percent = constrain(percent, 0, 100);
  int duty = map(percent, 0, 100, 0, BL_PWM_MAX);   // 0..1023
  analogWrite(TFT_BACKLIGHT, BL_PWM_MAX - duty);     // invertiert: 0=hell
}

// ── Portal, OTA & Icons ───────────────────────────────────
#include "config_portal.h"
#include "ota.h"
#include "weather_icons.h"
#include "tux1_icon.h"
#include "icons.h"
#if HAS_FORECAST
  #include "forecast.h"
#endif

// ── Fonts ─────────────────────────────────────────────────
#include "OCRA10pt7b.h"
#include "AGENCYB50pt7b.h"

// ── Globale Variablen ─────────────────────────────────────
const unsigned long AUTO_FETCH_INTERVAL = 15 * 60 * 1000UL;
const unsigned long WIFI_CHECK_INTERVAL = 60 * 1000UL;

struct WeatherStorage {
  float    temp;
  int      humidity;
  int      pressure;
  int      conditionId;
  char     iconCode[4];
  uint32_t lastFetchEpoch;
};
WeatherStorage currentW = {0.0f, 0, 0, 800, "01d", 0};

// Wetter-EEPROM-Ablage direkt hinter dem Config-Block
#define W_EE_ADDR   (CP_EE_ADDR_CFG + sizeof(DeviceConfig) + 4)
#define W_EE_MARKER 0xC7

int lastMinuteDrawn = -1;
int lastDayDrawn    = -1;
unsigned long lastWiFiCheckTime = 0;

// ── Vektor-Icons fuer die Live-Ansicht ─────────────────────
void drawThermometerIcon(int x, int y) {
  uint16_t red = ST77XX_RED, white = ST77XX_WHITE;
  tft.fillCircle(x, y + 4, 4, red);
  tft.fillRect(x - 2, y - 6, 4, 8, red);
  tft.fillCircle(x, y - 6, 2, red);
  tft.drawPixel(x - 1, y + 4, white);
}

void drawWaterDropIcon(int x, int y) {
  uint16_t blue = tft.color565(0, 180, 255);
  tft.fillCircle(x, y + 3, 4, blue);
  tft.fillTriangle(x - 4, y + 2, x + 4, y + 2, x, y - 6, blue);
  tft.drawPixel(x - 1, y + 2, ST77XX_WHITE);
}

// ── Live-Wetter im EEPROM ─────────────────────────────────
void saveWeatherToNVS() {
  cpEnsureEE();
  EEPROM.write(W_EE_ADDR, W_EE_MARKER);
  EEPROM.put(W_EE_ADDR + 1, currentW);
  EEPROM.commit();
}

void loadWeatherFromNVS() {
  cpEnsureEE();
  if (EEPROM.read(W_EE_ADDR) == W_EE_MARKER) {
    EEPROM.get(W_EE_ADDR + 1, currentW);
    currentW.iconCode[sizeof(currentW.iconCode) - 1] = '\0';
  } else {
    currentW.pressure    = 1013;
    currentW.conditionId = 800;
    strcpy(currentW.iconCode, "01d");
  }
}

// ── OpenWeather Abruf ─────────────────────────────────────
bool fetchOpenWeather() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (strlen(cfg.owKey) == 0 || strlen(cfg.owCity) == 0) return false;

  WiFiClient client;
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" +
               String(cfg.owCity) + "," + String(cfg.owCountry) +
               "&units=" + String(cfg.owUnits) +
               "&lang="  + String(cfg.owLang) +
               "&appid=" + String(cfg.owKey);

  http.begin(client, url);
  int httpCode = http.GET();
  bool success = false;

  if (httpCode == HTTP_CODE_OK) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      currentW.temp        = doc["main"]["temp"] | 0.0f;
      currentW.humidity    = doc["main"]["humidity"] | 0;
      currentW.pressure    = doc["main"]["pressure"] | 1013;
      currentW.conditionId = doc["weather"][0]["id"] | 800;
      const char* ic       = doc["weather"][0]["icon"] | "01d";
      strncpy(currentW.iconCode, ic, sizeof(currentW.iconCode));
      currentW.iconCode[sizeof(currentW.iconCode) - 1] = '\0';
      time_t now; time(&now);
      currentW.lastFetchEpoch = (uint32_t)now;
      saveWeatherToNVS();
      success = true;
    }
  }
  http.end();
  return success;
}

// ── Rendering (Hauptansicht) ──────────────────────────────
void updateDateOnly(struct tm* ptm, bool validTime) {
  tft.fillRect(0, 0, 160, 60, ST77XX_BLACK);
  tft.setFont(&OCRA10pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);

  char dStr[16], wStr[16];
  if (validTime) {
    strftime(dStr, sizeof(dStr), "%d.%m.%Y", ptm);
    strftime(wStr, sizeof(wStr), "%A", ptm);
    for (int i = 0; wStr[i]; i++) wStr[i] = toupper(wStr[i]);
  } else {
    strcpy(dStr, "--.--.----");
    strcpy(wStr, "WARTE NTP");
  }
  tft.setCursor(5, 20); tft.print(dStr);
  tft.setCursor(5, 45); tft.print(wStr);
}

void updateClock(struct tm* ptm, bool validTime) {
  tft.fillRect(0, 72, SCREEN_W, 84, ST77XX_BLACK);
  tft.setFont(&AGENCYB50pt7b);
  tft.setTextSize(1);

  char hStr[4], mStr[4];
  if (validTime) {
    strftime(hStr, sizeof(hStr), "%H", ptm);
    strftime(mStr, sizeof(mStr), "%M", ptm);
  } else {
    strcpy(hStr, "--"); strcpy(mStr, "--");
  }

  int16_t x1, y1;
  uint16_t wHours, hHours, wColon, hColon, wMins, hMins;
  tft.getTextBounds(hStr, 0, 0, &x1, &y1, &wHours, &hHours);
  tft.getTextBounds(":",  0, 0, &x1, &y1, &wColon, &hColon);
  tft.getTextBounds(mStr, 0, 0, &x1, &y1, &wMins, &hMins);

  const int spacing = 4;
  int totalWidth = wHours + spacing + wColon + spacing + wMins;
  int startX = (SCREEN_W - totalWidth) / 2;
  int baseY  = 148;

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(startX, baseY); tft.print(hStr);

  int colonX = startX + wHours + spacing;
  tft.setCursor(colonX, baseY); tft.print(":");

  int minX = colonX + wColon + spacing;
  tft.setTextColor(tft.color565(229, 169, 60));
  tft.setCursor(minX, baseY); tft.print(mStr);
}

void updateWeatherOnly() {
  tft.fillRect(165, 5, 64, 64, ST77XX_BLACK);
  bool isNight = (strchr(currentW.iconCode, 'n') != nullptr);
  const uint16_t* icon = getIconByOwmId(currentW.conditionId, isNight);
  tft.drawRGBBitmap(165, 5, icon, 64, 64);

  tft.fillRect(0, 160, SCREEN_W, 80, ST77XX_BLACK);

  char buf[40];
  uint16_t barBlue = tft.color565(0, 165, 255);

  drawThermometerIcon(12, 172);
  tft.drawRect(24, 168, 96, 9, ST77XX_WHITE);
  tft.fillRect(25, 169, 94, 7, ST77XX_BLACK);
  int tW = constrain(map((int)currentW.temp, -10, 40, 0, 94), 0, 94);
  tft.fillRect(25, 169, tW, 7, barBlue);

  // %f-frei: Adafruit-GFX + printf-float beissen sich auf Xtensa
  tft.setFont(NULL);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  snprintf(buf, sizeof(buf), "%s\xF7" "C", String(currentW.temp, 0).c_str());
  tft.setCursor(130, 165); tft.print(buf);

  drawWaterDropIcon(12, 193);
  tft.drawRect(24, 189, 96, 9, ST77XX_WHITE);
  tft.fillRect(25, 190, 94, 7, ST77XX_BLACK);
  int hW = constrain(map(currentW.humidity, 0, 100, 0, 94), 0, 94);
  tft.fillRect(25, 190, hW, 7, barBlue);

  tft.setFont(&OCRA10pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  snprintf(buf, sizeof(buf), "%d%%", currentW.humidity);
  tft.setCursor(130, 198); tft.print(buf);

  float cmHg = currentW.pressure * 0.0750062f;
  snprintf(buf, sizeof(buf), "%d hPa | %s cmHg", currentW.pressure, String(cmHg, 1).c_str());
  tft.setCursor(10, 226); tft.print(buf);
}

void refreshEntireDisplay() {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);
  bool validTime = (now > 1600000000);
  updateDateOnly(ptm, validTime);
  updateClock(ptm, validTime);
  updateWeatherOnly();
  if (validTime) { lastMinuteDrawn = ptm->tm_min; lastDayDrawn = ptm->tm_mday; }
}

// ── Boot-Log ──────────────────────────────────────────────
int bootLineY = 8;
void printBootLog(const char* tag, const char* msg, uint16_t tagColor) {
  tft.setFont(NULL); tft.setTextSize(1);
  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "[%3lu.%03lu000] ", millis() / 1000, millis() % 1000);
  tft.setTextColor(tft.color565(140, 140, 140), ST77XX_BLACK);
  tft.setCursor(4, bootLineY); tft.print(timeBuf);
  tft.setTextColor(tagColor, ST77XX_BLACK); tft.print(tag); tft.print(" ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); tft.print(msg);
  bootLineY += 12;
}

// ── Display-Init inkl. Spiegel-Schalter ───────────────────
void initDisplay() {
  SPI.begin();                       // HW-SPI fest auf GPIO13/14
  tft.init(SCREEN_W, SCREEN_H, SPI_MODE3);
  tft.setRotation(2);

#if USE_MIRROR
  // Prisma-Korrektur HelloCube: MADCTL (0x36) mit MX-Bit (0x40) horizontal spiegeln.
  // setRotation(2) setzt MADCTL selbst -> danach ueberschreiben.
  // Testwerte: 0xC0 = 180 | 0x40 = horizontal | 0x80 = vertikal | 0x00 = normal
  tft.startWrite();
  tft.writeCommand(0x36);
  tft.spiWrite(0x40);      // MX = horizontal gespiegelt (Cube-Prisma)
  tft.endWrite();
#endif

  tft.fillScreen(ST77XX_BLACK);
}

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n>>> ESP8266 HelloCube BEREIT <<<\n");

  loadDeviceConfig();
  analogWriteRange(BL_PWM_MAX);      // 8266: Range explizit auf 1023
  applyBrightness(cfg.brightness);

  initDisplay();

  factoryCounterOnBoot();

  if (!isProvisioned()) {
    Serial.println("[SETUP] Keine Zugangsdaten -> Starte AP-Provisioning");
    cpBeginProvisioningAP();
  }

  tft.drawRGBBitmap(170, 170, tux1_icon, 64, 64);
  printBootLog("[ OK ]", "Booting Kernel...", ST77XX_GREEN);
  loadWeatherFromNVS();
#if HAS_FORECAST
  loadForecastFromNVS();
#endif
  printBootLog("[ OK ]", "Mounted EEPROM storage.", ST77XX_GREEN);

  char buf[48];
  snprintf(buf, sizeof(buf), "Connecting: %s", cfg.ssid);
  printBootLog("[INFO]", buf, ST77XX_CYAN);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP8266-Cube");
  WiFi.begin(cfg.ssid, cfg.pass);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 25) { delay(300); retries++; yield(); }

  if (WiFi.status() == WL_CONNECTED) {
    snprintf(buf, sizeof(buf), "IP: %s", WiFi.localIP().toString().c_str());
    printBootLog("[ OK ]", buf, ST77XX_GREEN);
    setupOTA();
    printBootLog("[ OK ]", "OTA Service online.", ST77XX_GREEN);
  } else {
    printBootLog("[FAIL]", "WLAN connect failed!", ST77XX_RED);
  }

  printBootLog("[INFO]", "Syncing time...", ST77XX_CYAN);
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  time_t now = 0; int ntpRetries = 0;
  while (now < 1600000000 && ntpRetries < 12) { delay(400); time(&now); ntpRetries++; yield(); }
  if (now > 1600000000) {
    struct tm* ptm = localtime(&now);
    strftime(buf, sizeof(buf), "Time: %H:%M:%S OK", ptm);
    printBootLog("[ OK ]", buf, ST77XX_GREEN);
  } else {
    printBootLog("[WARN]", "NTP timeout", ST77XX_YELLOW);
  }

  printBootLog("[INFO]", "Starting Web Server...", ST77XX_CYAN);
  cpAttachRoutes(server);
  server.begin();
  printBootLog("[ OK ]", "Config portal online on :80", ST77XX_GREEN);

  if (WiFi.status() == WL_CONNECTED) {
    if (currentW.lastFetchEpoch == 0 ||
        (now - currentW.lastFetchEpoch > (AUTO_FETCH_INTERVAL / 1000))) {
      printBootLog("[INFO]", "Updating Weather...", ST77XX_CYAN);
      fetchOpenWeather();
      printBootLog("[ OK ]", "Weather sync done.", ST77XX_GREEN);
    }
#if HAS_FORECAST
    updateForecastIfNeeded();
#endif
  }

  delay(1400);
  tft.fillScreen(ST77XX_BLACK);
  refreshEntireDisplay();
}

// ── Loop ──────────────────────────────────────────────────
void loop() {
  if (WiFi.status() == WL_CONNECTED) handleOTA();
  server.handleClient();
  factoryCounterLoop();

  if (cpSaveRequested) { delay(400); ESP.restart(); }

  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);
  bool validTime = (now > 1600000000);

  if (millis() - lastWiFiCheckTime >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheckTime = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[WATCHDOG] WLAN getrennt! Reboot...");
      delay(300);
      ESP.restart();
    }
  }

  // Live-Wetter-Timer
  if (validTime && (now - currentW.lastFetchEpoch >= (AUTO_FETCH_INTERVAL / 1000))) {
    if (fetchOpenWeather()) {
      updateWeatherOnly();
    }
  }

  // Uhr aktualisieren
  if (validTime && ptm->tm_min != lastMinuteDrawn) {
    lastMinuteDrawn = ptm->tm_min;
    updateClock(ptm, true);
    if (ptm->tm_mday != lastDayDrawn) {
      lastDayDrawn = ptm->tm_mday;
      updateDateOnly(ptm, true);
    }
  }

  yield();
}