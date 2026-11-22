#include <stdio.h>

#include "fonts/ui_fonts.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_widgets.h"

namespace ui {
namespace {

struct BuildLabel {
  const char* text;
  uint32_t tint;
};

BuildLabel buildLabel(const RepoData& repo) {
  if (!repo.valid) return {"Sem dados", color::kTextMuted};
  switch (repo.build) {
    case BuildState::Passed:  return {"CI passou", color::kPositive};
    case BuildState::Failed:  return {"CI falhou", color::kNegative};
    case BuildState::Running: return {"CI rodando", color::kNeutral};
    default:                  return {"", color::kTextMuted};
  }
}

void drawStar(LovyanGFX& gfx, int x, int y, int size, uint32_t tint) {
  gfx.fillTriangle(x, y - size, x - size * 3 / 5, y + size * 4 / 5, x + size * 3 / 8, y + size / 5, tint);
  gfx.fillTriangle(x, y - size, x + size * 3 / 5, y + size * 4 / 5, x - size * 3 / 8, y + size / 5, tint);
  gfx.fillTriangle(x - size, y - size / 4, x + size, y - size / 4, x, y + size * 2 / 5, tint);
}

}  // namespace

void drawRepoScreen(LovyanGFX& gfx, const RepoData& repo) {
  const BuildLabel build = buildLabel(repo);
  fillBackground(gfx, 0x0D1117u, 0x1D2B4Fu);
  drawStars(gfx, 20);
  drawHeader(gfx, repo.name, build.text, build.tint, true);

  gfx.setTextDatum(lgfx::top_left);
  gfx.setTextColor(color::kText);
  gfx.setFont(&fonts().big);
  gfx.drawString(repo.valid ? formatThousands(repo.stars) : "-", kMargin + 44, kHeaderBottom + 22);
  drawGlow(gfx, kMargin + 16, kHeaderBottom + 46, 26, color::kSun);
  drawStar(gfx, kMargin + 16, kHeaderBottom + 46, 15, color::kSun);
  gfx.setFont(&fonts().medium);
  gfx.setTextColor(color::kTextMuted);
  gfx.drawString("estrelas", kMargin + 46, kHeaderBottom + 80);
  if (!repo.valid) return;

  FooterItem items[2] = {{"PRs abertos", nullptr, "", color::kCalm}, {"Issues", nullptr, "", color::kCalm}};
  snprintf(items[0].value, sizeof(items[0].value), "%lu", static_cast<unsigned long>(repo.openPullRequests));
  snprintf(items[1].value, sizeof(items[1].value), "%lu", static_cast<unsigned long>(repo.openIssues));
  drawFooterColumns(gfx, items, 2);
}

}  // namespace ui
