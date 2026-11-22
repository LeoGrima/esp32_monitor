#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr int kThumbnailHeight = 146;
constexpr int kThumbnailSourceHeight = 180;  // mqdefault.jpg: 320x180
constexpr uint32_t kProgress = 0xFDBA8Cu;

const char* ageLabel(long long publishedAt, long long nowEpoch) {
  static char buffer[16];
  const long long seconds = nowEpoch - publishedAt;
  if (publishedAt <= 0 || seconds < 0) return "";
  if (seconds < 3600) snprintf(buffer, sizeof(buffer), "há %lld min", seconds / 60);
  else if (seconds < 86400) snprintf(buffer, sizeof(buffer), "há %lld h", seconds / 3600);
  else snprintf(buffer, sizeof(buffer), "há %lld d", seconds / 86400);
  return buffer;
}

}  // namespace

void drawVideoScreen(LovyanGFX& gfx, const VideoData& video, long long nowEpoch) {
  fillBackground(gfx, 0x14121Fu, 0x2A2036u);
  gfx.setClipRect(0, 0, gfx.width(), kThumbnailHeight);
  if (video.thumbnail != nullptr) {
    gfx.drawJpg(video.thumbnail, video.thumbnailSize, 0, (kThumbnailHeight - kThumbnailSourceHeight) / 2);
  } else {
    const int cx = gfx.width() / 2;
    const int cy = kThumbnailHeight / 2;
    gfx.fillSmoothRoundRect(cx - 30, cy - 21, 60, 42, 10, 0xE5383Bu);
    gfx.fillTriangle(cx - 8, cy - 11, cx - 8, cy + 11, cx + 12, cy, color::kText);
  }
  gfx.clearClipRect();

  const int contentWidth = gfx.width() - 2 * kMargin;
  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().medium);
  drawFittedText(gfx, video.valid ? video.title : "Aguardando vídeos", kMargin, kThumbnailHeight + 8, contentWidth);

  gfx.setFont(&fonts().small);
  gfx.setTextColor(color::kTextMuted);
  drawFittedText(gfx, video.channel, kMargin, kThumbnailHeight + 32, contentWidth);
  if (!video.valid) return;

  char text[16];
  gfx.drawString(ageLabel(video.publishedAt, nowEpoch), kMargin, kThumbnailHeight + 50);
  snprintf(text, sizeof(text), "%u/%u", video.index + 1, video.total);
  gfx.setTextDatum(lgfx::top_right);
  gfx.drawString(text, gfx.width() - kMargin, kThumbnailHeight + 50);

  drawProgressBar(gfx, kThumbnailHeight + 70, video.index, video.total, kProgress);
}

}  // namespace ui
