#include "ui_widgets.h"

#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../model/dashboard_data.h"
#include "fonts/ui_fonts.h"
#include "ui_theme.h"

namespace ui {
namespace {
constexpr float kStableThreshold = 0.05f;  // variação (%) tratada como estável
}

uint32_t blend(uint32_t from, uint32_t to, float ratio) {
  uint32_t result = 0;
  for (int shift = 0; shift <= 16; shift += 8) {
    const float a = (from >> shift) & 0xFF;
    const float b = (to >> shift) & 0xFF;
    result |= static_cast<uint32_t>(a + (b - a) * ratio + 0.5f) << shift;
  }
  return result;
}

void fillGradientBand(LovyanGFX& gfx, int top, int bottom, uint32_t topColor, uint32_t bottomColor) {
  const int span = bottom - top - 1;
  for (int y = top; y < bottom; ++y) {
    const float ratio = span > 0 ? static_cast<float>(y - top) / span : 0;
    gfx.drawFastHLine(0, y, gfx.width(), blend(topColor, bottomColor, ratio));
  }
}

void fillBackground(LovyanGFX& gfx, uint32_t topColor, uint32_t bottomColor) {
  fillGradientBand(gfx, 0, gfx.height(), topColor, bottomColor);
}

void drawStars(LovyanGFX& gfx, int count) {
  uint32_t seed = 0x2545F491u;  // sequência fixa: as estrelas não mudam de lugar
  for (int i = 0; i < count; ++i) {
    seed = seed * 1664525u + 1013904223u;
    const int x = (seed >> 8) % gfx.width();
    const int y = (seed >> 20) % (gfx.height() * 2 / 3);
    const int size = 1 + (i % 6 == 0);
    gfx.fillRectAlpha(x, y, size, size, 90 + (seed >> 26) * 2, color::kText);
  }
}

void drawGlow(LovyanGFX& gfx, int x, int y, int radius, uint32_t tint) {
  constexpr int kRings = 14;
  for (int ring = kRings; ring >= 1; --ring) {
    const int r = radius * ring / kRings;
    const uint32_t under = gfx.readPixelRGB(x, std::clamp(y - r + 1, 0, static_cast<int>(gfx.height()) - 1)).RGB888();
    gfx.fillSmoothCircle(x, y, r, blend(under, tint, 0.09f));
  }
}

void drawHills(LovyanGFX& gfx, float darkness) {
  const int width = gfx.width();
  const int height = gfx.height();
  gfx.fillEllipse(width * 3 / 4, height + 30, width * 3 / 5, 80, blend(0x3F9B4Bu, 0x000000u, darkness));
  gfx.fillEllipse(width / 5, height + 36, width / 2, 74, blend(0x2F7D3Bu, 0x000000u, darkness));
}

void drawHeader(LovyanGFX& gfx, const char* title, const char* status, uint32_t statusColor, bool statusDot) {
  gfx.setFont(&fonts().medium);
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.drawString(title, kMargin, kMargin - 3);

  if (status == nullptr || status[0] == '\0') return;
  gfx.setFont(&fonts().small);
  gfx.setTextDatum(lgfx::top_right);
  gfx.setTextColor(statusColor);
  gfx.drawString(status, gfx.width() - kMargin, kMargin);
  if (statusDot) {
    gfx.fillSmoothCircle(gfx.width() - kMargin - gfx.textWidth(status) - 9, kMargin + 6, 3, statusColor);
  }
}

void fillPanel(LovyanGFX& gfx, const Rect& rect, int radius) {
  for (int row = 0; row < rect.h; ++row) {
    const float dy = radius - std::min(row, rect.h - 1 - row) - 0.5f;
    const int inset = dy > 0 ? static_cast<int>(radius - sqrtf(radius * radius - dy * dy) + 0.5f) : 0;
    gfx.fillRectAlpha(rect.x + inset, rect.y + row, rect.w - 2 * inset, 1, kPanelAlpha, color::kPanel);
  }
}

Rect drawFooterPanel(LovyanGFX& gfx) {
  const Rect rect = {kPanelInset, gfx.height() - kFooterBottomGap - kFooterHeight,
                     gfx.width() - 2 * kPanelInset, kFooterHeight};
  fillPanel(gfx, rect, rect.h / 2);
  return rect;
}

void drawChange(LovyanGFX& gfx, int x, int y, float change, uint8_t decimals, const char* suffix) {
  char text[32];
  const uint32_t tint = trendColor(change);
  snprintf(text, sizeof(text), "%s %%", formatDecimal(change, decimals, true));
  drawTrendArrow(gfx, x, y, change, tint);
  gfx.setFont(&fonts().medium);
  gfx.setTextDatum(lgfx::middle_left);
  gfx.setTextColor(tint);
  gfx.drawString(text, x + 14, y);
  if (suffix != nullptr) {
    gfx.setTextColor(color::kTextMuted);
    gfx.drawString(suffix, x + 20 + gfx.textWidth(text), y);
  }
}

void drawFooterColumns(LovyanGFX& gfx, const FooterItem* items, uint8_t count) {
  const Rect rect = {kPanelInset, gfx.height() - kFooterBottomGap - kFooterTallHeight,
                     gfx.width() - 2 * kPanelInset, kFooterTallHeight};
  fillPanel(gfx, rect, 14);
  if (count == 0) return;

  const int columnWidth = rect.w / count;
  for (uint8_t i = 0; i < count; ++i) {
    const int x = rect.x + i * columnWidth + 12;
    if (i > 0) gfx.fillRectAlpha(x - 12, rect.y + 6, 1, rect.h - 12, 60, color::kText);

    gfx.setFont(&fonts().medium);
    gfx.setTextDatum(lgfx::top_left);
    gfx.setTextColor(color::kText);
    gfx.drawString(items[i].label, x, rect.y + 5);
    if (items[i].detail != nullptr) {
      const int detailX = x + gfx.textWidth(items[i].label) + 6;
      gfx.setFont(&fonts().small);
      gfx.setTextColor(color::kTextMuted);
      gfx.drawString(items[i].detail, detailX, rect.y + 9);
    }

    gfx.setFont(&fonts().medium);
    gfx.setTextColor(items[i].tint);
    gfx.drawString(items[i].value, x, rect.y + 24);
  }
}

FooterItem changeItem(const char* label, const char* detail, float change, bool valid) {
  FooterItem item = {label, detail, "--", color::kTextMuted};
  if (valid) {
    snprintf(item.value, sizeof(item.value), "%s %%", formatDecimal(change, 1, true));
    item.tint = trendColor(change);
  }
  return item;
}

void drawProgressBar(LovyanGFX& gfx, int y, uint8_t index, uint8_t total, uint32_t tint) {
  const int width = gfx.width() - 2 * kMargin;
  const int filled = width * (index + 1) / (total ? total : 1);
  gfx.fillSmoothRoundRect(kMargin, y, width, 4, 2, blend(color::kPanel, color::kText, 0.25f));
  gfx.fillSmoothRoundRect(kMargin, y, filled, 4, 2, tint);
}

namespace {
// Recua um caractere UTF-8 inteiro
size_t previousChar(const char* text, size_t position) {
  do { --position; } while (position > 0 && (text[position] & 0xC0) == 0x80);
  return position;
}
}  // namespace

void drawFittedText(LovyanGFX& gfx, const char* text, int x, int y, int maxWidth) {
  char buffer[kNewsTitleSize + 4];
  snprintf(buffer, sizeof(buffer) - 4, "%s", text);
  size_t length = strlen(buffer);
  while (length > 0 && gfx.textWidth(buffer) > maxWidth) {
    length = previousChar(buffer, length);
    strcpy(buffer + length, "…");
  }
  gfx.drawString(buffer, x, y);
}

int drawWrappedText(LovyanGFX& gfx, const char* text, int x, int y, int maxWidth, int lineHeight, int maxLines) {
  char line[kNewsTitleSize];
  for (int row = 0; row < maxLines && *text != '\0'; ++row, y += lineHeight) {
    if (row == maxLines - 1) {
      drawFittedText(gfx, text, x, y, maxWidth);
      return y + lineHeight;
    }
    size_t fit = 0;
    for (size_t i = 0;; ++i) {
      if (text[i] != ' ' && text[i] != '\0') continue;
      snprintf(line, std::min(sizeof(line), i + 1), "%s", text);
      if (gfx.textWidth(line) > maxWidth && fit > 0) break;
      fit = i;
      if (text[i] == '\0') break;
    }
    snprintf(line, std::min(sizeof(line), fit + 1), "%s", text);
    gfx.drawString(line, x, y);
    text += fit;
    while (*text == ' ') ++text;
  }
  return y;
}

void drawPageDots(LovyanGFX& gfx, uint8_t current, uint8_t total) {
  constexpr int kSpacing = 10;
  const int startX = gfx.width() / 2 - (total - 1) * kSpacing / 2;
  const int y = gfx.height() - kFooterBottomGap / 2 - 1;
  for (uint8_t i = 0; i < total; ++i) {
    gfx.fillSmoothCircle(startX + i * kSpacing, y, 2,
                         i == current ? color::kText : blend(color::kPanel, color::kText, 0.35f));
  }
}

void drawTrendArrow(LovyanGFX& gfx, int x, int y, float change, uint32_t tint) {
  constexpr int kHalf = 4;
  if (fabsf(change) < kStableThreshold) {
    gfx.fillRect(x, y - 1, 2 * kHalf, 3, tint);
  } else if (change > 0) {
    gfx.fillTriangle(x, y + kHalf, x + 2 * kHalf, y + kHalf, x + kHalf, y - kHalf, tint);
  } else {
    gfx.fillTriangle(x, y - kHalf, x + 2 * kHalf, y - kHalf, x + kHalf, y + kHalf, tint);
  }
}

const char* formatDecimal(float value, uint8_t decimals, bool forceSign) {
  static char buffer[16];
  snprintf(buffer, sizeof(buffer), forceSign ? "%+.*f" : "%.*f", decimals, value);
  if (char* dot = strchr(buffer, '.')) *dot = ',';
  return buffer;
}

const char* formatThousands(float value) {
  static char buffer[16];
  const long rounded = lroundf(value);
  if (rounded >= 1000000) {
    snprintf(buffer, sizeof(buffer), "%ld.%03ld.%03ld", rounded / 1000000, rounded / 1000 % 1000, rounded % 1000);
  } else if (rounded >= 1000) {
    snprintf(buffer, sizeof(buffer), "%ld.%03ld", rounded / 1000, rounded % 1000);
  } else {
    return formatDecimal(value, 2);
  }
  return buffer;
}

const char* formatCompact(float value) {
  static char buffer[16];
  if (value >= 1000) {
    snprintf(buffer, sizeof(buffer), "%ld mil", lroundf(value / 1000));
    return buffer;
  }
  return formatDecimal(value, value >= 100 ? 0 : (value >= 10 ? 1 : 2));
}

uint32_t trendColor(float change) {
  if (fabsf(change) < kStableThreshold) return color::kNeutral;
  return change > 0 ? color::kPositive : color::kNegative;
}

}  // namespace ui
