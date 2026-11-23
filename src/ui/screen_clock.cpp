#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

const char* const kWeekdays[] = {"Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira",
                                 "Quinta-feira", "Sexta-feira", "Sábado"};
const char* const kMonths[] = {"janeiro", "fevereiro", "março", "abril", "maio", "junho",
                               "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"};

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

}  // namespace

void drawClockScreen(LovyanGFX& gfx, const tm& now) {
  drawSkyForHour(gfx, now.tm_hour);
  drawHeader(gfx, kWeekdays[now.tm_wday % 7], nullptr, color::kText);

  // Horas e minutos centralizados; os segundos ficam ao lado, alinhados pela base
  char text[40];
  const Rect footer = drawFooterPanel(gfx);
  const int baseline = (kHeaderBottom + footer.y) / 2 + 30;
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().large);
  snprintf(text, sizeof(text), "%02d", now.tm_sec);
  const int secondsWidth = gfx.textWidth("00") + 6;
  gfx.setFont(&fonts().huge);
  char time[8];
  snprintf(time, sizeof(time), "%02d:%02d", now.tm_hour, now.tm_min);
  const int timeX = (gfx.width() - gfx.textWidth(time) - secondsWidth) / 2;
  gfx.setTextDatum(lgfx::baseline_left);
  gfx.drawString(time, timeX, baseline);
  gfx.setFont(&fonts().large);
  gfx.setTextColor(color::kTextMuted);
  gfx.drawString(text, timeX + gfx.width() - 2 * timeX - secondsWidth + 6, baseline);

  gfx.setFont(&fonts().medium);
  gfx.setTextColor(color::kText);
  gfx.setTextDatum(lgfx::middle_center);
  snprintf(text, sizeof(text), "%d de %s de %d", now.tm_mday, kMonths[now.tm_mon % 12], now.tm_year + 1900);
  gfx.drawString(text, gfx.width() / 2, footer.y + footer.h / 2);
}

}  // namespace ui
