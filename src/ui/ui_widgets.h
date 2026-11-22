#pragma once

#include <LovyanGFX.hpp>

namespace ui {

struct Rect {
  int x, y, w, h;
};

uint32_t blend(uint32_t from, uint32_t to, float ratio);
void fillGradientBand(LovyanGFX& gfx, int top, int bottom, uint32_t topColor, uint32_t bottomColor);
void fillBackground(LovyanGFX& gfx, uint32_t topColor, uint32_t bottomColor);
void drawStars(LovyanGFX& gfx, int count);
void drawGlow(LovyanGFX& gfx, int x, int y, int radius, uint32_t tint);
void drawHills(LovyanGFX& gfx, float darkness);

void drawHeader(LovyanGFX& gfx, const char* title, const char* status, uint32_t statusColor, bool statusDot = false);
struct FooterItem {
  const char* label;
  const char* detail;  // opcional, ao lado do rótulo
  char value[20];
  uint32_t tint;
};

// Item de rodapé com variação percentual colorida
FooterItem changeItem(const char* label, const char* detail, float change, bool valid);

void fillPanel(LovyanGFX& gfx, const Rect& rect, int radius);
Rect drawFooterPanel(LovyanGFX& gfx);
void drawFooterColumns(LovyanGFX& gfx, const FooterItem* items, uint8_t count);
void drawChange(LovyanGFX& gfx, int x, int y, float change, uint8_t decimals, const char* suffix);
void drawProgressBar(LovyanGFX& gfx, int y, uint8_t index, uint8_t total, uint32_t tint);
// Corta o texto com reticências na largura disponível
void drawFittedText(LovyanGFX& gfx, const char* text, int x, int y, int maxWidth);
// Quebra o texto em linhas; retorna a coordenada y após a última linha
int drawWrappedText(LovyanGFX& gfx, const char* text, int x, int y, int maxWidth, int lineHeight, int maxLines);
void drawPageDots(LovyanGFX& gfx, uint8_t current, uint8_t total);
void drawTrendArrow(LovyanGFX& gfx, int x, int y, float change, uint32_t tint);

// Números no padrão brasileiro (vírgula decimal, ponto de milhar)
const char* formatDecimal(float value, uint8_t decimals, bool forceSign = false);
const char* formatThousands(float value);
const char* formatCompact(float value);
uint32_t trendColor(float change);

}  // namespace ui
