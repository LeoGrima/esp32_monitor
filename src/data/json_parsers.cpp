#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ArduinoJson.h>

#include "data_parsers.h"

namespace parsers {

constexpr uint8_t kDeepNesting = 24;  // a resposta do Yahoo é bem aninhada

bool parseLocation(const char* json, Place& place) {
  JsonDocument doc;
  if (deserializeJson(doc, json) || !(doc["success"] | false)) return false;
  const char* city = doc["city"];
  if (city == nullptr || doc["latitude"].isNull() || doc["longitude"].isNull()) return false;
  snprintf(place.city, sizeof(place.city), "%s", city);
  place.latitude = doc["latitude"];
  place.longitude = doc["longitude"];
  place.hasUtcOffset = !doc["timezone"]["offset"].isNull();
  place.utcOffsetSeconds = doc["timezone"]["offset"] | 0;
  return true;
}

bool parseWeather(const char* json, WeatherData& weather) {
  JsonDocument doc;
  if (deserializeJson(doc, json)) return false;
  JsonObjectConst current = doc["current"];
  if (current["temperature_2m"].isNull()) return false;

  weather.temperature = current["temperature_2m"];
  weather.weatherCode = current["weather_code"];
  weather.isDay = current["is_day"] != 0;
  weather.tempMax = doc["daily"]["temperature_2m_max"][0];
  weather.tempMin = doc["daily"]["temperature_2m_min"][0];
  weather.rainProbability = 0;
  for (JsonVariantConst probability : doc["hourly"]["precipitation_probability"].as<JsonArrayConst>()) {
    if (probability.as<int>() > weather.rainProbability) weather.rainProbability = probability;
  }
  weather.valid = true;
  return true;
}

bool parsePtax(const char* json, CurrencyQuote& quote, float* week, uint8_t* weekCount, char* updatedAt) {
  struct Day {
    char date[11];
    float close;
  };
  JsonDocument doc;
  if (deserializeJson(doc, json)) return false;

  // Há vários boletins por dia; o último de cada dia vale como fechamento
  Day days[kWeekHistorySize] = {};
  uint8_t count = 0;
  const char* lastTimestamp = nullptr;
  for (JsonObjectConst item : doc["value"].as<JsonArrayConst>()) {
    const char* timestamp = item["dataHoraCotacao"];  // "AAAA-MM-DD HH:MM:SS.mmm"
    if (timestamp == nullptr || strlen(timestamp) < 16) continue;
    if (count == 0 || strncmp(days[count - 1].date, timestamp, 10) != 0) {
      if (count == kWeekHistorySize) {
        memmove(days, days + 1, sizeof(Day) * (kWeekHistorySize - 1));
        --count;
      }
      snprintf(days[count++].date, sizeof(Day::date), "%s", timestamp);
    }
    days[count - 1].close = item["cotacaoVenda"];
    lastTimestamp = timestamp;
  }
  if (count == 0) return false;

  quote.bid = days[count - 1].close;
  quote.pctChange = count > 1 ? (quote.bid / days[count - 2].close - 1) * 100 : 0;
  quote.valid = true;
  if (week != nullptr && weekCount != nullptr) {
    for (uint8_t i = 0; i < count; ++i) week[i] = days[i].close;
    *weekCount = count;
  }
  if (updatedAt != nullptr) snprintf(updatedAt, 6, "%s", lastTimestamp + 11);
  return true;
}

bool parseCoin(const char* json, const char* coinId, const char* currency, CoinQuote& coin) {
  JsonDocument doc;
  if (deserializeJson(doc, json)) return false;
  JsonObjectConst entry = doc[coinId];
  if (entry[currency].isNull()) return false;

  char changeKey[24];
  snprintf(changeKey, sizeof(changeKey), "%s_24h_change", currency);
  coin.price = entry[currency];
  coin.pctChange24h = entry[changeKey];
  coin.valid = true;
  return true;
}

bool parseStock(const char* json, StockQuote& stock) {
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::NestingLimit(kDeepNesting))) return false;
  JsonObjectConst meta = doc["chart"]["result"][0]["meta"];
  const float price = meta["regularMarketPrice"] | 0.0f;
  const float previous = meta["chartPreviousClose"] | (meta["previousClose"] | 0.0f);
  if (price <= 0 || previous <= 0) return false;

  stock.price = price;
  stock.pctChange = (price / previous - 1) * 100;
  stock.valid = true;
  return true;
}

void formatTimezone(int32_t utcOffsetSeconds, char* out, size_t outSize) {
  const bool east = utcOffsetSeconds >= 0;
  const int32_t seconds = east ? utcOffsetSeconds : -utcOffsetSeconds;
  const int hours = seconds / 3600;
  const int minutes = seconds % 3600 / 60;
  // No padrão POSIX o sinal do deslocamento é invertido em relação ao nome
  snprintf(out, outSize, "<%c%02d%02d>%s%d:%02d", east ? '+' : '-', hours, minutes, east ? "-" : "", hours, minutes);
}

long long parseIsoDate(const char* text) {
  int year, month, day, hour, minute;
  int second = 0;
  if (sscanf(text, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) < 5) return 0;
  // Dias desde 1970 (algoritmo civil de Howard Hinnant)
  year -= month <= 2;
  const long long era = (year >= 0 ? year : year - 399) / 400;
  const long long yearOfEra = year - era * 400;
  const long long dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const long long dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
  const long long days = era * 146097 + dayOfEra - 719468;
  return days * 86400 + hour * 3600 + minute * 60 + second;
}

uint8_t parseStoryIds(const char* json, uint32_t* ids, uint8_t maxIds) {
  // A lista tem centenas de itens; lê só os primeiros, sem montar o documento
  uint8_t count = 0;
  const char* cursor = strchr(json, '[');
  while (cursor != nullptr && count < maxIds) {
    char* end = nullptr;
    const unsigned long id = strtoul(cursor + 1, &end, 10);
    if (end == cursor + 1) break;
    ids[count++] = id;
    cursor = (*end == ',') ? end : nullptr;
  }
  return count;
}

bool parseStory(const char* json, char* title, size_t titleSize, uint32_t& score, uint32_t& comments) {
  JsonDocument filter;
  filter["title"] = filter["score"] = filter["descendants"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter))) return false;
  const char* text = doc["title"];
  if (text == nullptr) return false;
  snprintf(title, titleSize, "%s", text);
  score = doc["score"] | 0;
  comments = doc["descendants"] | 0;
  return true;
}

}  // namespace parsers
