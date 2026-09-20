#ifndef PINS_H
#define PINS_H

// =========================================================================
//  GeekMagic SmallTV / HelloCube  (ESP8266 ESP-12F oder ESP-01/8266)
//  ST7789 240x240
//  Quelle Pinout: community.home-assistant.io/t/618029
// =========================================================================

// ── SPI / Display ────────────────────────────────────────
// HW-SPI liegt beim 8266 FEST auf GPIO13(MOSI)/GPIO14(SCK) -> SPI.begin() ohne Pins.
#define TFT_SCK        14      // GPIO14 = D5  (Hardware SPI - fest)
#define TFT_MOSI       13      // GPIO13 = D7  (Hardware SPI - fest)
#define TFT_CS         -1      // fest auf GND verdrahtet
#define TFT_DC          0      // GPIO0  = D3
#define TFT_RST         2      // GPIO2  = D4

// ── Backlight ────────────────────────────────────────────
// P-MOSFET invertiert: LOW = Licht AN. analogWrite-Range 0..1023, ebenfalls invertiert.
#define TFT_BACKLIGHT   5      // GPIO5  = D1
#define BL_PWM_MAX   1023      // voller analogWrite-Bereich beim 8266

// ── Bildschirm ───────────────────────────────────────────
#define SCREEN_W       240
#define SCREEN_H       240

// =========================================================================
//  SPIEGEL-SCHALTER (fuer den HelloCube mit Prisma obendrauf)
//  1 = MADCTL-MX gesetzt -> Bild horizontal gespiegelt (Cube)
//  0 = normal (nacktes NM-TV / GeekMagic ohne Spiegel)
//
//  Wird jetzt ueber die build_flags in platformio.ini gesetzt:
//    Cube_* -> -D USE_MIRROR=1
//    NMTV_* -> -D USE_MIRROR=0
//  Der Default hier greift nur, wenn KEIN Flag gesetzt ist (z.B. Alt-Build).
// =========================================================================
#ifndef USE_MIRROR
  #define USE_MIRROR    1
#endif

// =========================================================================
//  FORECAST-SCHALTER
//  Der 8266 hat KEINEN kapazitiven Touch -> kein Umschalter moeglich.
//  0 = 3-Tage-Vorhersage komplett raus (nur Live-Ansicht). So gewollt.
//  (Wenn Du je einen Hardware-Button nachruestest: hier auf 1 + Button-Pin.)
// =========================================================================
#ifndef HAS_FORECAST
  #define HAS_FORECAST  0
#endif

#endif
