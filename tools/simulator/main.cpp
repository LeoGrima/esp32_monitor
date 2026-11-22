// Renderiza as telas no PC (sem hardware) e grava um PPM por tela.
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <LovyanGFX.hpp>

#include "ui/ui_screens.h"
#include "ui/ui_widgets.h"
#include "ui/wifi_setup_ui.h"

namespace {

constexpr int kWidth = 320;
constexpr int kHeight = 240;
constexpr uint8_t kScreenCount = 9;

LGFX_Sprite canvas;
std::string outputDir;

void dump(const char* name);

void save(ui::Screen screen, const ui::DashboardState& state, const char* name) {
  ui::drawScreen(canvas, screen, state);
  ui::drawPageDots(canvas, static_cast<uint8_t>(screen), kScreenCount);
  dump(name);
}

void dump(const char* name) {
  std::vector<uint8_t> pixels(kWidth * kHeight * 3);
  canvas.readRectRGB(0, 0, kWidth, kHeight, pixels.data());
  FILE* file = fopen((outputDir + "/" + name + ".ppm").c_str(), "wb");
  fprintf(file, "P6\n%d %d\n255\n", kWidth, kHeight);
  fwrite(pixels.data(), 1, pixels.size(), file);
  fclose(file);
}

}  // namespace

