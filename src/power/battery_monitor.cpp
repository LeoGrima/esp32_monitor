#include "battery_monitor.h"

#include <Arduino.h>

#include "app_config.h"
#include "battery_estimator.h"
#include "board_pins.h"

namespace battery {
namespace {

constexpr uint32_t kSampleIntervalMs = 2000;
constexpr uint8_t kReadingsPerSample = 8;

BatteryEstimator estimator;

float readVoltage() {
  uint32_t millivolts = 0;
  for (uint8_t i = 0; i < kReadingsPerSample; ++i) millivolts += analogReadMilliVolts(pins::kBatteryAdc);
  return millivolts / static_cast<float>(kReadingsPerSample) / 1000.0f * config::kBatteryDividerRatio;
}

}  // namespace

void begin() { pinMode(pins::kBatteryAdc, INPUT); }

bool update(BatteryData& data) {
  static uint32_t lastSampleMs = 0;
  if (lastSampleMs != 0 && millis() - lastSampleMs < kSampleIntervalMs) return false;
  lastSampleMs = millis();
  estimator.addSample(readVoltage(), lastSampleMs);
  data = estimator.data();
  return true;
}

}  // namespace battery
