#pragma once

#include <LovyanGFX.hpp>

#include "app_config.h"
#include "board_pins.h"

// Display ILI9341V (SPI) + toque FT6336G (I2C) + backlight PWM do ES3C28P
class BoardDisplay : public lgfx::LGFX_Device {
 public:
  BoardDisplay() {
    {
      auto cfg = bus_.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.pin_sclk = pins::kLcdSclk;
      cfg.pin_mosi = pins::kLcdMosi;
      cfg.pin_miso = pins::kLcdMiso;
      cfg.pin_dc = pins::kLcdDc;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = pins::kLcdCs;
      cfg.pin_rst = -1;  // reset compartilhado com o ESP32-S3
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.invert = config::kDisplayInvertColors;
      cfg.bus_shared = false;
      panel_.config(cfg);
    }
    {
      auto cfg = light_.config();
      cfg.pin_bl = pins::kLcdBacklight;
      cfg.freq = 12000;
      cfg.pwm_channel = 7;
      light_.config(cfg);
      panel_.setLight(&light_);
    }
    {
      auto cfg = touch_.config();
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda = pins::kTouchSda;
      cfg.pin_scl = pins::kTouchScl;
      cfg.pin_rst = pins::kTouchReset;
      cfg.pin_int = pins::kTouchInterrupt;
      cfg.freq = 400000;
      cfg.x_max = 239;
      cfg.y_max = 319;
      cfg.offset_rotation = config::kTouchRotationOffset;
      cfg.bus_shared = false;
      touch_.config(cfg);
      panel_.setTouch(&touch_);
    }
    setPanel(&panel_);
  }

 private:
  lgfx::Bus_SPI bus_;
  lgfx::Panel_ILI9341 panel_;
  lgfx::Light_PWM light_;
  lgfx::Touch_FT5x06 touch_;
};
