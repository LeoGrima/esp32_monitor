#!/usr/bin/env bash
# Compila as telas para o PC e gera PNGs em docs/preview.
# Uso: tools/simulator/build_preview.sh [pasta_do_LovyanGFX]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
LGFX="${1:-$ROOT/.pio/libdeps/es3c28p/LovyanGFX}"
BUILD="$ROOT/.pio/simulator"
mkdir -p "$BUILD/obj" "$ROOT/docs/preview"

FLAGS=(-O1 -w -DLGFX_LINUX_FB -I"$LGFX/src" -I"$ROOT/src")
shopt -s nullglob
SOURCES=("$LGFX"/src/lgfx/v1/*.cpp "$LGFX"/src/lgfx/v1/misc/*.cpp "$LGFX"/src/lgfx/utility/*.c
         "$LGFX"/src/lgfx/Fonts/*/*.c "$ROOT"/src/ui/*.cpp "$ROOT"/src/ui/fonts/*.cpp
         "$ROOT"/tools/simulator/main.cpp)

OBJECTS=()
for source in "${SOURCES[@]}"; do
  object="$BUILD/obj/$(echo "$source" | md5sum | cut -c1-12).o"
  OBJECTS+=("$object")
  [ "$object" -nt "$source" ] && [[ "$source" == "$LGFX"* ]] && continue
  if [[ "$source" == *.c ]]; then gcc "${FLAGS[@]}" -c "$source" -o "$object"
  else g++ -std=c++17 "${FLAGS[@]}" -c "$source" -o "$object"; fi
done
g++ "${OBJECTS[@]}" -o "$BUILD/simulator"

python3 - "$BUILD" "$ROOT/docs/preview" <<'PY'
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw

build, target = Path(sys.argv[1]), Path(sys.argv[2])

# Miniatura sintética (320x180) para exercitar o decodificador JPEG
thumbnail = Image.new("RGB", (320, 180))
draw = ImageDraw.Draw(thumbnail)
for y in range(180):
    draw.line([(0, y), (320, y)], fill=(40 + y // 2, 60 + y // 3, 140 - y // 3))
draw.ellipse([200, 30, 290, 120], fill=(250, 200, 90))
thumbnail.save(build / "thumbnail.jpg", quality=80, progressive=False)

for old in build.glob("*.ppm"):
    old.unlink()
subprocess.run([build / "simulator", build, build / "thumbnail.jpg"], check=True)
for ppm in sorted(build.glob("*.ppm")):
    image = Image.open(ppm)
    image.resize((image.width * 2, image.height * 2), Image.NEAREST).save(target / f"{ppm.stem}.png")
print("telas geradas em", target)
PY
