#pragma once

#include <stdint.h>

constexpr uint8_t kWeekHistorySize = 7;
constexpr uint8_t kMaxCoins = 4;
constexpr uint8_t kMaxStocks = 3;
constexpr uint8_t kNewsVisible = 3;
constexpr uint8_t kNewsTitleSize = 160;

struct CurrencyQuote {
  float bid = 0;
  float pctChange = 0;  // em relação ao fechamento anterior
  bool valid = false;
};

struct CurrencyData {
  CurrencyQuote usd;
  CurrencyQuote eur;
  float usdWeek[kWeekHistorySize] = {};  // do mais antigo para o mais recente
  uint8_t usdWeekCount = 0;
  char updatedAt[6] = "";  // HH:MM do último boletim PTAX
};

struct CoinQuote {
  const char* symbol = "";
  float price = 0;
  float pctChange24h = 0;
  bool valid = false;
};

struct CryptoData {
  CoinQuote coins[kMaxCoins];  // a primeira é a moeda em destaque
  uint8_t count = 0;
  const char* currencyLabel = "R$";
};

struct StockQuote {
  const char* label = "";
  float price = 0;
  float pctChange = 0;
  bool isIndex = false;  // índices são exibidos em pontos
  bool valid = false;
};

struct StocksData {
  StockQuote items[kMaxStocks];  // o primeiro é o ativo em destaque
  uint8_t count = 0;
};

struct VideoData {
  char title[96] = "";
  char channel[40] = "";
  long long publishedAt = 0;  // epoch UTC
  uint8_t index = 0;          // posição entre os vídeos recentes
  uint8_t total = 0;
  const uint8_t* thumbnail = nullptr;  // JPEG
  uint32_t thumbnailSize = 0;
  bool valid = false;
};

struct NewsData {
  char headlines[kNewsVisible][kNewsTitleSize] = {};  // manchete + chamadas seguintes
  char detail[40] = "";  // linha complementar (ex.: pontos e comentários)
  uint8_t index = 0;
  uint8_t total = 0;
  bool valid = false;
};

enum class BatteryTrend : uint8_t { Unknown, Stable, Charging, Discharging };

struct BatteryData {
  float voltage = 0;
  uint8_t percent = 0;
  BatteryTrend trend = BatteryTrend::Unknown;
  float ratePerHour = 0;          // variação da carga, em % por hora
  int32_t minutesRemaining = -1;  // até esvaziar ou até 100%; -1 = ainda sem estimativa
  bool valid = false;
};

struct Place {
  char city[32] = "";
  float latitude = 0;
  float longitude = 0;
  int32_t utcOffsetSeconds = 0;
  bool hasUtcOffset = false;
};

struct WeatherData {
  float temperature = 0;
  float tempMax = 0;
  float tempMin = 0;
  uint8_t rainProbability = 0;  // máxima das próximas 12 h, em %
  uint8_t weatherCode = 0;      // código WMO
  bool isDay = true;
  bool valid = false;
};
