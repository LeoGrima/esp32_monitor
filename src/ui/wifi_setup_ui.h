#pragma once

#include <LovyanGFX.hpp>

// Telas de configuração do Wi-Fi: lista de redes e teclado. Somente desenho e mapeamento de toque.
namespace ui {

constexpr uint8_t kNetworksPerPage = 5;

struct NetworkEntry {
  char ssid[33];
  int8_t rssi;
  bool secured;
};

struct NetworkListView {
  const NetworkEntry* items;
  uint8_t count;
  uint8_t page;
};

enum class ListAction : uint8_t { None, Select, Previous, Next, Rescan };

struct ListHit {
  ListAction action;
  uint8_t index;  // rede escolhida, quando action == Select
};

enum class KeyboardMode : uint8_t { Lower, Upper, Symbols };

struct KeyboardView {
  const char* ssid;
  const char* text;
  KeyboardMode mode;
};

enum class KeyAction : uint8_t { None, Character, Backspace, Shift, Symbols, Cancel, Confirm };

struct KeyHit {
  KeyAction action;
  char character;
};

void drawNetworkList(LovyanGFX& gfx, const NetworkListView& view);
ListHit hitNetworkList(const NetworkListView& view, int x, int y);

void drawKeyboard(LovyanGFX& gfx, const KeyboardView& view);
KeyHit hitKeyboard(KeyboardMode mode, int x, int y);

}  // namespace ui
