#include "wifi_setup_ui.h"

#include <string.h>

#include "fonts/ui_fonts.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr uint32_t kKey = 0x26304Fu;
constexpr uint32_t kKeySpecial = 0x3B4A78u;
constexpr uint32_t kKeyConfirm = 0x1F9D55u;

// Lista de redes
constexpr int kListTop = 38;
constexpr int kListRowHeight = 34;
constexpr int kButtonTop = 210;
constexpr int kButtonHeight = 28;
constexpr int kButtonWidth = 98;
constexpr int kButtonGap = 7;
constexpr const char* kListButtons[] = {"Anterior", "Atualizar", "Próxima"};
constexpr ListAction kListActions[] = {ListAction::Previous, ListAction::Rescan, ListAction::Next};

// Teclado: 4 linhas de 10 teclas + 1 linha de funções
constexpr int kKeyboardTop = 70;
constexpr int kKeyWidth = 32;
constexpr int kKeyHeight = 34;
constexpr int kKeyColumns = 10;
constexpr int kCharacterRows = 4;
constexpr char kBackspace = '\b';
constexpr const char* kLayouts[][kCharacterRows] = {
    {"1234567890", "qwertyuiop", "asdfghjkl\b", "zxcvbnm.-_"},
    {"1234567890", "QWERTYUIOP", "ASDFGHJKL\b", "ZXCVBNM.-_"},
    {"1234567890", "!@#$%&*()+", "=/\\?:;'\"~\b", "[]{}<>,|^`"},
};

struct FunctionKey {
  KeyAction action;
  int x;
  int width;
  const char* label;
  uint32_t tint;
};
constexpr FunctionKey kFunctionKeys[] = {
    {KeyAction::Shift, 0, 48, "Aa", kKeySpecial},      {KeyAction::Symbols, 48, 48, "#+=", kKeySpecial},
    {KeyAction::Character, 96, 96, "espaço", kKey},    {KeyAction::Cancel, 192, 64, "Voltar", kKeySpecial},
    {KeyAction::Confirm, 256, 64, "OK", kKeyConfirm},
};

void drawButton(LovyanGFX& gfx, int x, int y, int width, int height, const char* label, uint32_t tint) {
  gfx.fillSmoothRoundRect(x + 1, y + 1, width - 2, height - 2, 5, tint);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(color::kText);
  gfx.drawString(label, x + width / 2, y + height / 2);
}

void drawSignal(LovyanGFX& gfx, int x, int y, int8_t rssi) {
  constexpr int8_t kThresholds[] = {-85, -75, -65, -55};
  for (int bar = 0; bar < 4; ++bar) {
    const int height = 4 + bar * 3;
    gfx.fillRect(x + bar * 5, y - height, 3, height,
                 rssi >= kThresholds[bar] ? color::kText : blend(color::kPanel, color::kText, 0.3f));
  }
}

void drawLock(LovyanGFX& gfx, int x, int y) {
  gfx.drawRoundRect(x + 2, y - 9, 6, 8, 3, color::kTextMuted);
  gfx.fillRect(x, y - 5, 10, 7, color::kTextMuted);
}

void drawBackspace(LovyanGFX& gfx, int cx, int cy) {
  gfx.fillTriangle(cx - 9, cy, cx - 3, cy - 6, cx - 3, cy + 6, color::kText);
  gfx.fillRect(cx - 3, cy - 6, 12, 13, color::kText);
}

// Mostra o final do texto quando ele não cabe no campo
const char* tailThatFits(LovyanGFX& gfx, const char* text, int maxWidth) {
  while (*text != '\0' && gfx.textWidth(text) > maxWidth) ++text;
  return text;
}

}  // namespace

