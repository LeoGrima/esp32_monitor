#pragma once

#include <stddef.h>

#include "../model/dashboard_data.h"

// Interpretação das respostas das APIs. Sem dependência de hardware: também roda no PC.
namespace parsers {

constexpr uint8_t kFeedTitleSize = kNewsTitleSize;

struct VideoEntry {
  char title[96];
  char id[16];
  long long publishedAt;
};

// Geolocalização pelo IP (ipwho.is)
bool parseLocation(const char* json, Place& place);

bool parseWeather(const char* json, WeatherData& weather);

// PTAX: preenche a cotação e, se informados, o histórico diário e a hora do último boletim
bool parsePtax(const char* json, CurrencyQuote& quote, float* week = nullptr, uint8_t* weekCount = nullptr,
               char* updatedAt = nullptr);

bool parseCoin(const char* json, const char* coinId, const char* currency, CoinQuote& coin);
bool parseStock(const char* json, StockQuote& stock);

// Fuso no formato POSIX a partir do deslocamento em relação ao UTC (ex.: -14400 -> "<-0400>4:00")
void formatTimezone(int32_t utcOffsetSeconds, char* out, size_t outSize);

// "AAAA-MM-DDTHH:MM[:SS]" (UTC) -> epoch
long long parseIsoDate(const char* text);

uint8_t parseNewsFeed(const char* xml, char titles[][kFeedTitleSize], uint8_t maxTitles);
uint8_t parseVideoFeed(const char* xml, VideoEntry* entries, uint8_t maxEntries, char* channel, size_t channelSize);

}  // namespace parsers