int main(int argc, char** argv) {
  outputDir = argc > 1 ? argv[1] : ".";
  std::vector<uint8_t> thumbnail;
  if (argc > 2) {
    std::ifstream file(argv[2], std::ios::binary);
    thumbnail.assign(std::istreambuf_iterator<char>(file), {});
  }
  canvas.setColorDepth(16);
  canvas.createSprite(kWidth, kHeight);

  ui::DashboardState state;
  state.city = "São Paulo";
  state.newsSource = "g1";
  state.weather = {27, 31, 22, 10, 2, true, true};
  state.currency.usd = {5.42f, 1.1f, true};
  state.currency.eur = {6.31f, 0.8f, true};
  strcpy(state.currency.updatedAt, "13:08");
  for (float value : {5.31f, 5.35f, 5.30f, 5.38f, 5.36f, 5.40f, 5.42f}) {
    state.currency.usdWeek[state.currency.usdWeekCount++] = value;
  }
  state.crypto.count = 4;
  state.crypto.coins[0] = {"BTC", 374918, 6.2f, true};
  state.crypto.coins[1] = {"ETH", 19034, 7.1f, true};
  state.crypto.coins[2] = {"SOL", 912, 4.4f, true};
  state.crypto.coins[3] = {"XRP", 14.6f, 5.2f, true};
  state.stocks.count = 3;
  state.stocks.items[0] = {"IBOVESPA", 132481, 1.24f, true, true};
  state.stocks.items[1] = {"S&P 500", 6100, 0.84f, true, true};
  state.stocks.items[2] = {"Nasdaq", 20100, -0.31f, true, true};
  state.news.valid = true;
  state.news.index = 1;
  state.news.total = 8;
  strcpy(state.news.headlines[0], "Prefeitura abre consulta pública sobre as novas linhas de ônibus da região central");
  strcpy(state.news.headlines[1], "Opine até sexta-feira pelo aplicativo");
  strcpy(state.news.headlines[2], "Veja os bairros atendidos e os horários previstos para cada linha");
  state.nowEpoch = 1795306477;
  state.video.valid = true;
  strcpy(state.video.title, "Como montar um painel de mesa com ESP32-S3 e tela IPS");
  strcpy(state.video.channel, "CANAL DE EXEMPLO");
  state.video.publishedAt = state.nowEpoch - 4 * 86400;
  state.video.index = 0;
  state.video.total = 5;
  if (!thumbnail.empty()) {
    state.video.thumbnail = thumbnail.data();
    state.video.thumbnailSize = thumbnail.size();
  }
  state.now.tm_year = 126; state.now.tm_mon = 10; state.now.tm_mday = 21; state.now.tm_wday = 6;
  state.now.tm_hour = 20; state.now.tm_min = 14; state.now.tm_sec = 37;

  state.hackerNews.valid = true;
  state.hackerNews.total = 5;
  strcpy(state.hackerNews.headlines[0], "Show HN: A desk dashboard built with an ESP32-S3 and a 2.8 inch IPS display");
  strcpy(state.hackerNews.headlines[1], "Why embedded fonts still matter");
  strcpy(state.hackerNews.headlines[2], "The surprising cost of TLS handshakes on microcontrollers");
  strcpy(state.hackerNews.detail, "538 pontos · 133 comentários");

  const ui::NetworkEntry networks[] = {{"Casa 2.4G", -48, true}, {"Vizinho", -67, true}, {"Cafeteria Livre", -72, false},
                                       {"Escritório", -80, true}, {"IoT", -88, true}, {"Outra", -90, true}};
  ui::drawNetworkList(canvas, {networks, 6, 0});
  dump("wifi_list");
  ui::drawKeyboard(canvas, {"Casa 2.4G", "minhaSenha123", ui::KeyboardMode::Lower});
  dump("wifi_keyboard");
  ui::drawKeyboard(canvas, {"Casa 2.4G", "", ui::KeyboardMode::Symbols});
  dump("alt_wifi_symbols");
  const ui::KeyHit keyQ = ui::hitKeyboard(ui::KeyboardMode::Lower, 5, 110);
  const ui::KeyHit keyOk = ui::hitKeyboard(ui::KeyboardMode::Lower, 300, 230);
  const ui::KeyHit keyDel = ui::hitKeyboard(ui::KeyboardMode::Upper, 300, 150);
  const ui::ListHit row2 = ui::hitNetworkList({networks, 6, 0}, 100, 80);
  const ui::ListHit rescan = ui::hitNetworkList({networks, 6, 0}, 160, 225);
  if (keyQ.character != 'q' || keyOk.action != ui::KeyAction::Confirm || keyDel.action != ui::KeyAction::Backspace ||
      row2.action != ui::ListAction::Select || row2.index != 1 || rescan.action != ui::ListAction::Rescan) {
    fprintf(stderr, "mapeamento de toque incorreto\n");
    return 1;
  }

  state.battery = {3.98f, 76, BatteryTrend::Discharging, -18.0f, 253, true};
  save(ui::Screen::Battery, state, "battery");
  state.battery = {4.12f, 91, BatteryTrend::Charging, 32.0f, 17, true};
  save(ui::Screen::Battery, state, "alt_battery_charging");
  save(ui::Screen::HackerNews, state, "hacker_news");
  save(ui::Screen::Weather, state, "weather");
  save(ui::Screen::Currency, state, "currency");
  save(ui::Screen::Crypto, state, "crypto");
  save(ui::Screen::Stocks, state, "stocks");
  save(ui::Screen::News, state, "news");
  save(ui::Screen::Video, state, "video");
  save(ui::Screen::Clock, state, "clock");

  // Variações: tempestade à noite, quedas e relógio de dia
  state.weather = {18, 21, 15, 80, 95, false, true};
  state.currency.usd.pctChange = -1.3f;
  state.crypto.coins[0].pctChange24h = -5.8f;
  state.stocks.items[0].pctChange = -1.37f;
  state.stocks.items[1].pctChange = -0.92f;
  state.video.thumbnail = nullptr;
  state.now.tm_hour = 10; state.now.tm_min = 9; state.now.tm_mon = 0; state.now.tm_mday = 14; state.now.tm_wday = 3;
  save(ui::Screen::Weather, state, "alt_weather");
  save(ui::Screen::Currency, state, "alt_currency");
  save(ui::Screen::Crypto, state, "alt_crypto");
  save(ui::Screen::Stocks, state, "alt_stocks");
  save(ui::Screen::Video, state, "alt_video");
  save(ui::Screen::Clock, state, "alt_clock");
  return 0;
}
