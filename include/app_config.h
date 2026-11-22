#pragma once

#include <stdint.h>

#include "ui/ui_screens.h"

// Ajustes do painel. Credenciais de Wi-Fi ficam em secrets.h.
namespace config {

// Telas exibidas, na ordem de rotação (remova as que não quiser)
constexpr ui::Screen kScreens[] = {
    ui::Screen::Clock, ui::Screen::Weather, ui::Screen::Currency,   ui::Screen::Crypto, ui::Screen::Stocks,
    ui::Screen::Match, ui::Screen::News,    ui::Screen::HackerNews, ui::Screen::Video,  ui::Screen::Radar,
    ui::Screen::Repo,  ui::Screen::Status,  ui::Screen::Services,   ui::Screen::Fire};
constexpr uint32_t kScreenRotationSeconds = 15;  // 0 = troca apenas pelo toque

// Tela
constexpr uint8_t kDisplayRotation = 1;  // paisagem: 1 ou 3 (invertida)
constexpr bool kDisplayInvertColors = true;  // painel IPS; troque se as cores saírem negativas
constexpr uint8_t kBrightness = 200;     // 0 a 255

// Data e hora
constexpr char kTimezone[] = "<-03>3";  // horário de Brasília (formato POSIX)
constexpr char kNtpServer[] = "a.st1.ntp.br";
constexpr bool kSouthernHemisphere = true;

// Local: usado pelo clima (Open-Meteo) e pelo radar de aeronaves (adsb.lol)
constexpr char kCityName[] = "São Paulo";
constexpr float kLatitude = -23.5505f;
constexpr float kLongitude = -46.6333f;

// Cripto (CoinGecko): a primeira moeda fica em destaque
struct Coin {
  const char* id;
  const char* symbol;
};
constexpr Coin kCoins[] = {{"bitcoin", "BTC"}, {"ethereum", "ETH"}, {"solana", "SOL"}, {"ripple", "XRP"}};
constexpr char kCryptoCurrency[] = "brl";
constexpr char kCryptoCurrencyLabel[] = "R$";

// Bolsa (Yahoo Finance): o primeiro ativo fica em destaque
struct Stock {
  const char* symbol;  // codificado para URL: ^ = %5E; ações da B3 terminam em .SA (ex.: PETR4.SA)
  const char* label;
  bool isIndex;
};
constexpr Stock kStocks[] = {{"%5EBVSP", "IBOVESPA", true}, {"%5EGSPC", "S&P 500", true}, {"%5EIXIC", "Nasdaq", true}};

// Notícias (RSS)
constexpr char kNewsFeedUrl[] = "https://g1.globo.com/rss/g1/";
constexpr char kNewsSource[] = "g1";

// YouTube: ID do canal (começa com "UC"); vazio desativa a tela
constexpr char kYoutubeChannelId[] = "";

// Jogo do time (ESPN): bra.1 = Brasileirão, eng.1 = Premier League, esp.1 = La Liga, uefa.champions = Champions
constexpr char kLeagueCode[] = "bra.1";
constexpr char kLeagueName[] = "Brasileirão";
constexpr char kFavoriteTeam[] = "";  // nome ou sigla (ex.: "Flamengo" ou "FLA"); vazio = primeiro jogo da rodada

// Radar
constexpr float kRadarRadiusKm = 40;

// Repositório (GitHub, público): dono/nome
constexpr char kGithubRepo[] = "espressif/arduino-esp32";

// Páginas de status (padrão Statuspage) e serviços monitorados (https), até 4 de cada
struct Endpoint {
  const char* label;
  const char* url;
};
constexpr Endpoint kStatusPages[] = {{"GitHub", "https://www.githubstatus.com"},
                                     {"Cloudflare", "https://www.cloudflarestatus.com"},
                                     {"Vercel", "https://www.vercel-status.com"},
                                     {"OpenAI", "https://status.openai.com"}};
constexpr Endpoint kMonitoredSites[] = {{"Google", "https://www.google.com"}, {"GitHub", "https://github.com"}};
constexpr uint32_t kSlowResponseMs = 3000;  // inclui a negociação TLS feita pelo ESP32

// Intervalos de atualização
constexpr uint32_t kMinuteMs = 60UL * 1000;
constexpr uint32_t kWeatherIntervalMs = 15 * kMinuteMs;
constexpr uint32_t kCurrencyIntervalMs = 10 * kMinuteMs;
constexpr uint32_t kCryptoIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kStocksIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kNewsIntervalMs = 2 * kMinuteMs;   // avança uma manchete por ciclo
constexpr uint32_t kVideoIntervalMs = 2 * kMinuteMs;  // avança um vídeo por ciclo
constexpr uint32_t kHackerNewsIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kMatchIntervalMs = 1 * kMinuteMs;
constexpr uint32_t kRadarIntervalMs = 1 * kMinuteMs;
constexpr uint32_t kRepoIntervalMs = 10 * kMinuteMs;  // a API sem token permite 60 consultas por hora
constexpr uint32_t kStatusIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kServicesIntervalMs = 2 * kMinuteMs;
constexpr uint32_t kRetryIntervalMs = 30UL * 1000;

}  // namespace config
