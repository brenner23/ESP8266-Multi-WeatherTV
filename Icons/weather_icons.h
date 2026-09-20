#pragma once
#include <Arduino.h>

#include "cloud_icon.h"
#include "fog_icon.h"
#include "heavy_rain_icon.h"
#include "moon_icon.h"
#include "partly_cloudy_icon.h"
#include "snowy_icon.h"
#include "sun_icon.h"
#include "thunderstorm_icon.h"

inline const uint16_t* getIconByOwmId(int conditionId, bool isNight) {
  if (isNight && conditionId == 800) return moon_icon;
  if (conditionId >= 200 && conditionId < 300) return thunderstorm_icon;
  if (conditionId >= 300 && conditionId < 600) return heavy_rain_icon;
  if (conditionId >= 600 && conditionId < 700) return snowy_icon;
  if (conditionId >= 700 && conditionId < 800) return fog_icon;
  if (conditionId == 800) return sun_icon;
  if (conditionId == 801 || conditionId == 802) return partly_cloudy_icon;
  return cloud_icon;
}