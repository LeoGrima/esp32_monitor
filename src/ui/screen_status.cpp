#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kRowHeight = 40;
constexpr int kRowGap = 6;

uint32_t levelColor(const StatusItem& item) {
  if (!item.valid) return color::kTextMuted;
  switch (item.level) {
    case HealthLevel::Ok:      return color::kPositive;
    case HealthLevel::Warning: return color::kNeutral;
    default:                   return color::kNegative;
  }
}

}  // namespace

void drawStatusScreen(LovyanGFX& gfx, const StatusData& status, const char* title) {
  uint8_t problems = 0;
  uint8_t known = 0;
  for (uint8_t i = 0; i < status.count; ++i) {
    known += status.items[i].valid;
    problems += status.items[i].valid && status.items[i].level != HealthLevel::Ok;
  }
  const bool healthy = problems == 0;
  fillBackground(gfx, healthy ? 0x08201Au : 0x2A0E12u, 0x0B1020u);

  char summary[24];
  snprintf(summary, sizeof(summary), "%u com problema", problems);
  drawHeader(gfx, title, known == 0 ? "Sem dados" : (healthy ? "Tudo no ar" : summary),
             known == 0 ? color::kTextMuted : (healthy ? color::kPositive : color::kNegative), true);

  int y = kHeaderBottom + 6;
  for (uint8_t i = 0; i < status.count; ++i, y += kRowHeight + kRowGap) {
    const StatusItem& item = status.items[i];
    const uint32_t tint = levelColor(item);
    fillPanel(gfx, {kPanelInset, y, gfx.width() - 2 * kPanelInset, kRowHeight}, 12);
    gfx.fillSmoothCircle(kPanelInset + 18, y + kRowHeight / 2, 5, tint);
    gfx.setFont(&fonts().medium);
    gfx.setTextDatum(lgfx::middle_left);
    gfx.setTextColor(color::kText);
    gfx.drawString(item.label, kPanelInset + 34, y + kRowHeight / 2);
    gfx.setTextDatum(lgfx::middle_right);
    gfx.setTextColor(tint);
    gfx.drawString(item.valid ? item.detail : "--", gfx.width() - kPanelInset - 14, y + kRowHeight / 2);
  }
}

}  // namespace ui
