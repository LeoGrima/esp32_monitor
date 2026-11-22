#include <math.h>
#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kBarWidth = 24;
constexpr int kBarMaxHeight = 38;
constexpr int kBarMinHeight = 12;
constexpr uint32_t kBarColor = 0x3E4F9Cu;
constexpr uint32_t kGround = 0x0A0D1Du;

void drawWeekBars(LovyanGFX& gfx, const CurrencyData& currency, int baseline, uint32_t accent) {
  if (currency.usdWeekCount < 2) return;

  float low = currency.usdWeek[0];
  float high = low;
  for (uint8_t i = 1; i < currency.usdWeekCount; ++i) {
    low = fminf(low, currency.usdWeek[i]);
    high = fmaxf(high, currency.usdWeek[i]);
  }
  const float range = fmaxf(high - low, 0.0001f);
  const int step = (gfx.width() - 2 * kMargin - kBarWidth) / (kWeekHistorySize - 1);
  int x = kMargin + (kWeekHistorySize - currency.usdWeekCount) * step;

  for (uint8_t i = 0; i < currency.usdWeekCount; ++i, x += step) {
    const float ratio = (currency.usdWeek[i] - low) / range;
    const int height = kBarMinHeight + static_cast<int>(ratio * (kBarMaxHeight - kBarMinHeight));
    const bool latest = i == currency.usdWeekCount - 1;
    // A base arredondada fica escondida pelo chão desenhado em seguida
    gfx.fillSmoothRoundRect(x, baseline - height, kBarWidth, height + kBarWidth, kBarWidth / 2,
                            latest ? accent : kBarColor);
  }
}

const char* headline(const CurrencyQuote& usd) {
  if (!usd.valid) return "Sem dados";
  const uint32_t tint = trendColor(usd.pctChange);
  if (tint == color::kNeutral) return "Dólar estável hoje";
  return tint == color::kPositive ? "Dólar subiu hoje" : "Dólar caiu hoje";
}

}  // namespace

void drawCurrencyScreen(LovyanGFX& gfx, const CurrencyData& currency) {
  const CurrencyQuote& usd = currency.usd;
  const uint32_t accent = usd.valid ? trendColor(usd.pctChange) : color::kTextMuted;
  const int horizon = gfx.height() - kFooterBottomGap - kFooterHeight - 22;

  // Entardecer: céu noturno que clareia até o horizonte
  fillGradientBand(gfx, 0, horizon * 3 / 5, color::kNightTop, 0x26306Eu);
  fillGradientBand(gfx, horizon * 3 / 5, horizon, 0x26306Eu, 0xF0A458u);
  drawStars(gfx, 30);
  drawGlow(gfx, gfx.width() * 2 / 3, horizon, 42, 0xFFE9B0u);
  drawWeekBars(gfx, currency, horizon, accent);
  fillGradientBand(gfx, horizon, gfx.height(), kGround, 0x1B1A3Au);

  drawHeader(gfx, "USD", headline(usd), accent);

  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kTextMuted);
  gfx.setFont(&fonts().medium);
  gfx.drawString("R$", kMargin, kHeaderBottom + 6);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().huge);
  gfx.drawString(usd.valid ? formatDecimal(usd.bid, 2) : "-,--", kMargin + 28, kHeaderBottom);
  if (usd.valid) drawChange(gfx, kMargin, 122, usd.pctChange, 1, "hoje");

  char text[32];
  gfx.setFont(&fonts().small);
  gfx.setTextColor(color::kTextMuted);
  gfx.setTextDatum(lgfx::top_left);
  gfx.drawString("SEMANA", kMargin, horizon + 5);
  if (currency.updatedAt[0] != '\0') {
    snprintf(text, sizeof(text), "PTAX %s", currency.updatedAt);
    gfx.setTextDatum(lgfx::top_right);
    gfx.drawString(text, gfx.width() - kMargin, horizon + 5);
  }

  const Rect footer = drawFooterPanel(gfx);
  const int middleY = footer.y + footer.h / 2;
  gfx.setTextDatum(lgfx::middle_left);
  gfx.drawString("EUR", footer.x + 16, middleY);
  gfx.setFont(&fonts().medium);
  gfx.setTextColor(color::kText);
  gfx.drawString(currency.eur.valid ? formatDecimal(currency.eur.bid, 2) : "-,--", footer.x + 50, middleY);
  if (currency.eur.valid) drawChange(gfx, footer.x + footer.w - 90, middleY, currency.eur.pctChange, 1, nullptr);
}

}  // namespace ui
