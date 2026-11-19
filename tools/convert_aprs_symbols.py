#!/usr/bin/env python3
"""
Pobiera sprite sheety symboli APRS z GitHub (hessu/aprs-symbols) i konwertuje
każdy symbol 24x24 → 16x16 raw RGB565 little-endian.

Wyjście:
  symbols/aprs_sym_0.raw  — primary table '/'  (96 symboli × 512 B = 49152 B)
  symbols/aprs_sym_1.raw  — alternate table '\' (96 symboli × 512 B = 49152 B)
  symbols/aprs_sym_2.raw  — overlays           (96 symboli × 512 B = 49152 B)

Format indeksowania: symbol_char → index = ord(symbol_char) - 33
('!' = 0, '"' = 1, ..., '~' = 93) — razem 94 symboli w 96-slotowym buforze
"""

import struct
import sys
import urllib.request
from io import BytesIO
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Wymagany Pillow: pip install Pillow")
    sys.exit(1)

COLS   = 16    # 16 kolumn w sprite sheecie
ROWS   = 6     # 6 rzędów (96 slotów)
SRC_SZ = 24    # każdy symbol 24x24 px w źródle
DST_SZ = 16    # docelowy rozmiar 16x16 px

BASE_URL = "https://raw.githubusercontent.com/hessu/aprs-symbols/master/png/"
SHEETS = [
    ("aprs-symbols-24-0.png", "aprs_sym_0.raw"),  # primary '/'
    ("aprs-symbols-24-1.png", "aprs_sym_1.raw"),  # alternate '\'
    ("aprs-symbols-24-2.png", "aprs_sym_2.raw"),  # overlays
]


def fetch_image(url: str) -> Image.Image:
    print(f"  Pobieram: {url}")
    with urllib.request.urlopen(url, timeout=15) as resp:
        data = resp.read()
    return Image.open(BytesIO(data)).convert("RGBA")


def symbol_to_raw565(img: Image.Image, col: int, row: int) -> bytes:
    """Wycina jeden symbol z sprite sheetu i konwertuje do RGB565 LE 16x16."""
    x0 = col * SRC_SZ
    y0 = row * SRC_SZ
    crop = img.crop((x0, y0, x0 + SRC_SZ, y0 + SRC_SZ))
    crop = crop.resize((DST_SZ, DST_SZ), Image.LANCZOS)
    # Przezroczyste tło → białe (zamiast czarnego, które zlewa się z TFT_BLACK)
    if crop.mode == 'RGBA':
        white = Image.new('RGB', crop.size, (255, 255, 255))
        white.paste(crop.convert('RGB'), mask=crop.split()[3])
        crop = white
    else:
        crop = crop.convert('RGB')
    pixels = crop.load()
    buf = bytearray(DST_SZ * DST_SZ * 2)
    idx = 0
    for y in range(DST_SZ):
        for x in range(DST_SZ):
            r, g, b = pixels[x, y]
            color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            struct.pack_into('<H', buf, idx, color)
            idx += 2
    return bytes(buf)


def convert_sheet(url: str, out_path: Path):
    img = fetch_image(url)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    result = bytearray(COLS * ROWS * DST_SZ * DST_SZ * 2)  # 96 slotów
    for row in range(ROWS):
        for col in range(COLS):
            idx = row * COLS + col
            raw = symbol_to_raw565(img, col, row)
            offset = idx * DST_SZ * DST_SZ * 2
            result[offset:offset + len(raw)] = raw

    out_path.write_bytes(result)
    print(f"  Zapisano: {out_path}  ({len(result)} B, {COLS * ROWS} symboli)")


def main():
    out_dir = Path(__file__).parent.parent / "symbols"
    print(f"Katalog wyjściowy: {out_dir}")

    for src_name, dst_name in SHEETS:
        url = BASE_URL + src_name
        out_path = out_dir / dst_name
        print(f"\n[{src_name}]")
        try:
            convert_sheet(url, out_path)
        except Exception as e:
            print(f"  BŁĄD: {e}")

    print("\nGotowe.")


if __name__ == "__main__":
    main()
