#ifndef OTA_H
#define OTA_H

// =========================================================================
//  OTA fuer ESP8266 (HelloCube / GeekMagic 12F)
//  Flash-Size in der Arduino-IDE: "4MB (FS:2MB OTA:~1019KB)" o.ae. -> OTA ok.
//  Auf echtem 1MB-Modul waere OTA zu eng; hier aber 12F/4MB -> passt.
// =========================================================================

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Adafruit_ST7789.h>

extern Adafruit_ST7789 tft;

#define OTA_PASSWORD_PHRASE "GitHub345809regdfjhokfgdsfgdgf"

#define OTA_TFT_BLACK ST77XX_BLACK
#define OTA_TFT_GREEN ST77XX_GREEN
#define OTA_TFT_WHITE ST77XX_WHITE

inline void showOTAScreen() {
  tft.fillScreen(OTA_TFT_BLACK);
  tft.setFont(NULL);
  tft.setTextColor(OTA_TFT_GREEN, OTA_TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(80, 70);
  tft.print("OTA");
  tft.setCursor(50, 110);
  tft.print("UPDATE");

  tft.drawRect(28, 160, 184, 18, OTA_TFT_WHITE);
  tft.fillRect(30, 162, 180, 14, OTA_TFT_BLACK);
}

inline void setupOTA() {
  uint8_t mac[6];
  WiFi.macAddress(mac);

  char hostname[32];
  snprintf(hostname, sizeof(hostname), "HelloCube-%02X%02X%02X", mac[3], mac[4], mac[5]);

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(OTA_PASSWORD_PHRASE);
  MDNS.begin(hostname);

  ArduinoOTA.onStart([]() {
    Serial.println("\n[OTA] Start...");
    showOTAScreen();
  });

  ArduinoOTA.onEnd([]() {
    tft.fillRect(20, 195, 200, 25, OTA_TFT_BLACK);
    tft.setFont(NULL);
    tft.setTextSize(2);
    tft.setTextColor(OTA_TFT_GREEN, OTA_TFT_BLACK);
    tft.setCursor(45, 195);
    tft.print("REBOOT...");
    Serial.println("\n[OTA] Erfolgreich geflasht!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static int lastReportedPercent = -1;
    int percent = progress / (total / 100);

    if (percent >= lastReportedPercent + 5 || percent == 100) {
      lastReportedPercent = percent;

      int barWidth = (percent * 180) / 100;
      tft.fillRect(30, 162, barWidth, 14, OTA_TFT_GREEN);

      tft.fillRect(75, 195, 90, 20, OTA_TFT_BLACK);
      tft.setFont(NULL);
      tft.setTextSize(2);
      tft.setTextColor(OTA_TFT_WHITE, OTA_TFT_BLACK);
      tft.setCursor(85, 195);
      tft.printf("%d%%", percent);

      Serial.printf("[OTA] Fortschritt: %d%%\n", percent);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    tft.fillScreen(OTA_TFT_BLACK);
    tft.setFont(NULL);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED, OTA_TFT_BLACK);
    tft.setCursor(25, 90);
    tft.print("OTA FEHLER!");

    tft.setTextSize(1);
    tft.setTextColor(OTA_TFT_WHITE, OTA_TFT_BLACK);
    tft.setCursor(25, 125);

    if (error == OTA_AUTH_ERROR)         tft.print("Auth fehlgeschlagen");
    else if (error == OTA_BEGIN_ERROR)   tft.print("Start fehlgeschlagen (Flash/Slot)");
    else if (error == OTA_CONNECT_ERROR) tft.print("Connect Timeout");
    else if (error == OTA_RECEIVE_ERROR) tft.print("Empfangsfehler");
    else if (error == OTA_END_ERROR)     tft.print("Abschlussfehler");

    Serial.printf("[OTA] Fehler [%u]!\n", error);
    delay(2000);
    ESP.restart();
  });

  ArduinoOTA.begin();
  Serial.printf("[OTA] Bereit unter Hostname: %s\n", hostname);
}

inline void handleOTA() {
  MDNS.update();
  ArduinoOTA.handle();
}

#endif // OTA_H