void drawNetworkList(LovyanGFX& gfx, const NetworkListView& view) {
  fillBackground(gfx, color::kNightTop, color::kNightBottom);
  drawHeader(gfx, "Escolha a rede Wi-Fi", nullptr, color::kText);

  gfx.setFont(&fonts().medium);
  if (view.count == 0) {
    gfx.setTextDatum(lgfx::middle_center);
    gfx.setTextColor(color::kTextMuted);
    gfx.drawString("Nenhuma rede encontrada", gfx.width() / 2, 120);
  }
  const uint8_t first = view.page * kNetworksPerPage;
  for (uint8_t row = 0; row < kNetworksPerPage && first + row < view.count; ++row) {
    const NetworkEntry& network = view.items[first + row];
    const int y = kListTop + row * kListRowHeight;
    fillPanel(gfx, {kPanelInset, y, gfx.width() - 2 * kPanelInset, kListRowHeight - 4}, 8);
    gfx.setTextDatum(lgfx::middle_left);
    gfx.setTextColor(color::kText);
    drawFittedText(gfx, network.ssid, kPanelInset + 12, y + kListRowHeight / 2 - 2, gfx.width() - 96);
    drawSignal(gfx, gfx.width() - kPanelInset - 30, y + kListRowHeight / 2 + 5, network.rssi);
    if (network.secured) drawLock(gfx, gfx.width() - kPanelInset - 50, y + kListRowHeight / 2 + 3);
  }

  for (int i = 0; i < 3; ++i) {
    drawButton(gfx, kButtonGap + i * (kButtonWidth + kButtonGap), kButtonTop, kButtonWidth, kButtonHeight,
               kListButtons[i], kKeySpecial);
  }
}

ListHit hitNetworkList(const NetworkListView& view, int x, int y) {
  if (y >= kButtonTop) {
    const int index = (x - kButtonGap) / (kButtonWidth + kButtonGap);
    return {index >= 0 && index < 3 ? kListActions[index] : ListAction::None, 0};
  }
  if (y < kListTop) return {ListAction::None, 0};
  const int index = view.page * kNetworksPerPage + (y - kListTop) / kListRowHeight;
  return {index < view.count ? ListAction::Select : ListAction::None, static_cast<uint8_t>(index)};
}

void drawKeyboard(LovyanGFX& gfx, const KeyboardView& view) {
  fillBackground(gfx, color::kNightTop, 0x121A38u);
  gfx.setFont(&fonts().small);
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kTextMuted);
  gfx.drawString("SENHA DA REDE", kMargin, 8);
  gfx.setTextDatum(lgfx::top_right);
  gfx.setTextColor(color::kText);
  drawFittedText(gfx, view.ssid, gfx.width() - kMargin, 8, 170);

  const int fieldWidth = gfx.width() - 2 * kPanelInset;
  gfx.fillSmoothRoundRect(kPanelInset, 28, fieldWidth, 34, 6, color::kPanel);
  gfx.setFont(&fonts().medium);
  gfx.setTextDatum(lgfx::middle_left);
  const char* visible = tailThatFits(gfx, view.text, fieldWidth - 28);
  gfx.drawString(visible, kPanelInset + 10, 45);
  gfx.fillRect(kPanelInset + 12 + gfx.textWidth(visible), 36, 2, 18, color::kCalm);

  const char* const* layout = kLayouts[static_cast<uint8_t>(view.mode)];
  for (int row = 0; row < kCharacterRows; ++row) {
    for (int column = 0; column < kKeyColumns; ++column) {
      const char label[2] = {layout[row][column], '\0'};
      const int x = column * kKeyWidth;
      const int y = kKeyboardTop + row * kKeyHeight;
      if (label[0] == kBackspace) {
        drawButton(gfx, x, y, kKeyWidth, kKeyHeight, "", kKeySpecial);
        drawBackspace(gfx, x + kKeyWidth / 2, y + kKeyHeight / 2);
      } else {
        drawButton(gfx, x, y, kKeyWidth, kKeyHeight, label, kKey);
      }
    }
  }
  const int functionY = kKeyboardTop + kCharacterRows * kKeyHeight;
  for (const FunctionKey& key : kFunctionKeys) {
    const bool symbolsKey = key.action == KeyAction::Symbols && view.mode == KeyboardMode::Symbols;
    drawButton(gfx, key.x, functionY, key.width, kKeyHeight, symbolsKey ? "abc" : key.label, key.tint);
  }
}

KeyHit hitKeyboard(KeyboardMode mode, int x, int y) {
  if (y < kKeyboardTop || x < 0 || x >= kKeyWidth * kKeyColumns) return {KeyAction::None, 0};
  const int row = (y - kKeyboardTop) / kKeyHeight;
  if (row < kCharacterRows) {
    const char character = kLayouts[static_cast<uint8_t>(mode)][row][x / kKeyWidth];
    return {character == kBackspace ? KeyAction::Backspace : KeyAction::Character, character};
  }
  for (const FunctionKey& key : kFunctionKeys) {
    if (x >= key.x && x < key.x + key.width) return {key.action, ' '};
  }
  return {KeyAction::None, 0};
}

}  // namespace ui
