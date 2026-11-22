#include <math.h>
#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr float kDegToRad = 0.0174533f;
constexpr uint32_t kScope = 0x34D399u;
constexpr uint32_t kScopeDim = 0x14532Du;
constexpr uint32_t kNearest = 0xFBBF24u;
constexpr int kScopeRadius = 88;

void drawScope(LovyanGFX& gfx, int cx, int cy, const RadarData& radar) {
  gfx.fillSmoothCircle(cx, cy, kScopeRadius, 0x06180Fu);
  for (int ring = 1; ring <= 3; ++ring) gfx.drawCircle(cx, cy, kScopeRadius * ring / 3, kScopeDim);
  gfx.drawFastHLine(cx - kScopeRadius, cy, 2 * kScopeRadius, kScopeDim);
  gfx.drawFastVLine(cx, cy - kScopeRadius, 2 * kScopeRadius, kScopeDim);
  gfx.drawCircle(cx, cy, kScopeRadius, kScope);
  gfx.fillSmoothCircle(cx, cy, 3, color::kText);

  for (int i = radar.count - 1; i >= 0; --i) {
    const Aircraft& aircraft = radar.items[i];
    const float distance = fminf(aircraft.distanceKm / radar.radiusKm, 1.0f) * (kScopeRadius - 5);
    const float angle = aircraft.bearing * kDegToRad;
    gfx.fillSmoothCircle(cx + static_cast<int>(sinf(angle) * distance), cy - static_cast<int>(cosf(angle) * distance),
                         i == 0 ? 4 : 3, i == 0 ? kNearest : kScope);
  }
}

}  // namespace

void drawRadarScreen(LovyanGFX& gfx, const RadarData& radar) {
  fillBackground(gfx, 0x03110Au, 0x0A2A1Au);
  char text[40];
  snprintf(text, sizeof(text), "%u no raio de %.0f km", radar.total, radar.radiusKm);
  drawHeader(gfx, "Radar", radar.valid ? text : "Sem dados", kScope);

  const int cy = kHeaderBottom + kScopeRadius + 8;
  drawScope(gfx, kMargin + kScopeRadius, cy, radar);
  if (!radar.valid) return;

  const int panelX = 2 * kMargin + 2 * kScopeRadius + 6;
  gfx.setTextDatum(lgfx::top_left);
  if (radar.count == 0) {
    gfx.setFont(&fonts().medium);
    gfx.setTextColor(color::kTextMuted);
    gfx.drawString("Céu vazio", panelX, cy - 10);
    return;
  }

  const Aircraft& nearest = radar.items[0];
  gfx.setFont(&fonts().small);
  gfx.setTextColor(kNearest);
  gfx.drawString("MAIS PERTO", panelX, kHeaderBottom + 16);
  gfx.setFont(&fonts().title);
  gfx.setTextColor(color::kText);
  gfx.drawString(nearest.flight[0] != '\0' ? nearest.flight : "Sem voo", panelX, kHeaderBottom + 34);
  gfx.setFont(&fonts().medium);
  gfx.setTextColor(color::kTextMuted);
  gfx.drawString(nearest.type, panelX, kHeaderBottom + 66);
  snprintf(text, sizeof(text), "a %s km", formatDecimal(nearest.distanceKm, 1));
  gfx.drawString(text, panelX, kHeaderBottom + 92);
  if (nearest.altitudeM > 0) {
    snprintf(text, sizeof(text), "%s m", formatThousands(nearest.altitudeM));
  } else {
    snprintf(text, sizeof(text), "no solo");
  }
  gfx.drawString(text, panelX, kHeaderBottom + 116);
}

}  // namespace ui
