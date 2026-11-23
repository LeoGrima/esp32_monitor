#include "world_time.h"

namespace world_time {
namespace {

constexpr long long kSecondsPerDay = 86400;
constexpr int kSecondsPerHour = 3600;
constexpr int kThursday = 4;  // 01/01/1970 foi quinta-feira (0 = domingo)

int weekday(long long days) { return static_cast<int>(((days % 7) + 7 + kThursday) % 7); }

// Dia (desde 1970) do n-ésimo domingo do mês; n = 0 devolve o último domingo
long long sundayOfMonth(int year, int month, int nth) {
  if (nth == 0) {
    const long long lastDay = daysFromCivil(month == 12 ? year + 1 : year, month == 12 ? 1 : month + 1, 1) - 1;
    return lastDay - weekday(lastDay);
  }
  const long long firstDay = daysFromCivil(year, month, 1);
  return firstDay + (7 - weekday(firstDay)) % 7 + (nth - 1) * 7;
}

bool isDaylightTime(const CityZone& zone, long long epochUtc) {
  const int standardOffset = zone.utcOffsetMinutes * 60;
  const int year = civilFromDays((epochUtc + standardOffset) / kSecondsPerDay).year;
  switch (zone.dst) {
    case DstRule::UnitedStates: {
      // Do 2º domingo de março ao 1º domingo de novembro, às 2h locais
      const long long start = sundayOfMonth(year, 3, 2) * kSecondsPerDay + 2 * kSecondsPerHour - standardOffset;
      const long long end = sundayOfMonth(year, 11, 1) * kSecondsPerDay + kSecondsPerHour - standardOffset;
      return epochUtc >= start && epochUtc < end;
    }
    case DstRule::Europe: {
      // Do último domingo de março ao último domingo de outubro, à 1h UTC
      const long long start = sundayOfMonth(year, 3, 0) * kSecondsPerDay + kSecondsPerHour;
      const long long end = sundayOfMonth(year, 10, 0) * kSecondsPerDay + kSecondsPerHour;
      return epochUtc >= start && epochUtc < end;
    }
    default:
      return false;
  }
}

}  // namespace

// Algoritmos civis de Howard Hinnant
long long daysFromCivil(int year, int month, int day) {
  year -= month <= 2;
  const long long era = (year >= 0 ? year : year - 399) / 400;
  const long long yearOfEra = year - era * 400;
  const long long dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const long long dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
  return era * 146097 + dayOfEra - 719468;
}

CivilDate civilFromDays(long long days) {
  days += 719468;
  const long long era = (days >= 0 ? days : days - 146096) / 146097;
  const long long dayOfEra = days - era * 146097;
  const long long yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
  const long long dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
  const long long monthIndex = (5 * dayOfYear + 2) / 153;
  const int month = static_cast<int>(monthIndex < 10 ? monthIndex + 3 : monthIndex - 9);
  return {static_cast<int>(yearOfEra + era * 400 + (month <= 2)), month,
          static_cast<int>(dayOfYear - (153 * monthIndex + 2) / 5 + 1)};
}

int32_t utcOffsetSeconds(const CityZone& zone, long long epochUtc) {
  return zone.utcOffsetMinutes * 60 + (isDaylightTime(zone, epochUtc) ? kSecondsPerHour : 0);
}

}  // namespace world_time
