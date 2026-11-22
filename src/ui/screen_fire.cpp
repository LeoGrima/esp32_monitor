#include <stdint.h>
#include <string.h>

#include "ui_screens.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kCell = 4;  // pixels de tela por célula da simulação
constexpr int kColumns = 80;
constexpr int kRows = 60;
constexpr uint8_t kMaxHeat = 36;

uint8_t heat[kRows + 1][kColumns];
uint32_t seed = 0x1234ABCDu;

uint32_t nextRandom() {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return seed;
}

// Paleta: preto -> vermelho -> laranja -> amarelo -> branco
uint32_t heatColor(uint8_t value) {
  constexpr uint32_t kStops[] = {0x070707u, 0x8F0F07u, 0xDF4F07u, 0xDFAF2Fu, 0xFFFFE0u};
  const float position = static_cast<float>(value) / kMaxHeat * 4;
  const int index = position >= 4 ? 3 : static_cast<int>(position);
  return blend(kStops[index], kStops[index + 1], position - index);
}

// Cada célula herda o calor da célula de baixo, com perda e deslocamento aleatórios
void stepSimulation() {
  memset(heat[kRows], kMaxHeat, kColumns);
  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kColumns; ++x) {
      const uint32_t random = nextRandom();
      const int source = x + static_cast<int>(random & 3) - 1;
      const uint8_t below = heat[y + 1][(source + kColumns) % kColumns];
      const uint8_t decay = (random >> 2) % 3;
      heat[y][x] = below > decay ? below - decay : 0;
    }
  }
}

}  // namespace

void drawFireScreen(LovyanGFX& gfx) {
  stepSimulation();
  for (int y = 0; y < kRows; ++y) {
    for (int x = 0; x < kColumns; ++x) {
      gfx.fillRect(x * kCell, y * kCell, kCell, kCell, heatColor(heat[y][x]));
    }
  }
}

}  // namespace ui
