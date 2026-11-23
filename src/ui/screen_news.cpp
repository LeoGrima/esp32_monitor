#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kBadgeHeight = 18;
constexpr int kHeadlineLineHeight = 30;
constexpr int kHeadlineMaxLines = 3;

}  // namespace

void drawHeadlineScreen(LovyanGFX& gfx, const NewsData& news, const HeadlineStyle& style) {
  const uint32_t kAccent = style.accent;
  fillBackground(gfx, style.backgroundTop, style.backgroundBottom);
  const int contentWidth = gfx.width() - 2 * kMargin;

  gfx.setFont(&fonts().small);
  const int badgeWidth = gfx.textWidth(style.badge) + 20;
  gfx.fillSmoothRoundRect(kMargin, kMargin - 2, badgeWidth, kBadgeHeight, kBadgeHeight / 2, kAccent);
  gfx.setTextColor(color::kText);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.drawString(style.badge, kMargin + badgeWidth / 2, kMargin - 2 + kBadgeHeight / 2);
  gfx.setFont(&fonts().medium);
  gfx.setTextColor(kAccent);
  gfx.setTextDatum(lgfx::middle_left);
  gfx.drawString(style.source, kMargin + badgeWidth + 8, kMargin - 2 + kBadgeHeight / 2);

  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().title);
  int y = drawWrappedText(gfx, news.valid ? news.headlines[0] : style.emptyMessage, kMargin,
                          kHeaderBottom + 8, contentWidth, kHeadlineLineHeight, kHeadlineMaxLines);
  if (!news.valid) return;

  gfx.setFont(&fonts().medium);
  y += 8;
  gfx.setTextColor(color::kTextMuted);
  for (uint8_t i = 1; i < kNewsVisible && news.headlines[i][0] != '\0'; ++i, y += 22) {
    if (y + 20 > gfx.height() - 20) break;
    gfx.fillSmoothCircle(kMargin + 3, y + 9, 2, kAccent);
    drawFittedText(gfx, news.headlines[i], kMargin + 14, y, contentWidth - 14);
  }
  drawProgressBar(gfx, gfx.height() - 14, news.index, news.total, color::kText);
}

}  // namespace ui
