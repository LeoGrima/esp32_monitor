#pragma once

#include <LovyanGFX.hpp>

extern const uint8_t font_huge_data[];
extern const uint8_t font_big_data[];
extern const uint8_t font_large_data[];
extern const uint8_t font_title_data[];
extern const uint8_t font_medium_data[];
extern const uint8_t font_small_data[];

namespace ui {

struct Fonts {
  lgfx::VLWfont huge;    // somente números e símbolos
  lgfx::VLWfont big;     // somente números e símbolos
  lgfx::VLWfont large;
  lgfx::VLWfont title;
  lgfx::VLWfont medium;
  lgfx::VLWfont small;
};

const Fonts& fonts();

}  // namespace ui
