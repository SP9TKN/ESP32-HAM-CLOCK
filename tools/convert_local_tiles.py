#!/usr/bin/env python3
"""
Konwertuje lokalne kafelki OSM (PNG) do formatu raw RGB565 dla ESP32-HAM-CLOCK.

Wejście:  E:\OSMTiles\{zoom:02d}\{x}\{y}.png
Wyjście:  tiles_sd\{zoom}\{x}\{y}.raw  (256*256*2 bajtów, little-endian RGB565)

Użycie:
  py tools\convert_local_tiles.py --src "E:\OSMTiles" --out tiles_sd
"""

import argparse
import os
import struct
from pathlib import Path
from PIL import Image, ImageEnhance

TILE_SIZE = 256


def png_to_raw565(img: Image.Image, contrast: float = 1.3) -> bytes:
    img = img.resize((TILE_SIZE, TILE_SIZE), Image.LANCZOS).convert("RGB")
    img = ImageEnhance.Contrast(img).enhance(contrast)
    img = ImageEnhance.Color(img).enhance(contrast)
    data = bytearray(TILE_SIZE * TILE_SIZE * 2)
    pixels = img.load()
    idx = 0
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            r, g, b = pixels[x, y]
            color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            struct.pack_into('<H', data, idx, color)
            idx += 2
    return bytes(data)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--src", default=r"E:\OSMTiles")
    parser.add_argument("--out", default="tiles_sd")
    parser.add_argument("--contrast", type=float, default=1.3)
    args = parser.parse_args()

    src = Path(args.src)
    out = Path(args.out)

    ok = err = skip = 0

    for zoom_dir in sorted(src.iterdir()):
        if not zoom_dir.is_dir():
            continue
        try:
            zoom = int(zoom_dir.name)  # "09" -> 9, usuwa padding
        except ValueError:
            continue

        for x_dir in sorted(zoom_dir.iterdir()):
            if not x_dir.is_dir():
                continue
            try:
                x = int(x_dir.name)
            except ValueError:
                continue

            for png_file in sorted(x_dir.glob("*.png")):
                try:
                    y = int(png_file.stem)
                except ValueError:
                    continue

                out_path = out / str(zoom) / str(x) / f"{y}.raw"
                if out_path.exists():
                    skip += 1
                    continue

                try:
                    img = Image.open(png_file)
                    raw = png_to_raw565(img, args.contrast)
                    out_path.parent.mkdir(parents=True, exist_ok=True)
                    out_path.write_bytes(raw)
                    ok += 1
                    if ok % 500 == 0:
                        print(f"  {ok} skonwertowanych...")
                except Exception as e:
                    print(f"  BŁĄD {png_file}: {e}")
                    err += 1

    print(f"\nGotowe: {ok} OK, {err} błędów, {skip} pominiętych (już istniały)")


if __name__ == "__main__":
    main()
