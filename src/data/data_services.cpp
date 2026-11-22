#include "data_services.h"

#include <time.h>

#include <Arduino.h>

#include "app_config.h"
#include "data_parsers.h"
#include "http_client.h"

namespace services {
namespace {

constexpr uint8_t kPtaxLookbackDays = 14;  // cobre fins de semana e feriados
constexpr uint8_t kNewsPoolSize = 8;
constexpr uint8_t kVideoPoolSize = 5;
constexpr uint8_t kStoryPoolSize = 5;
constexpr size_t kThumbnailCapacity = 64 * 1024;

bool fetchPtax(const char* currencyCode, CurrencyQuote& quote, float* week, uint8_t* weekCount, char* updatedAt) {
  char dateFrom[11];
  char dateTo[11];
  const time_t now = time(nullptr);
  const time_t start = now - kPtaxLookbackDays * 86400L;
  tm parts;
  strftime(dateTo, sizeof(dateTo), "%m-%d-%Y", localtime_r(&now, &parts));
  strftime(dateFrom, sizeof(dateFrom), "%m-%d-%Y", localtime_r(&start, &parts));

  char url[360];
  snprintf(url, sizeof(url),
           "https://olinda.bcb.gov.br/olinda/servico/PTAX/versao/v1/odata/"
           "CotacaoMoedaPeriodo(moeda=@moeda,dataInicial=@dataInicial,dataFinalCotacao=@dataFinalCotacao)"
           "?@moeda=%%27%s%%27&@dataInicial=%%27%s%%27&@dataFinalCotacao=%%27%s%%27"
           "&$format=json&$select=cotacaoVenda,dataHoraCotacao&$orderby=dataHoraCotacao%%20asc",
           currencyCode, dateFrom, dateTo);
  String body;
  return http::get(url, body) && parsers::parsePtax(body.c_str(), quote, week, weekCount, updatedAt);
}

Place& place() {
  static Place instance = [] {
    Place fallback;
    snprintf(fallback.city, sizeof(fallback.city), "%s", config::kCityName);
    fallback.latitude = config::kLatitude;
    fallback.longitude = config::kLongitude;
    return fallback;
  }();
  return instance;
}

}  // namespace

const Place& currentPlace() { return place(); }

void setPlace(const Place& detected) { place() = detected; }

bool detectPlace(Place& detected) {
  String body;
  return http::get("https://ipwho.is/?fields=success,city,latitude,longitude,timezone", body) &&
         parsers::parseLocation(body.c_str(), detected);
}

bool fetchWeather(WeatherData& weather) {
  char url[320];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,weather_code,is_day&hourly=precipitation_probability&forecast_hours=12"
           "&daily=temperature_2m_max,temperature_2m_min&forecast_days=1&timezone=auto",
           place().latitude, place().longitude);
  String body;
  return http::get(url, body) && parsers::parseWeather(body.c_str(), weather);
}

bool fetchCurrency(CurrencyData& currency) {
  const bool usdOk = fetchPtax("USD", currency.usd, currency.usdWeek, &currency.usdWeekCount, currency.updatedAt);
  const bool eurOk = fetchPtax("EUR", currency.eur, nullptr, nullptr, nullptr);
  return usdOk && eurOk;
}

bool fetchCrypto(CryptoData& crypto) {
  String url = "https://api.coingecko.com/api/v3/simple/price?include_24hr_change=true&vs_currencies=";
  url += config::kCryptoCurrency;
  url += "&ids=";
  for (const config::Coin& coin : config::kCoins) {
    url += coin.id;
    url += ',';
  }
  String body;
  if (!http::get(url.c_str(), body)) return false;

  crypto.currencyLabel = config::kCryptoCurrencyLabel;
  crypto.count = 0;
  for (const config::Coin& coin : config::kCoins) {
    if (crypto.count == kMaxCoins) break;
    CoinQuote& quote = crypto.coins[crypto.count++];
    quote.symbol = coin.symbol;
    parsers::parseCoin(body.c_str(), coin.id, config::kCryptoCurrency, quote);
  }
  return crypto.coins[0].valid;
}

