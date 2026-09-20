#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// ── WiFi Credentials ──
const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

// 1 = Nutzt beim ersten Booten/nach Flash diese Daten automatisch
// 0 = Ignoriert diese Daten, startet zwingend den AP-Setup-Bildschirm
#define USE_FALLBACK_CREDENTIALS 0

// ── OpenWeather API (optional) ──
const char* OPENWEATHER_API_KEY = "";
const char* OPENWEATHER_CITY     = "";
const char* OPENWEATHER_COUNTRY  = "DE";
const char* OPENWEATHER_UNITS    = "metric";
const char* OPENWEATHER_LANG     = "de";

// http://api.openweathermap.org/data/2.5/forecast?q=xxx&units=metric&lang=de&appid=xxxxxxxxxxxxxxxxxxxxxxxxxxxx

#endif
