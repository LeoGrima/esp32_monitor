#pragma once

#include "../model/dashboard_data.h"

// Busca os dados de cada tela na internet. Retornam false se a atualização falhar;
// nesse caso o conteúdo anterior é mantido.
namespace services {

bool fetchWeather(WeatherData& weather);
bool fetchCurrency(CurrencyData& currency);
bool fetchCrypto(CryptoData& crypto);
bool fetchStocks(StocksData& stocks);
bool fetchNews(NewsData& news);
bool fetchVideo(VideoData& video);
bool fetchHackerNews(NewsData& news);
bool fetchMatch(MatchData& match);
bool fetchRadar(RadarData& radar);
bool fetchRepo(RepoData& repo);
bool fetchStatus(StatusData& status);
bool fetchSites(StatusData& sites);

}  // namespace services
