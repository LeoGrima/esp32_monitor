#include "wifi_setup.h"

#include <Preferences.h>
#include <WiFi.h>

#include "ui/ui_screens.h"
#include "ui/wifi_setup_ui.h"

#if __has_include("secrets.h")
#include "secrets.h"  // opcional: rede padrão para placas sem toque
#define HAS_DEFAULT_CREDENTIALS 1
#endif

namespace wifi_setup {
namespace {

constexpr char kStorage[] = "wifi";
constexpr char kSsidKey[] = "ssid";
constexpr char kPasswordKey[] = "password";
constexpr uint32_t kConnectTimeoutMs = 15000;
constexpr uint32_t kMessageMs = 2000;
constexpr uint8_t kMaxNetworks = 20;
constexpr uint8_t kMaxPasswordLength = 63;

ui::NetworkEntry networks[kMaxNetworks];
uint8_t networkCount = 0;

void showMessage(LGFX_Sprite& canvas, const char* title, const char* detail) {
  ui::drawMessage(canvas, title, detail);
  canvas.pushSprite(0, 0);
}

// Aguarda um novo toque (borda de descida) e devolve a posição
void waitTap(lgfx::LGFX_Device& display, int32_t& x, int32_t& y) {
  while (display.getTouch(&x, &y)) delay(10);
  while (!display.getTouch(&x, &y)) delay(10);
}

bool isListed(const String& ssid) {
  for (uint8_t i = 0; i < networkCount; ++i) {
    if (ssid == networks[i].ssid) return true;
  }
  return false;
}

void scanNetworks(LGFX_Sprite& canvas) {
  showMessage(canvas, "Procurando redes", "Aguarde alguns segundos");
  WiFi.disconnect();
  const int found = WiFi.scanNetworks();  // já ordenadas pelo sinal
  networkCount = 0;
  for (int i = 0; i < found && networkCount < kMaxNetworks; ++i) {
    const String ssid = WiFi.SSID(i);
    if (ssid.isEmpty() || isListed(ssid)) continue;
    ui::NetworkEntry& entry = networks[networkCount++];
    snprintf(entry.ssid, sizeof(entry.ssid), "%s", ssid.c_str());
    entry.rssi = WiFi.RSSI(i);
    entry.secured = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
}

// Retorna o índice da rede escolhida
uint8_t chooseNetwork(lgfx::LGFX_Device& display, LGFX_Sprite& canvas) {
  ui::NetworkListView view = {networks, networkCount, 0};
  for (;;) {
    ui::drawNetworkList(canvas, view);
    canvas.pushSprite(0, 0);
    int32_t x = 0;
    int32_t y = 0;
    waitTap(display, x, y);
    const ui::ListHit hit = ui::hitNetworkList(view, x, y);
    const uint8_t lastPage = networkCount == 0 ? 0 : (networkCount - 1) / ui::kNetworksPerPage;
    switch (hit.action) {
      case ui::ListAction::Select:   return hit.index;
      case ui::ListAction::Previous: if (view.page > 0) --view.page; break;
      case ui::ListAction::Next:     if (view.page < lastPage) ++view.page; break;
      case ui::ListAction::Rescan:
        scanNetworks(canvas);
        view = {networks, networkCount, 0};
        break;
      default: break;
    }
  }
}

// Retorna false se o usuário voltou para a lista
bool askPassword(lgfx::LGFX_Device& display, LGFX_Sprite& canvas, const char* ssid, String& password) {
  ui::KeyboardMode mode = ui::KeyboardMode::Lower;
  password = "";
  for (;;) {
    ui::drawKeyboard(canvas, {ssid, password.c_str(), mode});
    canvas.pushSprite(0, 0);
    int32_t x = 0;
    int32_t y = 0;
    waitTap(display, x, y);
    const ui::KeyHit hit = ui::hitKeyboard(mode, x, y);
    switch (hit.action) {
      case ui::KeyAction::Character:
        if (password.length() < kMaxPasswordLength) password += hit.character;
        break;
      case ui::KeyAction::Backspace: password.remove(password.length() - 1); break;
      case ui::KeyAction::Shift:
        mode = mode == ui::KeyboardMode::Lower ? ui::KeyboardMode::Upper : ui::KeyboardMode::Lower;
        break;
      case ui::KeyAction::Symbols:
        mode = mode == ui::KeyboardMode::Symbols ? ui::KeyboardMode::Lower : ui::KeyboardMode::Symbols;
        break;
      case ui::KeyAction::Cancel:  return false;
      case ui::KeyAction::Confirm: return true;
      default: break;
    }
  }
}

bool connect(const char* ssid, const String& password) {
  WiFi.begin(ssid, password.c_str());
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < kConnectTimeoutMs) delay(100);
  return WiFi.status() == WL_CONNECTED;
}

void saveCredentials(const char* ssid, const String& password) {
  Preferences storage;
  storage.begin(kStorage, false);
  storage.putString(kSsidKey, ssid);
  storage.putString(kPasswordKey, password);
  storage.end();
}

}  // namespace

bool loadCredentials(String& ssid, String& password) {
  Preferences storage;
  storage.begin(kStorage, true);
  ssid = storage.getString(kSsidKey, "");
  password = storage.getString(kPasswordKey, "");
  storage.end();
#ifdef HAS_DEFAULT_CREDENTIALS
  if (ssid.isEmpty()) {
    ssid = secrets::kWifiSsid;
    password = secrets::kWifiPassword;
  }
#endif
  return !ssid.isEmpty();
}

void run(lgfx::LGFX_Device& display, LGFX_Sprite& canvas) {
  WiFi.mode(WIFI_STA);
  scanNetworks(canvas);
  for (;;) {
    const ui::NetworkEntry& network = networks[chooseNetwork(display, canvas)];
    String password;
    if (network.secured && !askPassword(display, canvas, network.ssid, password)) continue;

    showMessage(canvas, "Conectando", network.ssid);
    if (connect(network.ssid, password)) {
      saveCredentials(network.ssid, password);
      showMessage(canvas, "Conectado", network.ssid);
      delay(kMessageMs);
      return;
    }
    showMessage(canvas, "Não conectou", "Confira a senha e tente de novo");
    delay(kMessageMs);
  }
}

}  // namespace wifi_setup
