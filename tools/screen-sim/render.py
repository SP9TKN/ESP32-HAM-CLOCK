# out/*.rgb565 (320x240, uint16 little-endian) -> PNG x2 in a frame that looks like the CYD (yellow board, black bezel).
# Usage: py render.py out
import glob, os, struct, sys
from PIL import Image, ImageDraw

OUT = sys.argv[1]
for p in glob.glob(os.path.join(OUT, "*.rgb565")):
    raw = open(p, "rb").read()
    px = struct.unpack("<%dH" % (320 * 240), raw)
    img = Image.new("RGB", (320, 240))
    img.putdata([(((c >> 11) & 31) * 255 // 31, ((c >> 5) & 63) * 255 // 63, (c & 31) * 255 // 31) for c in px])
    img = img.resize((640, 480), Image.NEAREST)
    b, m = 20, 10
    ramka = Image.new("RGBA", (640 + 2 * (b + m), 480 + 2 * (b + m)), (0, 0, 0, 0))
    d = ImageDraw.Draw(ramka)
    d.rounded_rectangle((0, 0, ramka.width - 1, ramka.height - 1), radius=24, fill=(214, 168, 46, 255))   # board
    d.rounded_rectangle((m, m, ramka.width - 1 - m, ramka.height - 1 - m), radius=14, fill=(18, 18, 20, 255))
    ramka.paste(img, (b + m, b + m))
    ramka.save(p[:-7] + ".png", optimize=True)
    print(p[:-7] + ".png", ramka.size)
