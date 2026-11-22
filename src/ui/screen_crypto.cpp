#include <math.h>
#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr float kBigMovePercent = 5.0f;
constexpr uint32_t kGold = 0xF5B93Au;
constexpr uint32_t kGoldDark = 0xC98A1Bu;

struct Mood {
  const char* label;
  uint32_t tint;
  uint32_t skyTop;
  uint32_t skyBottom;
};

Mood moodFor(const CoinQuote& coin) {
  if (!coin.valid) return {"Sem dados", color::kTextMuted, 0x050818u, 0x14245Au};
  if (coin.pctChange24h >= kBigMovePercent) return {"Foguete!", color::kPositive, 0x050818u, 0x1B2A7Au};
  if (coin.pctChange24h <= -kBigMovePercent) return {"Queda!", color::kNegative, 0x1A0508u, 0x5A1E14u};
  return {"Tranquilo.", color::kCalm, 0x050818u, 0x14245Au};
}

void drawCoin(LovyanGFX& gfx, int x, int y, int radius, const char* symbol) {
  drawGlow(gfx, x, y, radius + 16, kGold);
  gfx.fillSmoothCircle(x, y, radius, kGoldDark);
  gfx.fillSmoothCircle(x, y, radius - 3, kGold);
  gfx.drawCircle(x, y, radius - 7, kGoldDark);
  const char initial[2] = {symbol[0], '\0'};
  gfx.setFont(&fonts().large);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(kGoldDark);
  gfx.drawString(initial, x, y + 1);
}

}  // namespace

void drawCryptoScreen(LovyanGFX& gfx, const CryptoData& crypto) {
  const CoinQuote& main = crypto.coins[0];
  const Mood mood = moodFor(main);
  const int footerTop = gfx.height() - kFooterBottomGap - kFooterTallHeight;

  fillBackground(gfx, mood.skyTop, mood.skyBottom);
  drawStars(gfx, 70);
  // Curvatura de um planeta no horizonte
  gfx.fillEllipse(gfx.width() / 2, footerTop + 150, gfx.width(), 160, blend(mood.skyBottom, color::kText, 0.45f));
  gfx.fillEllipse(gfx.width() / 2, footerTop + 153, gfx.width(), 160, blend(mood.skyTop, mood.skyBottom, 0.4f));
  drawCoin(gfx, gfx.width() - 60, 136, 26, main.symbol);

  drawHeader(gfx, main.symbol, mood.label, mood.tint);

  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kTextMuted);
  gfx.setFont(&fonts().medium);
  gfx.drawString(crypto.currencyLabel, kMargin, kHeaderBottom + 8);
  const int priceX = kMargin + gfx.textWidth(crypto.currencyLabel) + 6;
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().big);
  gfx.drawString(main.valid ? formatThousands(main.price) : "-", priceX, kHeaderBottom + 4);
  if (main.valid) drawChange(gfx, kMargin, 104, main.pctChange24h, 1, "em 24 h");

  FooterItem items[kMaxCoins - 1];
  char details[kMaxCoins - 1][12];
  uint8_t count = 0;
  for (uint8_t i = 1; i < crypto.count && i < kMaxCoins; ++i, ++count) {
    const CoinQuote& coin = crypto.coins[i];
    snprintf(details[count], sizeof(details[count]), "%s", coin.valid ? formatCompact(coin.price) : "");
    items[count] = changeItem(coin.symbol, details[count], coin.pctChange24h, coin.valid);
  }
  drawFooterColumns(gfx, items, count);
}

}  // namespace ui
