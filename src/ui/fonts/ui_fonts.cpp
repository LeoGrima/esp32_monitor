#include "ui_fonts.h"

namespace ui {

const Fonts& fonts() {
  static Fonts instance;
  static lgfx::PointerWrapper sources[] = {
      lgfx::PointerWrapper(font_huge_data), lgfx::PointerWrapper(font_big_data),
      lgfx::PointerWrapper(font_large_data), lgfx::PointerWrapper(font_title_data),
      lgfx::PointerWrapper(font_medium_data),
      lgfx::PointerWrapper(font_small_data)};
  static bool loaded = false;
  if (!loaded) {
    lgfx::VLWfont* targets[] = {&instance.huge,  &instance.big,    &instance.large,
                                &instance.title, &instance.medium, &instance.small};
    for (size_t i = 0; i < 6; ++i) targets[i]->loadFont(&sources[i]);
    loaded = true;
  }
  return instance;
}

}  // namespace ui
