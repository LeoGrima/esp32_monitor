"""Gera as fontes VLW (anti-aliased) embutidas no firmware a partir da Inter (licença OFL).

Uso: python tools/generate_fonts.py [pasta_das_fontes_inter]
"""
import struct
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

FONT_DIR = Path(sys.argv[1] if len(sys.argv) > 1 else "/usr/share/fonts/opentype/inter")
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "src" / "ui" / "fonts"

CHARSET_TEXT = (
    "".join(chr(code) for code in range(0x21, 0x7F))
    + "°·ºª“”‘’–—…ÁÀÂÃÉÊÍÓÔÕÚÜÇáàâãéêíóôõúüç"
)
CHARSET_NUMERIC = "0123456789,.+-:%°"

# nome, arquivo, tamanho em px, conjunto de caracteres
FONTS = [
    ("font_huge", "InterDisplay-Light.otf", 84, CHARSET_NUMERIC),
    ("font_big", "InterDisplay-Light.otf", 58, CHARSET_NUMERIC),
    ("font_large", "InterDisplay-Regular.otf", 30, CHARSET_TEXT),
    ("font_title", "InterDisplay-SemiBold.otf", 25, CHARSET_TEXT),
    ("font_medium", "Inter-Medium.otf", 17, CHARSET_TEXT),
    ("font_small", "Inter-SemiBold.otf", 12, CHARSET_TEXT),
]


def render_glyph(font, char):
    left, top, right, bottom = font.getbbox(char, anchor="ls")
    width, height = right - left, bottom - top
    image = Image.new("L", (max(width, 1), max(height, 1)), 0)
    ImageDraw.Draw(image).text((-left, -top), char, font=font, fill=255, anchor="ls")
    return {
        "code": ord(char),
        "width": width,
        "height": height,
        "advance": round(font.getlength(char)),
        "top": -top,
        "left": left,
        "bitmap": image.tobytes() if width and height else b"",
    }


def build_vlw(font_file, size, charset):
    font = ImageFont.truetype(str(FONT_DIR / font_file), size)
    glyphs = [render_glyph(font, char) for char in sorted(set(charset))]
    space = {"code": 0x20, "width": 0, "height": 0, "advance": round(font.getlength(" ")),
             "top": 0, "left": 0, "bitmap": b""}
    glyphs = [space] + glyphs

    ascent = -font.getbbox("d", anchor="ls")[1]
    descent = font.getbbox("p", anchor="ls")[3]
    data = struct.pack(">6i", len(glyphs), 11, size, 0, ascent, descent)
    for glyph in glyphs:
        data += struct.pack(">7i", glyph["code"], glyph["height"], glyph["width"],
                            glyph["advance"], glyph["top"], glyph["left"], 0)
    return data + b"".join(glyph["bitmap"] for glyph in glyphs)


def write_source(name, data):
    rows = [", ".join(f"0x{byte:02X}" for byte in data[i:i + 16]) for i in range(0, len(data), 16)]
    body = ",\n  ".join(rows)
    (OUTPUT_DIR / f"{name}.cpp").write_text(
        "// Arquivo gerado por tools/generate_fonts.py - não editar.\n"
        '#include "ui_fonts.h"\n\n'
        f"extern const uint8_t {name}_data[] = {{\n  {body}\n}};\n",
        encoding="utf-8",
    )


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    for name, font_file, size, charset in FONTS:
        data = build_vlw(font_file, size, charset)
        write_source(name, data)
        print(f"{name}: {font_file} {size}px, {len(data)} bytes")


if __name__ == "__main__":
    main()
