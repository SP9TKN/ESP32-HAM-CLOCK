# Cuts the functions and structures needed for the aircraft radar and the APRS map out of ESP32-HAM-CLOCK.ino.
# Usage: py extract_parts.py ../../src/ESP32-HAM-CLOCK.ino   (writes parts_structs.inc and parts_code.inc)
import re
import sys

src = open(sys.argv[1], encoding="utf-8", errors="replace").read()
lines = src.split("\n")


def block_from(start_idx):
    """From line start_idx to the matching closing brace (skips strings, chars and comments)."""
    depth, seen = 0, False
    in_block_comment = False
    for i in range(start_idx, len(lines)):
        s = lines[i]
        j = 0
        while j < len(s):
            c = s[j]
            if in_block_comment:
                if s.startswith("*/", j):
                    in_block_comment = False
                    j += 2
                    continue
                j += 1
                continue
            if s.startswith("//", j):
                break
            if s.startswith("/*", j):
                in_block_comment = True
                j += 2
                continue
            if c in "\"'":
                q = c
                j += 1
                while j < len(s) and s[j] != q:
                    j += 2 if s[j] == "\\" else 1
                j += 1
                continue
            if c == "{":
                depth += 1
                seen = True
            elif c == "}":
                depth -= 1
                if seen and depth == 0:
                    return "\n".join(lines[start_idx:i + 1])
            j += 1
    raise RuntimeError("no closing brace from line %d" % (start_idx + 1))


def find(pattern):
    rx = re.compile(pattern)
    for i, l in enumerate(lines):
        if rx.match(l):
            return i
    raise RuntimeError("not found: " + pattern)


parts = []
for pat in [r"^struct APRSStation \{", r"^struct AircraftEntry \{"]:
    parts.append(block_from(find(pat)) + "\n")
for pat in [r"^uint16_t rgb565\(", r"^String getAPRSSymbolRaw\(", r"^String getAPRSSymbolShort\(",
            r"^float calculateDistance\(", r"^float calculateBearing\(",
            r"^static void aircraftPredictPosition\(", r"^static void aircraftTick\(",
            r"^static void aircraftGetDisplayPosition\(", r"^static uint16_t getAprsRadarColorForStation\("]:
    parts.append(block_from(find(pat)))
# plane icons: from the constants to the end of planeLeft
i0 = find(r"^constexpr int PLANE_ICON_SIZE")
parts.append("\n".join(lines[i0:i0 + 2]))
for name in ("planeUp", "planeRight", "planeDown", "planeLeft"):
    parts.append(block_from(find(r"^const uint16_t %s\[\] PROGMEM" % name)))
g0 = find(r"^const uint16_t AIRCRAFT_RADAR_GREEN_BRIGHT")
parts.append("\n".join(lines[g0:g0 + 3]))
for pat in [r"^static void computeAircraftRadarGeometry\(", r"^static void drawAircraftScanTrail\(",
            r"^static void paintAircraftRadarBackground\(", r"^static void paintAircraftRadarAircraft\(",
            r"^void drawAircraftRadar\(", r"^static void latLonToAbsPixel\(", r"^static void absPixelToLatLon\(",
            r"^static bool drawTileRaw565\(", r"^static bool writeTileToBuffer\(", r"^void drawAprsMap\("]:
    parts.append(block_from(find(pat)))

text = "\n\n".join(parts) + "\n"
cut = text.index("uint16_t rgb565(")
open("parts_structs.inc", "w", encoding="utf-8").write(text[:cut])
open("parts_code.inc", "w", encoding="utf-8").write(text[cut:])
print("parts:", len(parts))
