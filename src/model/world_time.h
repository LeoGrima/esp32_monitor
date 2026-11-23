#pragma once

#include <stdint.h>

#include "dashboard_data.h"

// Cálculo de data e fuso sem depender do fuso configurado no sistema
namespace world_time {

struct CivilDate {
  int year;
  int month;  // 1 a 12
  int day;
};

long long daysFromCivil(int year, int month, int day);  // dias desde 01/01/1970
CivilDate civilFromDays(long long days);

// Deslocamento em relação ao UTC no instante informado, já com o horário de verão da cidade
int32_t utcOffsetSeconds(const CityZone& zone, long long epochUtc);

}  // namespace world_time
