#pragma once

#include "../model/dashboard_data.h"

// Busca os dados de cada tela na internet. Retornam false se a atualização falhar;
// nesse caso o conteúdo anterior é mantido.
namespace services {

// Local usado pelo clima e pelo radar: o de app_config.h até ser detectado pela rede
const Place& currentPlace();
void setPlace(const Place& place);
bool detectPlace(Place& place);

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
