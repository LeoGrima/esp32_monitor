#pragma once

#include <stdint.h>

#include "../model/dashboard_data.h"

// Estima carga, tendência e tempo restante a partir da tensão da bateria. Sem dependência de hardware.
class BatteryEstimator {
 public:
  // Deve ser chamado em intervalos regulares com a tensão medida (V)
  void addSample(float voltage, uint32_t nowMs);
  BatteryData data() const;

  static float percentFromVoltage(float voltage);

 private:
  static constexpr uint8_t kHistorySize = 10;
  static constexpr uint32_t kHistoryIntervalMs = 60UL * 1000;

  struct Point {
    float percent;
    uint32_t timeMs;
  };

  float voltage_ = 0;
  Point history_[kHistorySize] = {};
  uint8_t historyCount_ = 0;
};
