#include <math.h>
#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr float kTwoPi = 6.2831853f;
constexpr uint32_t kFace = 0x0E1A33u;
constexpr uint32_t kSecondHand = 0xFF5A4Fu;

const char* const kWeekdays[] = {"Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira",
                                 "Quinta-feira", "Sexta-feira", "Sábado"};
const char* const kMonths[] = {"janeiro", "fevereiro", "março", "abril", "maio", "junho",
                               "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"};
const char* const kSeasons[] = {"VERÃO", "OUTONO", "INVERNO", "PRIMAVERA"};

// Estações astronômicas aproximadas; índice 0 = verão no hemisfério sul
const char* seasonName(const tm& now, bool southernHemisphere) {
  const int dayCode = (now.tm_mon + 1) * 100 + now.tm_mday;
  int index = 0;
  if (dayCode >= 320 && dayCode < 621) index = 1;
  else if (dayCode >= 621 && dayCode < 922) index = 2;
  else if (dayCode >= 922 && dayCode < 1221) index = 3;
  return kSeasons[southernHemisphere ? index : (index + 2) % 4];
}

void drawSkyForHour(LovyanGFX& gfx, int hour) {
  if (hour >= 6 && hour < 8) {
    fillBackground(gfx, 0x35508Fu, 0xF2A66Bu);
  } else if (hour >= 8 && hour < 17) {
    fillBackground(gfx, color::kDayTop, color::kDayBottom);
  } else if (hour >= 17 && hour < 19) {
    fillBackground(gfx, 0x3B2A6Bu, 0xF08A5Du);
  } else {
    fillBackground(gfx, color::kNightTop, color::kNightBottom);
    drawStars(gfx, 45);
  }
}

void drawHand(LovyanGFX& gfx, int cx, int cy, float turns, int length, float thickness, uint32_t tint) {
  const float angle = turns * kTwoPi;
  gfx.drawWideLine(cx - static_cast<int>(sinf(angle) * length * 0.15f),
                   cy + static_cast<int>(cosf(angle) * length * 0.15f),
                   cx + static_cast<int>(sinf(angle) * length),
                   cy - static_cast<int>(cosf(angle) * length), thickness, tint);
}

void drawFace(LovyanGFX& gfx, int cx, int cy, int radius, const tm& now) {
  gfx.fillSmoothCircle(cx, cy, radius, color::kText);
  gfx.fillSmoothCircle(cx, cy, radius - 3, kFace);

  for (int i = 0; i < 12; ++i) {
    const float angle = i * kTwoPi / 12;
    const bool quarter = i % 3 == 0;
    const int inner = radius - (quarter ? 13 : 9);
    const int outer = radius - 6;
    gfx.drawWideLine(cx + static_cast<int>(sinf(angle) * inner), cy - static_cast<int>(cosf(angle) * inner),
                     cx + static_cast<int>(sinf(angle) * outer), cy - static_cast<int>(cosf(angle) * outer),
                     quarter ? 1.4f : 0.8f, quarter ? color::kText : color::kTextMuted);
  }

  gfx.setFont(&fonts().small);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(color::kText);
  for (int hour = 3; hour <= 12; hour += 3) {
    char label[3];
    snprintf(label, sizeof(label), "%d", hour);
    const float angle = hour * kTwoPi / 12;
    gfx.drawString(label, cx + static_cast<int>(sinf(angle) * (radius - 22)),
                   cy - static_cast<int>(cosf(angle) * (radius - 22)));
  }

  const float seconds = now.tm_sec / 60.0f;
  const float minutes = (now.tm_min + seconds) / 60.0f;
  const float hours = (now.tm_hour % 12 + minutes) / 12.0f;
  drawHand(gfx, cx, cy, hours, radius * 50 / 100, 2.6f, color::kText);
  drawHand(gfx, cx, cy, minutes, radius * 74 / 100, 1.8f, color::kText);
  drawHand(gfx, cx, cy, seconds, radius * 80 / 100, 0.8f, kSecondHand);
  gfx.fillSmoothCircle(cx, cy, 4, kSecondHand);
}

}  // namespace

void drawClockScreen(LovyanGFX& gfx, const tm& now, bool southernHemisphere) {
  drawSkyForHour(gfx, now.tm_hour);
  drawHeader(gfx, kWeekdays[now.tm_wday % 7], seasonName(now, southernHemisphere), color::kNeutral, true);

  const Rect footer = drawFooterPanel(gfx);
  const int radius = (footer.y - kHeaderBottom - 6) / 2;
  drawFace(gfx, gfx.width() / 2, kHeaderBottom + radius, radius, now);

  char text[40];
  const int middleY = footer.y + footer.h / 2;
  gfx.setFont(&fonts().medium);
  gfx.setTextColor(color::kText);
  gfx.setTextDatum(lgfx::middle_left);
  snprintf(text, sizeof(text), "%d de %s de %d", now.tm_mday, kMonths[now.tm_mon % 12], now.tm_year + 1900);
  gfx.drawString(text, footer.x + 16, middleY);
  gfx.setTextDatum(lgfx::middle_right);
  snprintf(text, sizeof(text), "%02d:%02d", now.tm_hour, now.tm_min);
  gfx.drawString(text, footer.x + footer.w - 16, middleY);
}

}  // namespace ui
