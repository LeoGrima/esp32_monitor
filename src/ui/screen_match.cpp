#include <stdio.h>
#include <string.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

constexpr uint32_t kLive = 0xFF5A4Fu;
constexpr uint32_t kLine = 0x4FA86Bu;
constexpr int kScoreY = 84;

const char* kickoffLabel(long long startsAt) {
  static char buffer[20];
  const time_t start = static_cast<time_t>(startsAt);
  tm parts;
  localtime_r(&start, &parts);
  snprintf(buffer, sizeof(buffer), "%02d/%02d %02d:%02d", parts.tm_mday, parts.tm_mon + 1, parts.tm_hour, parts.tm_min);
  return buffer;
}

const char* stateLabel(const MatchInfo& match) {
  if (match.state == MatchState::Finished) return "Encerrado";
  if (match.state == MatchState::Scheduled) return kickoffLabel(match.startsAt);
  return strcmp(match.clock, "HT") == 0 ? "Intervalo" : match.clock;
}

void drawPitch(LovyanGFX& gfx) {
  fillBackground(gfx, 0x0A3319u, 0x1C6E36u);
  const int cx = gfx.width() / 2;
  gfx.drawFastVLine(cx, kHeaderBottom, gfx.height() - kHeaderBottom - kFooterBottomGap - kFooterTallHeight - 6, kLine);
  gfx.drawCircle(cx, kScoreY + 6, 46, kLine);
}

void drawSide(LovyanGFX& gfx, const MatchSide& side, int x) {
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().large);
  gfx.drawString(side.abbreviation, x, kScoreY);
  gfx.setFont(&fonts().small);
  gfx.setTextColor(color::kTextMuted);
  gfx.drawString(side.name, x, kScoreY + 28);
}

}  // namespace

void drawMatchScreen(LovyanGFX& gfx, const MatchData& data) {
  const MatchInfo& match = data.main;
  drawPitch(gfx);
  if (!match.valid) {
    drawHeader(gfx, "Jogo do time", "Sem dados", color::kTextMuted);
    return;
  }

  const bool live = match.state == MatchState::Live;
  char text[32];
  snprintf(text, sizeof(text), live ? "AO VIVO %s" : "%s", stateLabel(match));
  drawHeader(gfx, data.league, text, live ? kLive : color::kText, live);

  const int cx = gfx.width() / 2;
  drawSide(gfx, match.home, 50);
  drawSide(gfx, match.away, gfx.width() - 50);
  gfx.setTextDatum(lgfx::middle_center);
  gfx.setTextColor(color::kText);
  if (match.state == MatchState::Scheduled) {
    gfx.setFont(&fonts().large);
    gfx.drawString("x", cx, kScoreY);
  } else {
    gfx.setFont(&fonts().huge);
    snprintf(text, sizeof(text), "%u", match.home.score);
    gfx.drawString(text, cx - 40, kScoreY + 4);
    snprintf(text, sizeof(text), "%u", match.away.score);
    gfx.drawString(text, cx + 40, kScoreY + 4);
  }

  FooterItem items[kMaxOtherMatches];
  char labels[kMaxOtherMatches][24];
  for (uint8_t i = 0; i < data.otherCount; ++i) {
    const MatchInfo& other = data.others[i];
    if (other.state == MatchState::Scheduled) {
      snprintf(labels[i], sizeof(labels[i]), "%s x %s", other.home.abbreviation, other.away.abbreviation);
    } else {
      snprintf(labels[i], sizeof(labels[i]), "%s %u x %u %s", other.home.abbreviation, other.home.score,
               other.away.score, other.away.abbreviation);
    }
    items[i] = {labels[i], nullptr, "", other.state == MatchState::Live ? kLive : color::kTextMuted};
    snprintf(items[i].value, sizeof(items[i].value), "%s", stateLabel(other));
  }
  drawFooterColumns(gfx, items, data.otherCount);
}

}  // namespace ui
