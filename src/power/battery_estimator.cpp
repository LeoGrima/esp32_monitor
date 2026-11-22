#include "battery_estimator.h"

#include <math.h>

namespace {

constexpr float kSmoothing = 0.1f;            // peso de cada nova leitura
constexpr float kMinRatePerHour = 1.0f;       // abaixo disso a carga é considerada estável
constexpr uint32_t kMinTrendWindowMs = 3UL * 60 * 1000;
constexpr float kMsPerHour = 3600000.0f;

struct CurvePoint {
  float voltage;
  float percent;
};
// Curva típica de descarga de uma célula LiPo de 3,7 V em repouso
constexpr CurvePoint kCurve[] = {{4.20f, 100}, {4.11f, 90}, {4.02f, 80}, {3.95f, 70}, {3.87f, 60}, {3.84f, 50},
                                 {3.80f, 40},  {3.77f, 30}, {3.73f, 20}, {3.69f, 10}, {3.27f, 0}};
constexpr int kCurveSize = sizeof(kCurve) / sizeof(kCurve[0]);

}  // namespace

float BatteryEstimator::percentFromVoltage(float voltage) {
  if (voltage >= kCurve[0].voltage) return 100;
  for (int i = 1; i < kCurveSize; ++i) {
    if (voltage < kCurve[i].voltage) continue;
    const float span = kCurve[i - 1].voltage - kCurve[i].voltage;
    return kCurve[i].percent + (voltage - kCurve[i].voltage) / span * (kCurve[i - 1].percent - kCurve[i].percent);
  }
  return 0;
}

void BatteryEstimator::addSample(float voltage, uint32_t nowMs) {
  voltage_ = voltage_ == 0 ? voltage : voltage_ + (voltage - voltage_) * kSmoothing;

  if (historyCount_ > 0 && nowMs - history_[historyCount_ - 1].timeMs < kHistoryIntervalMs) return;
  if (historyCount_ == kHistorySize) {
    for (uint8_t i = 1; i < kHistorySize; ++i) history_[i - 1] = history_[i];
    --historyCount_;
  }
  history_[historyCount_++] = {percentFromVoltage(voltage_), nowMs};
}

BatteryData BatteryEstimator::data() const {
  BatteryData result;
  if (voltage_ == 0) return result;
  const float percent = percentFromVoltage(voltage_);
  result.voltage = voltage_;
  result.percent = static_cast<uint8_t>(lroundf(percent));
  result.valid = true;

  const uint32_t window = historyCount_ > 0 ? history_[historyCount_ - 1].timeMs - history_[0].timeMs : 0;
  if (window < kMinTrendWindowMs) return result;  // ainda sem histórico para estimar

  result.ratePerHour = (history_[historyCount_ - 1].percent - history_[0].percent) / (window / kMsPerHour);
  if (result.ratePerHour >= kMinRatePerHour) {
    result.trend = BatteryTrend::Charging;
    result.minutesRemaining = static_cast<int32_t>((100 - percent) / result.ratePerHour * 60);
  } else if (result.ratePerHour <= -kMinRatePerHour) {
    result.trend = BatteryTrend::Discharging;
    result.minutesRemaining = static_cast<int32_t>(percent / -result.ratePerHour * 60);
  } else {
    result.trend = BatteryTrend::Stable;
  }
  return result;
}
