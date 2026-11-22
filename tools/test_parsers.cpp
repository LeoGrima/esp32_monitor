// Testa os parsers no PC com respostas de exemplo (ou reais, salvas em arquivo).
// Uso: test_parsers [pasta com weather.json, ptax.json, crypto.json, stock.json, news.xml, video.xml,
//                    stories.json, story.json, aircraft.json, repo.json]
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "data/data_parsers.h"
#include "power/battery_estimator.h"

namespace {

std::string load(const std::string& dir, const char* name, const char* fallback) {
  std::ifstream file(dir + "/" + name);
  if (dir.empty() || !file) return fallback;
  std::stringstream buffer;
  buffer << file.rdbuf();
  printf("[arquivo] %s\n", name);
  return buffer.str();
}

bool near(float a, float b) { return fabsf(a - b) < 0.01f; }

constexpr char kWeather[] = R"({"current":{"time":"2026-11-21T20:00","temperature_2m":27.3,"weather_code":2,"is_day":1},
"hourly":{"precipitation_probability":[0,5,10,45,20]},
"daily":{"temperature_2m_max":[31.2],"temperature_2m_min":[21.8]}})";

constexpr char kPtax[] = R"({"@odata.context":"x","value":[
{"cotacaoVenda":5.3000,"dataHoraCotacao":"2026-11-18 10:08:11.111","tipoBoletim":"Abertura"},
{"cotacaoVenda":5.3600,"dataHoraCotacao":"2026-11-18 13:08:11.111","tipoBoletim":"Fechamento"},
{"cotacaoVenda":5.4000,"dataHoraCotacao":"2026-11-19 10:08:11.111","tipoBoletim":"Abertura"},
{"cotacaoVenda":5.4136,"dataHoraCotacao":"2026-11-19 13:08:29.674","tipoBoletim":"Fechamento"}]})";

constexpr char kCrypto[] = R"({"bitcoin":{"brl":374918,"brl_24h_change":6.21},"ethereum":{"brl":19034.5,"brl_24h_change":-1.2}})";

constexpr char kStock[] = R"({"chart":{"result":[{"meta":{"currency":"BRL","symbol":"^BVSP",
"regularMarketPrice":132481.0,"chartPreviousClose":130858.0},"timestamp":[1],"indicators":{}}],"error":null}})";

constexpr char kNews[] = R"(<?xml version="1.0"?><rss><channel><title>g1</title>
<item><title><![CDATA[Prefeitura abre consulta sobre ônibus]]></title><description>x</description></item>
<item><title>Chuva &amp; vento: &quot;alerta&quot; &#8211; veja</title></item></channel></rss>)";

constexpr char kVideo[] = R"(<feed><title>Canal de Exemplo</title>
<entry><id>yt:video:abc123DEF45</id><yt:videoId>abc123DEF45</yt:videoId><title>Primeiro &amp; único</title>
<published>2026-11-17T15:00:12+00:00</published></entry>
<entry><yt:videoId>zzz999</yt:videoId><title>Segundo</title><published>1970-01-02T00:00:00+00:00</published></entry></feed>)";

constexpr char kStories[] = "[50582069,50593925,50593199,50593125]";
constexpr char kStory[] = R"({"by":"x","descendants":133,"id":1,"kids":[1,2],"score":538,"title":"Hello \"HN\"","type":"story"})";

constexpr char kAircraft[] = R"({"ac":[{"flight":"TAM3321 ","t":"A320","alt_baro":17000,"dst":17.0,"dir":40.5},
{"flight":"GLO1513 ","t":"B38M","alt_baro":"ground","dst":5.4,"dir":269.3},{"hex":"x"}],"total":3})";

constexpr char kRepo[] = R"({"full_name":"a/b","stargazers_count":15234,"open_issues_count":449,"forks_count":7})";

}  // namespace

