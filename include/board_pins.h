#pragma once

// Pinagem do módulo ES3C28P (ESP32-S3, IPS 2.8" ILI9341V, toque FT6336G) - ver docs/hardware.md
namespace pins {

constexpr int kLcdCs = 10;
constexpr int kLcdDc = 46;
constexpr int kLcdSclk = 12;
constexpr int kLcdMosi = 11;
constexpr int kLcdMiso = 13;
constexpr int kLcdBacklight = 45;

constexpr int kTouchSda = 16;
constexpr int kTouchScl = 15;
constexpr int kTouchReset = 18;
constexpr int kTouchInterrupt = 17;

constexpr int kAudioAmpEnable = 1;  // ativo em nível baixo
constexpr int kBootButton = 0;

}  // namespace pins
