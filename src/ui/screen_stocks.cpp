#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr uint32_t kBuilding = 0x1A1B45u;
constexpr uint32_t kBuildingFar = 0x3A2F6Bu;
constexpr uint32_t kWindow = 0xFFD98Au;

void drawSkyline(LovyanGFX& gfx, int baseline) {
  uint32_t seed = 0x9E3779B9u;  // sequência fixa: a cidade é sempre a mesma
  for (int layer = 0; layer < 2; ++layer) {
    for (int x = -4; x < gfx.width();) {
      seed = seed * 1664525u + 1013904223u;
      const int width = 16 + (seed >> 10) % 16;
      const int height = (layer == 0 ? 26 : 12) + (seed >> 18) % 22;
      gfx.fillRect(x, baseline - height, width, height, layer == 0 ? kBuildingFar : kBuilding);
      if (layer == 1) {
        for (int wy = baseline - height + 4; wy < baseline - 4; wy += 6) {
          for (int wx = x + 3; wx < x + width - 3; wx += 5) {
            seed = seed * 1664525u + 1013904223u;
            if ((seed >> 28) < 6) gfx.fillRect(wx, wy, 2, 3, kWindow);
          }
        }
      }
      x += width + (layer == 0 ? 2 : 3);
    }
  }
}

}  // namespace

void drawStocksScreen(LovyanGFX& gfx, const StocksData& stocks) {
  const StockQuote& main = stocks.items[0];
  const uint32_t accent = main.valid ? trendColor(main.pctChange) : color::kTextMuted;
  const bool falling = main.valid && accent == color::kNegative;
  const int baseline = gfx.height() - kFooterBottomGap - kFooterTallHeight - 6;

  fillGradientBand(gfx, 0, baseline, falling ? 0x2A0A2Au : 0x0B1450u, falling ? 0xF0703Cu : 0xC878B4u);
  drawStars(gfx, 25);
  drawGlow(gfx, gfx.width() - 44, baseline - 44, 30, accent);
  gfx.fillSmoothCircle(gfx.width() - 44, baseline - 44, 12, blend(accent, color::kText, 0.7f));
  drawSkyline(gfx, baseline);
  fillGradientBand(gfx, baseline, gfx.height(), 0x15163Au, 0x0C0D24u);

  const char* headline = "Sem dados";
  if (main.valid) {
    headline = accent == color::kNeutral ? "Dia estável" : (falling ? "Dia no vermelho" : "Dia no verde");
  }
  drawHeader(gfx, main.label, headline, accent, true);

  char text[32];
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().big);
  snprintf(text, sizeof(text), "%s", main.valid ? formatDecimal(main.pctChange, 2, true) : "-,--");
  gfx.drawString(text, kMargin - 2, kHeaderBottom + 2);
  const int percentX = kMargin + gfx.textWidth(text) + 4;
  gfx.setFont(&fonts().large);
  gfx.drawString("%", percentX, kHeaderBottom + 4);

  uint8_t rising = 0;
  uint8_t valid = 0;
  for (uint8_t i = 0; i < stocks.count; ++i) {
    valid += stocks.items[i].valid;
    rising += stocks.items[i].valid && stocks.items[i].pctChange > 0;
  }
  if (main.valid) {
    gfx.setFont(&fonts().medium);
    gfx.drawString(formatThousands(main.price), kMargin, 92);
    const int unitX = kMargin + gfx.textWidth(formatThousands(main.price)) + 6;
    gfx.setTextColor(color::kTextMuted);
    gfx.drawString(main.isIndex ? "pontos" : "BRL", unitX, 92);
    const bool mostlyUp = rising * 2 >= valid;
    snprintf(text, sizeof(text), "%u de %u %s", mostlyUp ? rising : valid - rising, valid,
             mostlyUp ? "sobem" : "caem");
    gfx.setTextDatum(lgfx::top_right);
    gfx.drawString(text, gfx.width() - kMargin, 92);
  }

  FooterItem items[kMaxStocks - 1];
  uint8_t count = 0;
  for (uint8_t i = 1; i < stocks.count && i < kMaxStocks; ++i, ++count) {
    items[count] = changeItem(stocks.items[i].label, nullptr, stocks.items[i].pctChange, stocks.items[i].valid);
  }
  drawFooterColumns(gfx, items, count);
}

}  // namespace ui
