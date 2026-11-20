# Screen simulator

Draws the AIRCRAFT RADAR and APRS MAP screens on a PC with the firmware's own functions, cut out of
`src/ESP32-HAM-CLOCK.ino`, on a small stand-in for TFT_eSPI. The README pictures were made with it.

1. Copy `Fonts/glcdfont.c` from the TFT_eSPI library (e.g. `.pio/libdeps/esp32-2432s028/TFT_eSPI/`) into this folder.
2. `py extract_parts.py ../../src/ESP32-HAM-CLOCK.ino`
3. `py fetch_tiles.py ../.. sd` (four OpenStreetMap tiles; needs `requests` and Pillow)
4. `g++ -std=gnu++17 -O1 screens.cpp -o screens.exe`, `mkdir out`, `screens.exe sd`
5. `py render.py out`

The flights and stations are fictional (N0CALL is the placeholder callsign). Map data © OpenStreetMap contributors.
