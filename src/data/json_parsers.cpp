#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include <ArduinoJson.h>

#include "data_parsers.h"

namespace parsers {

constexpr uint8_t kDeepNesting = 24;  // respostas da ESPN e do Yahoo são bem aninhadas

bool parseLocation(const char* json, Place& place) {
  JsonDocument doc;
  if (deserializeJson(doc, json) || !(doc["success"] | false)) return false;
  const char* city = doc["city"];
  if (city == nullptr || doc["latitude"].isNull() || doc["longitude"].isNull()) return false;
  snprintf(place.city, sizeof(place.city), "%s", city);
  place.latitude = doc["latitude"];
  place.longitude = doc["longitude"];
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

namespace {

void readSide(JsonObjectConst competitor, MatchSide& side) {
  JsonObjectConst team = competitor["team"];
  snprintf(side.name, sizeof(side.name), "%s", team["shortDisplayName"] | "");
  snprintf(side.abbreviation, sizeof(side.abbreviation), "%s", team["abbreviation"] | "");
  side.score = atoi(competitor["score"] | "0");
}

bool readMatch(JsonObjectConst event, MatchInfo& match) {
  JsonArrayConst competitors = event["competitions"][0]["competitors"];
  if (competitors.size() < 2) return false;
  for (JsonObjectConst competitor : competitors) {
    readSide(competitor, strcmp(competitor["homeAway"] | "", "home") == 0 ? match.home : match.away);
  }
  JsonObjectConst type = event["status"]["type"];
  const char* state = type["state"] | "pre";
  match.state = strcmp(state, "in") == 0 ? MatchState::Live
                : strcmp(state, "post") == 0 ? MatchState::Finished : MatchState::Scheduled;
  snprintf(match.clock, sizeof(match.clock), "%s", type["shortDetail"] | "");
  match.startsAt = parseIsoDate(event["date"] | "");
  match.valid = true;
  return true;
}

bool isTeam(const MatchSide& side, const char* team) {
  return strcasecmp(side.name, team) == 0 || strcasecmp(side.abbreviation, team) == 0;
}

}  // namespace

bool parseScoreboard(const char* json, const char* favoriteTeam, MatchData& data) {
  JsonDocument filter;
  JsonObject event = filter["events"][0].to<JsonObject>();
  event["date"] = true;
  event["status"]["type"]["state"] = event["status"]["type"]["shortDetail"] = true;
  JsonObject competitor = event["competitions"][0]["competitors"][0].to<JsonObject>();
  competitor["homeAway"] = competitor["score"] = true;
  competitor["team"]["shortDisplayName"] = competitor["team"]["abbreviation"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(kDeepNesting))) {
    return false;
  }

  MatchData result;
  snprintf(result.league, sizeof(result.league), "%s", data.league);
  for (JsonObjectConst item : doc["events"].as<JsonArrayConst>()) {
    MatchInfo match;
    if (!readMatch(item, match)) continue;
    const bool favorite = isTeam(match.home, favoriteTeam) || isTeam(match.away, favoriteTeam);
    if (!result.main.valid) {
      result.main = match;
    } else if (favorite) {
      if (result.otherCount < kMaxOtherMatches) result.others[result.otherCount++] = result.main;
      result.main = match;
    } else if (result.otherCount < kMaxOtherMatches) {
      result.others[result.otherCount++] = match;
    }
  }
  if (!result.main.valid) return false;
  data = result;
  return true;
}

bool parseAircraft(const char* json, RadarData& radar) {
  constexpr float kNauticalMileKm = 1.852f;
  constexpr float kFootM = 0.3048f;
  JsonDocument filter;
  JsonObject fields = filter["ac"][0].to<JsonObject>();
  fields["flight"] = fields["t"] = fields["alt_baro"] = fields["dst"] = fields["dir"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter)) || !doc["ac"].is<JsonArrayConst>()) return false;

  radar.count = 0;
  radar.total = 0;
  for (JsonObjectConst item : doc["ac"].as<JsonArrayConst>()) {
    if (item["dst"].isNull()) continue;
    ++radar.total;
    Aircraft aircraft;
    aircraft.distanceKm = item["dst"].as<float>() * kNauticalMileKm;
    aircraft.bearing = item["dir"] | 0.0f;
    aircraft.altitudeM = static_cast<int32_t>((item["alt_baro"] | 0.0f) * kFootM);  // "ground" vira 0
    sscanf(item["flight"] | "", "%9s", aircraft.flight);
    snprintf(aircraft.type, sizeof(aircraft.type), "%s", item["t"] | "");

    // Inserção ordenada por distância; descarta as mais distantes quando a lista enche
    uint8_t position = radar.count;
    while (position > 0 && radar.items[position - 1].distanceKm > aircraft.distanceKm) --position;
    if (position >= kMaxAircraft) continue;
    if (radar.count < kMaxAircraft) ++radar.count;
    for (uint8_t i = radar.count - 1; i > position; --i) radar.items[i] = radar.items[i - 1];
    radar.items[position] = aircraft;
  }
  radar.valid = true;
  return true;
}

bool parseRepo(const char* json, RepoData& repo) {
  JsonDocument filter;
  filter["stargazers_count"] = filter["open_issues_count"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter)) || doc["stargazers_count"].isNull()) return false;
  repo.stars = doc["stargazers_count"];
  repo.openIssues = doc["open_issues_count"];  // inclui pull requests
  repo.valid = true;
  return true;
}

bool parseSearchCount(const char* json, uint32_t& count) {
  JsonDocument filter;
  filter["total_count"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter)) || doc["total_count"].isNull()) return false;
  count = doc["total_count"];
  return true;
}

bool parseWorkflowRun(const char* json, BuildState& build) {
  JsonDocument filter;
  filter["workflow_runs"][0]["status"] = filter["workflow_runs"][0]["conclusion"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, json, DeserializationOption::Filter(filter))) return false;
  JsonObjectConst run = doc["workflow_runs"][0];
  if (run.isNull()) return false;
  if (strcmp(run["status"] | "", "completed") != 0) {
    build = BuildState::Running;
  } else {
    build = strcmp(run["conclusion"] | "", "success") == 0 ? BuildState::Passed : BuildState::Failed;
  }
  return true;
}

}  // namespace parsers
