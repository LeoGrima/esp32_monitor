#pragma once

#include <stdint.h>

namespace ui {

// Cores em RGB888
namespace color {
constexpr uint32_t kText = 0xFFFFFFu;
constexpr uint32_t kTextMuted = 0xC9D4E8u;
constexpr uint32_t kPositive = 0x4ADE80u;
constexpr uint32_t kNegative = 0xF87171u;
constexpr uint32_t kNeutral = 0xFBBF24u;
constexpr uint32_t kCalm = 0x93C5FDu;
constexpr uint32_t kPanel = 0x0B1020u;
constexpr uint32_t kSun = 0xFFD54Au;
constexpr uint32_t kMoon = 0xE8ECF5u;
constexpr uint32_t kCloud = 0xF1F5FBu;
constexpr uint32_t kCloudDark = 0x9AA7BDu;
constexpr uint32_t kRain = 0x9CCBFFu;
constexpr uint32_t kNightTop = 0x070B24u;
constexpr uint32_t kNightBottom = 0x2A3A78u;
constexpr uint32_t kDayTop = 0x1E7FD6u;
constexpr uint32_t kDayBottom = 0x9AD4F5u;
}  // namespace color

constexpr int kMargin = 14;
constexpr int kHeaderBottom = 34;
constexpr int kPanelInset = 10;
constexpr int kFooterHeight = 30;
constexpr int kFooterTallHeight = 46;
constexpr int kFooterBottomGap = 12;
constexpr uint8_t kPanelAlpha = 160;

}  // namespace ui
