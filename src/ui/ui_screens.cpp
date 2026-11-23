#include "ui_screens.h"

#include "fonts/ui_fonts.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {

void drawScreen(LovyanGFX& gfx, Screen screen, const DashboardState& state) {
  switch (screen) {
    case Screen::Weather:  drawWeatherScreen(gfx, state.weather, state.city); break;
    case Screen::Currency: drawCurrencyScreen(gfx, state.currency); break;
    case Screen::Crypto:   drawCryptoScreen(gfx, state.crypto); break;
    case Screen::Stocks:   drawStocksScreen(gfx, state.stocks); break;
    case Screen::News:
      drawHeadlineScreen(gfx, state.news, {"MANCHETE", state.newsSource, "Aguardando notícias", 0xE5383Bu, 0x3A0E14u, 0x140A0Du});
      break;
    case Screen::Video:    drawVideoScreen(gfx, state.video, state.nowEpoch); break;
    case Screen::Clock:    drawClockScreen(gfx, state.now); break;
    case Screen::WorldClock: drawWorldClockScreen(gfx, state); break;
  }
}

void drawMessage(LovyanGFX& gfx, const char* title, const char* detail) {
  fillBackground(gfx, color::kNightTop, color::kNightBottom);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().large);
  gfx.drawString(title, gfx.width() / 2, gfx.height() / 2 - 16);
  gfx.setTextColor(color::kTextMuted);
  gfx.setFont(&fonts().medium);
  gfx.drawString(detail, gfx.width() / 2, gfx.height() / 2 + 20);
}

}  // namespace ui
