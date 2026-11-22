#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kIconWidth = 96;
constexpr int kIconHeight = 48;

struct TrendLabel {
  const char* text;
  uint32_t tint;
};

TrendLabel trendLabel(const BatteryData& battery) {
  if (!battery.valid) return {"Sem leitura", color::kTextMuted};
  switch (battery.trend) {
    case BatteryTrend::Charging:    return {"Carregando", color::kPositive};
    case BatteryTrend::Discharging: return {"Descarregando", color::kNeutral};
    case BatteryTrend::Stable:      return {battery.percent >= 98 ? "Carregada" : "Estável", color::kCalm};
    default:                        return {"Analisando", color::kTextMuted};
  }
}

uint32_t levelColor(uint8_t percent) {
  if (percent > 50) return color::kPositive;
  return percent > 20 ? color::kNeutral : color::kNegative;
}

void drawBatteryIcon(LovyanGFX& gfx, int x, int y, uint8_t percent) {
  gfx.fillSmoothRoundRect(x, y, kIconWidth, kIconHeight, 8, color::kText);
  gfx.fillSmoothRoundRect(x + 3, y + 3, kIconWidth - 6, kIconHeight - 6, 6, color::kPanel);
  gfx.fillSmoothRoundRect(x + kIconWidth + 1, y + kIconHeight / 2 - 8, 6, 16, 2, color::kText);
  const int fill = (kIconWidth - 12) * percent / 100;
  if (fill > 4) gfx.fillSmoothRoundRect(x + 6, y + 6, fill, kIconHeight - 12, 4, levelColor(percent));
}

const char* durationLabel(int32_t minutes) {
  static char buffer[16];
  if (minutes < 0) return "calculando";
  if (minutes >= 60) snprintf(buffer, sizeof(buffer), "%ld h %02ld", static_cast<long>(minutes / 60), static_cast<long>(minutes % 60));
  else snprintf(buffer, sizeof(buffer), "%ld min", static_cast<long>(minutes));
  return buffer;
}

}  // namespace

void drawBatteryScreen(LovyanGFX& gfx, const BatteryData& battery) {
  const TrendLabel trend = trendLabel(battery);
  fillBackground(gfx, 0x06141Fu, 0x123A3Au);
  drawHeader(gfx, "Bateria", trend.text, trend.tint, true);

  char text[24];
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().huge);
  snprintf(text, sizeof(text), battery.valid ? "%u" : "--", battery.percent);
  gfx.drawString(text, kMargin - 4, kHeaderBottom + 6);
  const int percentX = kMargin + gfx.textWidth(text) + 2;
  gfx.setFont(&fonts().large);
  gfx.drawString("%", percentX, kHeaderBottom + 10);
  drawBatteryIcon(gfx, gfx.width() - kMargin - kIconWidth - 8, kHeaderBottom + 16, battery.valid ? battery.percent : 0);
  if (!battery.valid) return;

  const bool charging = battery.trend == BatteryTrend::Charging;
  FooterItem items[3] = {{"Tensão", nullptr, "", color::kText},
                         {charging ? "Até 100%" : "Restante", nullptr, "", color::kText},
                         {"Ritmo", nullptr, "", trend.tint}};
  snprintf(items[0].value, sizeof(items[0].value), "%s V", formatDecimal(battery.voltage, 2));
  if (battery.trend == BatteryTrend::Stable) {
    snprintf(items[1].value, sizeof(items[1].value), "--");
    snprintf(items[2].value, sizeof(items[2].value), "0 %%/h");
  } else {
    snprintf(items[1].value, sizeof(items[1].value), "%s", durationLabel(battery.minutesRemaining));
    if (battery.trend == BatteryTrend::Unknown) snprintf(items[2].value, sizeof(items[2].value), "calculando");
    else snprintf(items[2].value, sizeof(items[2].value), "%s %%/h", formatDecimal(battery.ratePerHour, 0, true));
  }
  drawFooterColumns(gfx, items, 3);
}

}  // namespace ui
