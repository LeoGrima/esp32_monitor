#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

enum class Sky : uint8_t { Clear, PartlyCloudy, Cloudy, Fog, Rain, Storm, Snow };

struct Condition {
  Sky sky;
  const char* label;
};

// Códigos WMO usados pela Open-Meteo
Condition conditionFromCode(uint8_t code) {
  if (code == 0) return {Sky::Clear, "Céu limpo"};
  if (code <= 2) return {Sky::PartlyCloudy, "Parcialmente nublado"};
  if (code == 3) return {Sky::Cloudy, "Nublado"};
  if (code <= 48) return {Sky::Fog, "Neblina"};
  if (code <= 57) return {Sky::Rain, "Garoa"};
  if (code <= 67) return {Sky::Rain, "Chuva"};
  if (code <= 77) return {Sky::Snow, "Neve"};
  if (code <= 82) return {Sky::Rain, "Pancadas de chuva"};
  if (code <= 86) return {Sky::Snow, "Neve"};
  return {Sky::Storm, "Tempestade"};
}

void drawCloud(LovyanGFX& gfx, int x, int y, int size, uint32_t tint) {
  gfx.fillSmoothCircle(x, y, size, tint);
  gfx.fillSmoothCircle(x + size, y - size / 2, size * 5 / 4, tint);
  gfx.fillSmoothCircle(x + size * 9 / 4, y, size, tint);
  gfx.fillRect(x, y, size * 9 / 4, size + 1, tint);
}

void drawSky(LovyanGFX& gfx, Sky sky, bool isDay) {
  const bool overcast = sky != Sky::Clear && sky != Sky::PartlyCloudy;
  if (!isDay) {
    fillBackground(gfx, color::kNightTop, overcast ? 0x2B3350u : color::kNightBottom);
    if (!overcast) drawStars(gfx, 40);
  } else if (overcast) {
    fillBackground(gfx, 0x55657Eu, 0x9FB0C6u);
  } else {
    fillBackground(gfx, color::kDayTop, color::kDayBottom);
  }

  const int centerX = gfx.width() - 70;
  const int centerY = 92;
  if (!overcast) {
    drawGlow(gfx, centerX, centerY, 46, isDay ? color::kSun : color::kMoon);
    gfx.fillSmoothCircle(centerX, centerY, 24, isDay ? color::kSun : color::kMoon);
  }
  if (sky == Sky::Clear) return;

  const bool dark = sky == Sky::Rain || sky == Sky::Storm || !isDay;
  const uint32_t cloud = dark ? color::kCloudDark : color::kCloud;
  if (overcast) drawCloud(gfx, centerX - 14, centerY - 10, 14, blend(cloud, 0x000000u, 0.12f));
  drawCloud(gfx, centerX - 52, centerY + 16, 16, cloud);

  for (int i = 0; i < 6; ++i) {
    const int x = centerX - 48 + i * 14;
    const int y = centerY + 40 + (i % 2) * 7;
    if (sky == Sky::Rain || sky == Sky::Storm) gfx.drawWideLine(x, y, x - 5, y + 12, 1.2f, color::kRain);
    if (sky == Sky::Snow) gfx.fillSmoothCircle(x, y + 6, 2, color::kText);
  }
  if (sky == Sky::Storm) {
    gfx.fillTriangle(centerX - 4, centerY + 34, centerX - 18, centerY + 56, centerX - 6, centerY + 54, color::kSun);
    gfx.fillTriangle(centerX - 14, centerY + 52, centerX - 2, centerY + 50, centerX - 16, centerY + 72, color::kSun);
  }
}

}  // namespace

void drawWeatherScreen(LovyanGFX& gfx, const WeatherData& weather, const char* city) {
  const Condition condition = conditionFromCode(weather.weatherCode);
  drawSky(gfx, condition.sky, weather.isDay);
  drawHills(gfx, weather.isDay ? 0.0f : 0.55f);
  drawHeader(gfx, city, weather.valid ? condition.label : "Sem dados", color::kText);

  char text[48];
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().huge);
  if (weather.valid) {
    snprintf(text, sizeof(text), "%.0f°", weather.temperature);
  } else {
    snprintf(text, sizeof(text), "--°");
  }
  gfx.drawString(text, kMargin - 4, kHeaderBottom + 6);

  if (weather.valid) {
    gfx.setFont(&fonts().medium);
    snprintf(text, sizeof(text), "Máx %.0f°   Mín %.0f°", weather.tempMax, weather.tempMin);
    gfx.drawString(text, kMargin, 118);
  }

  const Rect footer = drawFooterPanel(gfx);
  const bool rain = weather.rainProbability >= 30;
  if (!weather.valid) {
    snprintf(text, sizeof(text), "Aguardando previsão");
  } else if (rain) {
    snprintf(text, sizeof(text), "%u%% de chance de chuva", weather.rainProbability);
  } else {
    snprintf(text, sizeof(text), "Sem chuva nas próximas 12 h");
  }
  const int middleY = footer.y + footer.h / 2;
  gfx.fillSmoothCircle(footer.x + 18, middleY, 4, rain ? color::kRain : color::kSun);
  gfx.setFont(&fonts().medium);
  gfx.setTextDatum(lgfx::middle_left);
  gfx.drawString(text, footer.x + 32, middleY);
}

}  // namespace ui
