#pragma once

#include <time.h>

#include <LovyanGFX.hpp>

#include "../model/dashboard_data.h"

namespace ui {

enum class Screen : uint8_t {
  Clock, Weather, Currency, Crypto, Stocks, News, HackerNews, Video, Battery
};

struct HeadlineStyle {
  const char* badge;
  const char* source;
  const char* emptyMessage;
  uint32_t accent;
  uint32_t backgroundTop;
  uint32_t backgroundBottom;
};

struct DashboardState {
  WeatherData weather;
  CurrencyData currency;
  CryptoData crypto;
  StocksData stocks;
  NewsData news;
  NewsData hackerNews;
  BatteryData battery;
  VideoData video;
  tm now = {};
  long long nowEpoch = 0;
  const char* city = "";
  const char* newsSource = "";
};

void drawScreen(LovyanGFX& gfx, Screen screen, const DashboardState& state);
void drawMessage(LovyanGFX& gfx, const char* title, const char* detail);

void drawWeatherScreen(LovyanGFX& gfx, const WeatherData& weather, const char* city);
void drawCurrencyScreen(LovyanGFX& gfx, const CurrencyData& currency);
void drawCryptoScreen(LovyanGFX& gfx, const CryptoData& crypto);
void drawStocksScreen(LovyanGFX& gfx, const StocksData& stocks);
void drawHeadlineScreen(LovyanGFX& gfx, const NewsData& news, const HeadlineStyle& style);
void drawBatteryScreen(LovyanGFX& gfx, const BatteryData& battery);
void drawVideoScreen(LovyanGFX& gfx, const VideoData& video, long long nowEpoch);
void drawClockScreen(LovyanGFX& gfx, const tm& now);

}  // namespace ui
