#!/usr/bin/env python3
"""
Pobieranie i konwersja kafelków OSM do formatu raw RGB565 dla ESP32-HAM-CLOCK.

Format pliku wyjściowego: /tiles/{zoom}/{x}/{y}.raw
  - 256 x 256 pikseli
  - Każdy piksel: uint16_t little-endian (RGB565)
  - Rozmiar pliku: 256 * 256 * 2 = 131072 bajtów

Użycie:
  python convert_tiles.py --lat 50.0 --lon 22.0 --zoom 12 --radius 2 --out tiles/

Wymagania:
  pip install requests Pillow
"""

import argparse
import math
import os
import struct
import time
import requests
from PIL import Image
from io import BytesIO

TILE_SIZE = 256
HEADERS = {"User-Agent": "ESP32-HAM-CLOCK/1.0 tile-downloader (sp9tkn@gmail.com)"}

def lat_lon_to_tile(lat, lon, zoom):
    n = 2 ** zoom
    x = int((lon + 180.0) / 360.0 * n)
    lat_r = math.radians(lat)
    y = int((1.0 - math.log(math.tan(lat_r) + 1.0 / math.cos(lat_r)) / math.pi) / 2.0 * n)
    return x, y

def download_tile(x, y, zoom, tile_server="https://tile.openstreetmap.org/{z}/{x}/{y}.png"):
    url = tile_server.replace("{z}", str(zoom)).replace("{x}", str(x)).replace("{y}", str(y))
    for attempt in range(3):
        try:
            r = requests.get(url, headers=HEADERS, timeout=10)
            if r.status_code == 200:
                return Image.open(BytesIO(r.content)).convert("RGB")
        except Exception as e:
            print(f"  Błąd pobierania {url}: {e}")
        time.sleep(1)
    return None

def png_to_raw565(img: Image.Image, contrast: float = 1.3) -> bytes:
    img = img.resize((TILE_SIZE, TILE_SIZE), Image.LANCZOS)
    from PIL import ImageEnhance
    img = ImageEnhance.Contrast(img).enhance(contrast)
    img = ImageEnhance.Color(img).enhance(contrast)
    data = bytearray()
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            r, g, b = img.getpixel((x, y))
            color = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            data += struct.pack('<H', color)  # little-endian
    return bytes(data)

def main():
    parser = argparse.ArgumentParser(description="Pobierz i konwertuj kafelki OSM do raw RGB565")
    parser.add_argument("--lat", type=float, required=True, help="Szerokość geograficzna centrum")
    parser.add_argument("--lon", type=float, required=True, help="Długość geograficzna centrum")
    parser.add_argument("--zoom", type=int, nargs="+", default=[12], help="Poziom(y) zoomu (np. 10 12 14)")
    parser.add_argument("--radius", type=int, default=2, help="Promień kafelków wokół centrum (default: 2)")
    parser.add_argument("--out", type=str, default="tiles", help="Katalog wyjściowy")
    parser.add_argument("--server", type=str,
                        default="https://tile.openstreetmap.org/{z}/{x}/{y}.png",
                        help="URL serwera kafelków")
    args = parser.parse_args()

    for zoom in args.zoom:
        cx, cy = lat_lon_to_tile(args.lat, args.lon, zoom)
        print(f"\nZoom {zoom}: centrum kafelek ({cx}, {cy})")
        for ty in range(cy - args.radius, cy + args.radius + 1):
            for tx in range(cx - args.radius, cx + args.radius + 1):
                out_path = os.path.join(args.out, str(zoom), str(tx), f"{ty}.raw")
                if os.path.exists(out_path):
                    print(f"  [{zoom}/{tx}/{ty}] już istnieje, pomijam")
                    continue
                print(f"  [{zoom}/{tx}/{ty}] pobieranie...")
                img = download_tile(tx, ty, zoom, args.server)
                if img is None:
                    print(f"  [{zoom}/{tx}/{ty}] BŁĄD pobierania!")
                    continue
                os.makedirs(os.path.dirname(out_path), exist_ok=True)
                with open(out_path, "wb") as f:
                    f.write(png_to_raw565(img))
                print(f"  [{zoom}/{tx}/{ty}] OK -> {out_path}")
                time.sleep(0.3)  # grzeczność wobec serwera OSM

    print("\nGotowe! Skopiuj katalog 'tiles/' na kartę SD.")
    print("Struktura: /tiles/{zoom}/{x}/{y}.raw")

if __name__ == "__main__":
    main()