int main(int argc, char** argv) {
  const std::string dir = argc > 1 ? argv[1] : "";
  const bool samples = dir.empty();

  Place place;
  assert(parsers::parseLocation(R"({"success":true,"city":"Curitiba","latitude":-25.4284,"longitude":-49.2733})", place));
  assert(strcmp(place.city, "Curitiba") == 0 && near(place.latitude, -25.4284f) && near(place.longitude, -49.2733f));
  assert(!place.hasUtcOffset);
  assert(parsers::parseLocation(R"({"success":true,"city":"Cuiabá","latitude":-15.6,"longitude":-56.1,
    "timezone":{"id":"America/Cuiaba","offset":-14400,"utc":"-04:00"}})", place));
  assert(place.hasUtcOffset && place.utcOffsetSeconds == -14400);
  char timezone[24];
  parsers::formatTimezone(-14400, timezone, sizeof(timezone));
  assert(strcmp(timezone, "<-0400>4:00") == 0);
  parsers::formatTimezone(19800, timezone, sizeof(timezone));
  assert(strcmp(timezone, "<+0530>-5:30") == 0);
  assert(!parsers::parseLocation(R"({"success":false,"message":"Invalid IP address"})", place));

  WeatherData weather;
  assert(parsers::parseWeather(load(dir, "weather.json", kWeather).c_str(), weather));
  printf("clima: %.1f graus (max %.1f, min %.1f), codigo %u, chuva %u%%, dia=%d\n", weather.temperature,
         weather.tempMax, weather.tempMin, weather.weatherCode, weather.rainProbability, weather.isDay);
  if (samples) assert(near(weather.temperature, 27.3f) && weather.rainProbability == 45 && weather.weatherCode == 2);

  CurrencyData currency;
  assert(parsers::parsePtax(load(dir, "ptax.json", kPtax).c_str(), currency.usd, currency.usdWeek,
                            &currency.usdWeekCount, currency.updatedAt));
  printf("dolar: %.4f (%+.2f%%), %u dias, PTAX %s\n", currency.usd.bid, currency.usd.pctChange,
         currency.usdWeekCount, currency.updatedAt);
  if (samples) {
    assert(near(currency.usd.bid, 5.4136f) && near(currency.usd.pctChange, 1.0f));
    assert(currency.usdWeekCount == 2 && strcmp(currency.updatedAt, "13:08") == 0);
  }

  CoinQuote coin;
  assert(parsers::parseCoin(load(dir, "crypto.json", kCrypto).c_str(), "bitcoin", "brl", coin));
  printf("bitcoin: %.0f (%+.2f%%)\n", coin.price, coin.pctChange24h);
  if (samples) assert(near(coin.price, 374918) && near(coin.pctChange24h, 6.21f));

  StockQuote stock;
  assert(parsers::parseStock(load(dir, "stock.json", kStock).c_str(), stock));
  printf("indice: %.0f (%+.2f%%)\n", stock.price, stock.pctChange);
  if (samples) assert(near(stock.pctChange, 1.24f));

  char titles[8][parsers::kFeedTitleSize];
  const uint8_t newsCount = parsers::parseNewsFeed(load(dir, "news.xml", kNews).c_str(), titles, 8);
  assert(newsCount > 0);
  for (uint8_t i = 0; i < newsCount; ++i) printf("noticia %u: %s\n", i + 1, titles[i]);
  if (samples) assert(newsCount == 2 && strcmp(titles[1], "Chuva & vento: \"alerta\" \xE2\x80\x93 veja") == 0);

  parsers::VideoEntry videos[5];
  char channel[40];
  const uint8_t videoCount = parsers::parseVideoFeed(load(dir, "video.xml", kVideo).c_str(), videos, 5, channel, sizeof(channel));
  assert(videoCount > 0);
  printf("canal: %s\n", channel);
  for (uint8_t i = 0; i < videoCount; ++i) printf("video %u: [%s] %s (%lld)\n", i + 1, videos[i].id, videos[i].title, videos[i].publishedAt);
  if (samples) {
    assert(videoCount == 2 && strcmp(channel, "Canal de Exemplo") == 0 && videos[1].publishedAt == 86400);
    assert(strcmp(videos[0].title, "Primeiro & único") == 0 && videos[0].publishedAt == 1794927612);
  }
  uint32_t ids[3];
  assert(parsers::parseStoryIds(load(dir, "stories.json", kStories).c_str(), ids, 3) == 3);
  char title[kNewsTitleSize];
  uint32_t score = 0;
  uint32_t comments = 0;
  assert(parsers::parseStory(load(dir, "story.json", kStory).c_str(), title, sizeof(title), score, comments));
  printf("hacker news: %u, %u... | %s (%u pontos, %u comentarios)\n", ids[0], ids[1], title, score, comments);
  if (samples) assert(ids[2] == 50593199 && score == 538 && comments == 133 && strcmp(title, "Hello \"HN\"") == 0);

  RadarData radar;
  assert(parsers::parseAircraft(load(dir, "aircraft.json", kAircraft).c_str(), radar));
  printf("radar: %u aeronaves", radar.total);
  for (uint8_t i = 0; i < radar.count; ++i) {
    printf(" | %s %s %.1f km %ld m", radar.items[i].flight, radar.items[i].type, radar.items[i].distanceKm,
           static_cast<long>(radar.items[i].altitudeM));
  }
  puts("");
  if (samples) {
    assert(radar.total == 2 && radar.count == 2 && strcmp(radar.items[0].flight, "GLO1513") == 0);
    assert(near(radar.items[0].distanceKm, 10.0f) && radar.items[0].altitudeM == 0 && radar.items[1].altitudeM == 5181);
  }

  RepoData repo;
  assert(parsers::parseRepo(load(dir, "repo.json", kRepo).c_str(), repo));
  uint32_t pulls = 0;
  BuildState build = BuildState::Unknown;
  assert(parsers::parseSearchCount(R"({"total_count":37,"items":[]})", pulls) && pulls == 37);
  assert(parsers::parseWorkflowRun(R"({"workflow_runs":[{"status":"completed","conclusion":"failure"}]})", build));
  assert(build == BuildState::Failed);
  printf("repositorio: %u estrelas, %u issues\n", repo.stars, repo.openIssues);
  if (samples) assert(repo.stars == 15234);

  // Bateria: 30 minutos descarregando de 4,02 V (80%) para 3,95 V (70%)
  BatteryEstimator estimator;
  assert(near(BatteryEstimator::percentFromVoltage(4.3f), 100) && near(BatteryEstimator::percentFromVoltage(3.0f), 0));
  assert(near(BatteryEstimator::percentFromVoltage(3.84f), 50));
  for (uint32_t second = 0; second <= 1800; second += 2) {
    estimator.addSample(4.02f - 0.07f * second / 1800.0f, second * 1000);
  }
  const BatteryData batteryData = estimator.data();
  printf("bateria: %.2f V, %u%%, %.1f %%/h, %ld min restantes\n", batteryData.voltage, batteryData.percent,
         batteryData.ratePerHour, static_cast<long>(batteryData.minutesRemaining));
  assert(batteryData.trend == BatteryTrend::Discharging && batteryData.percent >= 69 && batteryData.percent <= 71);
  assert(batteryData.ratePerHour < -15 && batteryData.ratePerHour > -25);
  assert(batteryData.minutesRemaining > 170 && batteryData.minutesRemaining < 280);

  puts("OK");
  return 0;
}
