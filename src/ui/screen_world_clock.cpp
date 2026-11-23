#include <stdio.h>
#include <stdlib.h>

#include "../model/world_time.h"
#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr long long kSecondsPerDay = 86400;
constexpr int kPanelHeight = 72;
constexpr int kGlobeRadius = 54;
constexpr uint32_t kOcean = 0x1B4F9Cu;
constexpr uint32_t kGrid = 0x6FA8FFu;

struct Activity {
  const char* label;
  uint32_t tint;
};

Activity activityAt(int hour, int weekday) {
  const bool weekend = weekday == 0 || weekday == 6;
  if (hour >= 22 || hour < 7) return {"DORMINDO", color::kCalm};
  if (!weekend && hour >= 9 && hour < 18) return {"NO EXPEDIENTE", color::kNeutral};
  return {"FORA DO EXPEDIENTE", color::kTextMuted};
}

// Globo em linhas: meridianos (elipses) e paralelos (cordas)
void drawGlobe(LovyanGFX& gfx, int cx, int cy) {
  drawGlow(gfx, cx, cy, kGlobeRadius + 14, kGrid);
  gfx.fillSmoothCircle(cx, cy, kGlobeRadius, kOcean);
  for (int i = 1; i <= 2; ++i) gfx.drawEllipse(cx, cy, kGlobeRadius * i / 3, kGlobeRadius, kGrid);
  gfx.drawFastVLine(cx, cy - kGlobeRadius, 2 * kGlobeRadius, kGrid);
  for (int i = -2; i <= 2; ++i) {
    const int dy = kGlobeRadius * i / 3;
    int half = 0;
    while ((half + 1) * (half + 1) + dy * dy <= kGlobeRadius * kGlobeRadius) ++half;
    gfx.drawFastHLine(cx - half, cy + dy, 2 * half, kGrid);
  }
  gfx.drawCircle(cx, cy, kGlobeRadius, color::kText);
}

// "+3h", "-2h", "+8h30" ou "mesma hora"
const char* differenceLabel(int32_t seconds) {
  static char buffer[16];
  const int minutes = abs(seconds) / 60;
  if (minutes == 0) return "mesma hora";
  if (minutes % 60 == 0) snprintf(buffer, sizeof(buffer), "%c%dh", seconds > 0 ? '+' : '-', minutes / 60);
  else snprintf(buffer, sizeof(buffer), "%c%dh%02d", seconds > 0 ? '+' : '-', minutes / 60, minutes % 60);
  return buffer;
}

}  // namespace

void drawWorldClockScreen(LovyanGFX& gfx, const DashboardState& state) {
  fillBackground(gfx, 0x050A24u, 0x16307Au);
  drawStars(gfx, 60);

  // Hora local expressa como se fosse UTC: a diferença para o epoch real é o fuso daqui
  const tm& now = state.now;
  const long long localSeconds = world_time::daysFromCivil(now.tm_year + 1900, now.tm_mon + 1, now.tm_mday) * kSecondsPerDay +
                                 now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
  const int32_t localOffset = static_cast<int32_t>(localSeconds - state.nowEpoch);

  char text[32];
  const int panelTop = gfx.height() - kFooterBottomGap - kPanelHeight;
  drawGlobe(gfx, gfx.width() - kMargin - kGlobeRadius - 14, (kHeaderBottom + panelTop) / 2 + 4);

  gfx.setFont(&fonts().small);
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.drawString("AQUI", kMargin, kMargin);
  gfx.setFont(&fonts().big);
  snprintf(text, sizeof(text), "%02d:%02d", now.tm_hour, now.tm_min);
  gfx.drawString(text, kMargin - 2, kHeaderBottom + 2);

  const Rect panel = {kPanelInset, panelTop, gfx.width() - 2 * kPanelInset, kPanelHeight};
  fillPanel(gfx, panel, 14);
  if (state.cityCount == 0) {
    gfx.setFont(&fonts().medium);
    gfx.setTextDatum(lgfx::middle_center);
    gfx.setTextColor(color::kTextMuted);
    gfx.drawString("Nenhuma cidade configurada", gfx.width() / 2, panel.y + panel.h / 2);
    return;
  }

  const int columnWidth = panel.w / state.cityCount;
  for (uint8_t i = 0; i < state.cityCount; ++i) {
    const CityZone& city = state.cities[i];
    const int32_t offset = world_time::utcOffsetSeconds(city, state.nowEpoch);
    const long long citySeconds = state.nowEpoch + offset;
    const int secondsOfDay = static_cast<int>(((citySeconds % kSecondsPerDay) + kSecondsPerDay) % kSecondsPerDay);
    const long long cityDay = (citySeconds - secondsOfDay) / kSecondsPerDay;
    const long long dayShift = cityDay - (localSeconds - localSeconds % kSecondsPerDay) / kSecondsPerDay;
    const int hour = secondsOfDay / 3600;

    if (i == 0) {
      const Activity activity = activityAt(hour, static_cast<int>(((cityDay % 7) + 7 + 4) % 7));
      snprintf(text, sizeof(text), "%s: %s", city.name, activity.label);
      gfx.setFont(&fonts().small);
      gfx.setTextDatum(lgfx::top_right);
      gfx.setTextColor(activity.tint);
      gfx.drawString(text, gfx.width() - kMargin, kMargin);
    }

    const int x = panel.x + i * columnWidth + 9;
    if (i > 0) gfx.fillRectAlpha(x - 9, panel.y + 8, 1, panel.h - 16, 60, color::kText);
    gfx.setTextDatum(lgfx::top_left);
    gfx.setFont(&fonts().medium);
    gfx.setTextColor(i == 0 ? color::kText : color::kTextMuted);
    drawFittedText(gfx, city.name, x, panel.y + 6, columnWidth - 11);
    gfx.setFont(&fonts().title);
    snprintf(text, sizeof(text), "%02d:%02d", hour, secondsOfDay % 3600 / 60);
    gfx.drawString(text, x, panel.y + 24);
    gfx.setFont(&fonts().small);
    gfx.setTextColor(color::kTextMuted);
    snprintf(text, sizeof(text), "%s%s", dayShift > 0 ? "amanhã " : (dayShift < 0 ? "ontem " : ""),
             differenceLabel(offset - localOffset));
    gfx.drawString(text, x, panel.y + 53);
  }
}

}  // namespace ui
