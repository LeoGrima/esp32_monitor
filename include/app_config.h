#pragma once

#include <stdint.h>

#include "ui/ui_screens.h"

// Ajustes do painel. Credenciais de Wi-Fi ficam em secrets.h.
namespace config {

// Telas exibidas, na ordem de rotação (remova as que não quiser)
constexpr ui::Screen kScreens[] = {
    ui::Screen::Clock, ui::Screen::Weather, ui::Screen::Currency,   ui::Screen::Crypto, ui::Screen::Stocks,
    ui::Screen::News,  ui::Screen::HackerNews, ui::Screen::Video, ui::Screen::Battery};
constexpr uint32_t kScreenRotationSeconds = 15;  // 0 = troca apenas pelo toque

// Tela
constexpr uint8_t kDisplayRotation = 1;  // paisagem: 1 ou 3 (invertida)
constexpr bool kDisplayInvertColors = true;  // painel IPS; troque se as cores saírem negativas
constexpr uint8_t kTouchRotationOffset = 2;  // 0 a 3: alinha o toque com a imagem (2 = 180 graus)
constexpr uint8_t kBrightness = 200;     // 0 a 255

// Bateria: a tensão chega ao ADC por um divisor resistivo. Se a tensão exibida não bater com a de um
// multímetro, ajuste: novo valor = valor atual x (tensão real / tensão exibida)
constexpr float kBatteryDividerRatio = 2.0f;

// Data e hora
constexpr char kTimezone[] = "<-04>4";  // reserva: Cuiabá/MT, UTC-4 (formato POSIX; Brasília seria "<-03>3")
constexpr char kNtpServer[] = "a.st1.ntp.br";
constexpr bool kSouthernHemisphere = true;

// Local: usado pelo clima (Open-Meteo).
// Com a detecção ligada, cidade, coordenadas e fuso horário vêm do endereço de rede (ipwho.is);
// os valores deste arquivo valem enquanto a detecção não responde ou se ela estiver desligada.
constexpr bool kDetectLocation = true;
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

// Intervalos de atualização
constexpr uint32_t kMinuteMs = 60UL * 1000;
constexpr uint32_t kWeatherIntervalMs = 15 * kMinuteMs;
constexpr uint32_t kCurrencyIntervalMs = 10 * kMinuteMs;
constexpr uint32_t kCryptoIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kStocksIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kNewsIntervalMs = 2 * kMinuteMs;   // avança uma manchete por ciclo
constexpr uint32_t kVideoIntervalMs = 2 * kMinuteMs;  // avança um vídeo por ciclo
constexpr uint32_t kHackerNewsIntervalMs = 5 * kMinuteMs;
constexpr uint32_t kRetryIntervalMs = 30UL * 1000;

}  // namespace config
