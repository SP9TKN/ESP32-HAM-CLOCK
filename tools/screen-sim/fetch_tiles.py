# Downloads only the OSM tiles needed for the 320x240 APRS map window (zoom 13 around the demo position) and converts
# them with png_to_raw565 from tools/convert_tiles.py, exactly as for the SD card.
# Usage: py fetch_tiles.py <repository folder> <SD folder>
import math, os, sys, time
from io import BytesIO
import requests
from PIL import Image

REPO, SD = sys.argv[1], sys.argv[2]
sys.path.insert(0, os.path.join(REPO, "tools"))
import convert_tiles as ct   # noqa: E402

LAT, LON, Z = 50.060300, 19.911600, 13
UA = {"User-Agent": "ESP32-HAM-CLOCK-readme-demo/1.0 (+https://github.com/SP9TKN/ESP32-HAM-CLOCK)"}
n = (1 << Z) * 256
px = int((LON + 180.0) / 360.0 * n)
lr = math.radians(LAT)
py = int((1.0 - math.log(math.tan(lr) + 1.0 / math.cos(lr)) / math.pi) / 2.0 * n)
tx0, tx1 = (px - 160) // 256, (px - 160 + 319) // 256
ty0, ty1 = (py - 120) // 256, (py - 120 + 239) // 256
for tx in range(tx0, tx1 + 1):
    for ty in range(ty0, ty1 + 1):
        out = os.path.join(SD, "tiles", str(Z), str(tx), f"{ty}.raw")
        if os.path.exists(out):
            continue
        r = requests.get(f"https://tile.openstreetmap.org/{Z}/{tx}/{ty}.png", headers=UA, timeout=15)
        r.raise_for_status()
        img = Image.open(BytesIO(r.content)).convert("RGB")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        open(out, "wb").write(ct.png_to_raw565(img))
        print("tile", Z, tx, ty)
        time.sleep(1)
