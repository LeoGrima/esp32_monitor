#include <Arduino.h>
#include <WiFi.h>

#include "app_config.h"
#include "board_pins.h"
#include "data/data_services.h"
#include "display/board_display.h"
#include "net/wifi_setup.h"
#include "ui/ui_screens.h"
#include "ui/ui_widgets.h"

namespace {

constexpr time_t kValidEpoch = 1700000000;  // antes disso o relógio ainda não foi sincronizado
constexpr uint32_t kNetworkTaskStack = 16384;
constexpr uint32_t kTouchDebounceMs = 250;
constexpr uint32_t kSetupHoldMs = 2000;  // BOOT pressionado por esse tempo abre a configuração do Wi-Fi

BoardDisplay display;
LGFX_Sprite canvas(&display);

ui::DashboardState state;
SemaphoreHandle_t stateMutex;
volatile uint32_t stateRevision = 0;
volatile bool networkPaused = false;

ui::Screen activeScreens[sizeof(config::kScreens) / sizeof(config::kScreens[0])];
uint8_t activeScreenCount = 0;
uint8_t currentScreen = 0;

struct StateLock {
  StateLock() { xSemaphoreTake(stateMutex, portMAX_DELAY); }
  ~StateLock() { xSemaphoreGive(stateMutex); }
};

// Busca os dados em uma cópia e só então publica no estado compartilhado
template <typename T>
bool refresh(T ui::DashboardState::*field, bool (*fetch)(T&)) {
  T data;
  {
    StateLock lock;
    data = state.*field;
  }
  if (!fetch(data)) return false;
  StateLock lock;
  state.*field = data;
  stateRevision = stateRevision + 1;
  return true;
}

struct Service {
  ui::Screen screen;
  uint32_t intervalMs;
  bool (*run)();
  uint32_t nextRunMs;
  bool enabled;
};

Service services[] = {
    {ui::Screen::Weather, config::kWeatherIntervalMs, [] { return refresh(&ui::DashboardState::weather, services::fetchWeather); }},
    {ui::Screen::Currency, config::kCurrencyIntervalMs, [] { return refresh(&ui::DashboardState::currency, services::fetchCurrency); }},
    {ui::Screen::Crypto, config::kCryptoIntervalMs, [] { return refresh(&ui::DashboardState::crypto, services::fetchCrypto); }},
    {ui::Screen::Stocks, config::kStocksIntervalMs, [] { return refresh(&ui::DashboardState::stocks, services::fetchStocks); }},
    {ui::Screen::News, config::kNewsIntervalMs, [] { return refresh(&ui::DashboardState::news, services::fetchNews); }},
    {ui::Screen::Video, config::kVideoIntervalMs, [] { return refresh(&ui::DashboardState::video, services::fetchVideo); }},
    {ui::Screen::HackerNews, config::kHackerNewsIntervalMs, [] { return refresh(&ui::DashboardState::hackerNews, services::fetchHackerNews); }},
    {ui::Screen::Match, config::kMatchIntervalMs, [] { return refresh(&ui::DashboardState::match, services::fetchMatch); }},
    {ui::Screen::Radar, config::kRadarIntervalMs, [] { return refresh(&ui::DashboardState::radar, services::fetchRadar); }},
    {ui::Screen::Repo, config::kRepoIntervalMs, [] { return refresh(&ui::DashboardState::repo, services::fetchRepo); }},
    {ui::Screen::Status, config::kStatusIntervalMs, [] { return refresh(&ui::DashboardState::status, services::fetchStatus); }},
    {ui::Screen::Services, config::kServicesIntervalMs, [] { return refresh(&ui::DashboardState::services, services::fetchSites); }},
};

bool isTimeSynced() { return time(nullptr) > kValidEpoch; }

void selectActiveScreens() {
  for (ui::Screen screen : config::kScreens) {
    if (screen == ui::Screen::Video && config::kYoutubeChannelId[0] == '\0') continue;
    activeScreens[activeScreenCount++] = screen;
    for (Service& service : services) service.enabled |= service.screen == screen;
  }
}

void networkTask(void*) {
  for (;;) {
    if (!networkPaused && WiFi.status() == WL_CONNECTED && isTimeSynced()) {
      for (Service& service : services) {
        if (!service.enabled || static_cast<int32_t>(millis() - service.nextRunMs) < 0) continue;
        const bool ok = service.run();
        service.nextRunMs = millis() + (ok ? service.intervalMs : config::kRetryIntervalMs);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void changeScreen(int step) {
  currentScreen = (currentScreen + activeScreenCount + step) % activeScreenCount;
}

// Toque na faixa esquerda volta; no restante da tela (ou botão BOOT) avança
bool handleInput() {
  static uint32_t lastInputMs = 0;
  static bool wasPressed = false;
  int32_t x = 0;
  int32_t y = 0;
  const bool touched = display.getTouch(&x, &y);
  const bool pressed = touched || digitalRead(pins::kBootButton) == LOW;
  const bool triggered = pressed && !wasPressed && millis() - lastInputMs > kTouchDebounceMs;
  wasPressed = pressed;
  if (!triggered) return false;

  lastInputMs = millis();
  changeScreen(touched && x < display.width() / 3 ? -1 : 1);
  return true;
}

void renderDashboard(time_t now) {
  {
    StateLock lock;
    localtime_r(&now, &state.now);
    state.nowEpoch = now;
    ui::drawScreen(canvas, activeScreens[currentScreen], state);
  }
  ui::drawPageDots(canvas, currentScreen, activeScreenCount);
  canvas.pushSprite(0, 0);
}

void renderMessage(const char* title, const char* detail) {
  ui::drawMessage(canvas, title, detail);
  canvas.pushSprite(0, 0);
}

void runWifiSetup() {
  networkPaused = true;
  wifi_setup::run(display, canvas);
  networkPaused = false;
}

// BOOT mantido pressionado abre a configuração do Wi-Fi
bool isSetupRequested() {
  static uint32_t pressedSinceMs = 0;
  if (digitalRead(pins::kBootButton) != LOW) {
    pressedSinceMs = 0;
    return false;
  }
  if (pressedSinceMs == 0) pressedSinceMs = millis();
  return millis() - pressedSinceMs > kSetupHoldMs;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(pins::kAudioAmpEnable, OUTPUT);
  digitalWrite(pins::kAudioAmpEnable, HIGH);  // amplificador desligado
  pinMode(pins::kBootButton, INPUT_PULLUP);

  display.init();
  display.setRotation(config::kDisplayRotation);
  display.setBrightness(config::kBrightness);
  canvas.setPsram(true);
  canvas.setColorDepth(16);
  canvas.createSprite(display.width(), display.height());

  state.city = config::kCityName;
  state.newsSource = config::kNewsSource;
  state.southernHemisphere = config::kSouthernHemisphere;
  state.repo.name = config::kGithubRepo;
  selectActiveScreens();

  String ssid;
  String password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  if (wifi_setup::loadCredentials(ssid, password)) {
    renderMessage("Conectando", "Segure BOOT para trocar a rede");
    WiFi.begin(ssid.c_str(), password.c_str());
  } else {
    wifi_setup::run(display, canvas);
  }
  configTzTime(config::kTimezone, config::kNtpServer, "pool.ntp.org");

  stateMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(networkTask, "network", kNetworkTaskStack, nullptr, 1, nullptr, 0);
}

void loop() {
  static time_t lastRenderedSecond = 0;
  static uint32_t lastRevision = 0;
  static uint32_t lastScreenChangeMs = 0;
  static bool ready = false;

  delay(20);
  if (isSetupRequested()) {
    runWifiSetup();
    ready = false;
  }
  if (!isTimeSynced()) {
    if (WiFi.status() == WL_CONNECTED) renderMessage("Sincronizando", "Ajustando data e hora");
    delay(500);
    return;
  }

  bool dirty = !ready;
  ready = true;
  if (handleInput()) {
    lastScreenChangeMs = millis();
    dirty = true;
  } else if (config::kScreenRotationSeconds > 0 &&
             millis() - lastScreenChangeMs > config::kScreenRotationSeconds * 1000UL) {
    changeScreen(1);
    lastScreenChangeMs = millis();
    dirty = true;
  }

  const time_t now = time(nullptr);
  const bool isClock = activeScreens[currentScreen] == ui::Screen::Clock;
  dirty |= activeScreens[currentScreen] == ui::Screen::Fire;  // animação contínua
  dirty |= stateRevision != lastRevision;
  dirty |= isClock ? now != lastRenderedSecond : now / 60 != lastRenderedSecond / 60;
  if (!dirty) return;

  lastRevision = stateRevision;
  lastRenderedSecond = now;
  renderDashboard(now);
}