bool fetchStocks(StocksData& stocks) {
  stocks.count = 0;
  for (const config::Stock& stock : config::kStocks) {
    if (stocks.count == kMaxStocks) break;
    StockQuote& quote = stocks.items[stocks.count++];
    quote.label = stock.label;
    quote.isIndex = stock.isIndex;

    char url[128];
    snprintf(url, sizeof(url), "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d&range=1d",
             stock.symbol);
    String body;
    if (http::get(url, body)) parsers::parseStock(body.c_str(), quote);
  }
  return stocks.items[0].valid;
}

bool fetchNews(NewsData& news) {
  static char pool[kNewsPoolSize][parsers::kFeedTitleSize];
  String body;
  if (!http::get(config::kNewsFeedUrl, body)) return false;
  const uint8_t total = parsers::parseNewsFeed(body.c_str(), pool, kNewsPoolSize);
  if (total == 0) return false;

  news.index = news.valid ? (news.index + 1) % total : 0;
  news.total = total;
  for (uint8_t i = 0; i < kNewsVisible; ++i) {
    const char* title = i < total ? pool[(news.index + i) % total] : "";
    snprintf(news.headlines[i], sizeof(news.headlines[i]), "%s", title);
  }
  news.valid = true;
  return true;
}

bool fetchVideo(VideoData& video) {
  // Dois buffers alternados: a miniatura em exibição nunca é sobrescrita
  static uint8_t* buffers[2] = {nullptr, nullptr};
  static uint8_t activeBuffer = 0;
  if (buffers[0] == nullptr) {
    buffers[0] = static_cast<uint8_t*>(ps_malloc(kThumbnailCapacity));
    buffers[1] = static_cast<uint8_t*>(ps_malloc(kThumbnailCapacity));
    if (buffers[0] == nullptr || buffers[1] == nullptr) return false;
  }

  char url[128];
  snprintf(url, sizeof(url), "https://www.youtube.com/feeds/videos.xml?channel_id=%s", config::kYoutubeChannelId);
  String body;
  if (!http::get(url, body)) return false;

  parsers::VideoEntry entries[kVideoPoolSize];
  const uint8_t total = parsers::parseVideoFeed(body.c_str(), entries, kVideoPoolSize, video.channel, sizeof(video.channel));
  if (total == 0) return false;

  video.index = video.valid ? (video.index + 1) % total : 0;
  video.total = total;
  const parsers::VideoEntry& entry = entries[video.index];
  snprintf(video.title, sizeof(video.title), "%s", entry.title);
  video.publishedAt = entry.publishedAt;

  const uint8_t nextBuffer = activeBuffer ^ 1;
  snprintf(url, sizeof(url), "https://i.ytimg.com/vi/%s/mqdefault.jpg", entry.id);
  const size_t size = http::download(url, buffers[nextBuffer], kThumbnailCapacity);
  if (size > 0) {
    activeBuffer = nextBuffer;
    video.thumbnail = buffers[activeBuffer];
    video.thumbnailSize = size;
  } else {
    video.thumbnail = nullptr;
    video.thumbnailSize = 0;
  }
  video.valid = true;
  return true;
}

bool fetchHackerNews(NewsData& news) {
  uint32_t ids[kStoryPoolSize];
  String body;
  if (!http::get("https://hacker-news.firebaseio.com/v0/topstories.json", body)) return false;
  const uint8_t total = parsers::parseStoryIds(body.c_str(), ids, kStoryPoolSize);
  if (total == 0) return false;

  news.index = news.valid ? (news.index + 1) % total : 0;
  news.total = total;
  for (uint8_t i = 0; i < kNewsVisible; ++i) {
    char url[80];
    uint32_t score = 0;
    uint32_t comments = 0;
    news.headlines[i][0] = '\0';
    snprintf(url, sizeof(url), "https://hacker-news.firebaseio.com/v0/item/%lu.json",
             static_cast<unsigned long>(ids[(news.index + i) % total]));
    if (i >= total || !http::get(url, body) ||
        !parsers::parseStory(body.c_str(), news.headlines[i], sizeof(news.headlines[i]), score, comments)) {
      continue;
    }
    if (i == 0) {
      snprintf(news.detail, sizeof(news.detail), "%lu pontos · %lu comentários", static_cast<unsigned long>(score),
               static_cast<unsigned long>(comments));
    }
  }
  news.valid = news.headlines[0][0] != '\0';
  return news.valid;
}

}  // namespace services
