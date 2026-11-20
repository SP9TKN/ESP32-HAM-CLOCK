// The AIRCRAFT RADAR and APRS MAP screens drawn by the real HAM CLOCK functions (parts_*.inc, cut from the .ino by
// extract_parts.py) on the TFT stand-in. Demo data: position = Blonia park in Krakow, fictional flights and N0CALL
// stations. Output: out/*.rgb565 (320x240).   Usage: screens.exe <SD card folder with tiles/>
#include "shim.h"

unsigned long g_millis = 300500;          // sweep line: fmod(300500/3000, 1) = 0.167 of a turn = 60 degrees
SDClass SD;
TFT_eSPI tft;

uint16_t menuThemeColor = 0xFD20;         // DEFAULT_MENU_THEME_COLOR
#define TFT_RADIO_ORANGE menuThemeColor
#define APRS_TILE_SIZE 256
#define MAX_APRS_STATIONS 30
#define MAX_AIRCRAFT 40
int aprsMapZoom = 13;
bool sdCardAvailable = true;
unsigned long aprsMapHintUntilMs = 0;
float mapCenterLat = 0, mapCenterLon = 0;
static uint16_t *mapScreenBuf = nullptr;
static bool mapBufValid = false;
static int mapBufZoom = -1;
static float mapBufCenterLat = 0.0f, mapBufCenterLon = 0.0f;
static void sdAcquireBus() {}
static void sdReleaseBus() {}

const double HOME_LAT = 50.060300, HOME_LON = 19.911600;   // Blonia park, Krakow (demo position)
bool getEffectiveAprsPosition(double &lat, double &lon) { lat = HOME_LAT; lon = HOME_LON; return true; }
bool getTimeWithTimezone(struct tm *t) { memset(t, 0, sizeof *t); t->tm_hour = 14; t->tm_min = 27; return true; }

struct APRSStation;
struct AircraftEntry;
float aircraftRadarRadiusKm = 100.0f;
bool aircraftShowScanline = true, aircraftShowInfoText = true, aircraftShowDirectionIcon = true;
String lastAircraftFetchStatus = "OK 7 arr7 r=100km";

#include "parts_structs.inc"
APRSStation aprsStations[MAX_APRS_STATIONS];
int aprsStationCount = 0;
AircraftEntry aircraftEntries[MAX_AIRCRAFT];
int aircraftCount = 0;
#include "parts_code.inc"

static void save(const char *nazwa) {
  char p[256];
  snprintf(p, sizeof p, "out/%s.rgb565", nazwa);
  FILE *f = fopen(p, "wb");
  fwrite(tft.fb, 1, sizeof tft.fb, f);
  fclose(f);
  printf("%s\n", p);
}

static void samolot(const char *call, const char *icao, double lat, double lon, float kurs, float kmh, float altM) {
  AircraftEntry &a = aircraftEntries[aircraftCount++];
  a = AircraftEntry();
  a.callsign = call; a.icao24 = icao; a.lat = (float)lat; a.lon = (float)lon;
  a.trueTrack = kurs; a.velocity = kmh / 3.6f; a.baroAltitude = altM; a.onGround = false;
  a.timePosition = a.lastContact = 0; a.lastSeenMs = g_millis; a.lastTickMs = 0; a.blendAlpha = 1.0f;
  a.active = true;
}

static void stacja(const char *call, const char *tab, const char *sym, double dLatKm, double dLonKm) {
  APRSStation &s = aprsStations[aprsStationCount++];
  s.callsign = call; s.symbolTable = tab; s.symbol = sym;
  s.lat = (float)(HOME_LAT + dLatKm / 111.32);
  s.lon = (float)(HOME_LON + dLonKm / (111.32 * cos(HOME_LAT * M_PI / 180.0)));
  s.hasLatLon = true; s.receivedAtMs = 0;
}

int main(int argc, char **argv) {
  SD.root = argc > 1 ? argv[1] : ".";

  // aircraft within 100 km (fictional flights), positions relative to the station
  samolot("LOT3907", "48ad01", HOME_LAT + 0.42, HOME_LON - 0.55, 300, 820, 10970);
  samolot("RYR4KB",  "4ca1f2", HOME_LAT - 0.35, HOME_LON + 0.60, 95,  790, 11280);
  samolot("WZZ6TX",  "471f0a", HOME_LAT + 0.13, HOME_LON - 0.34, 250, 410, 2130);
  samolot("DLH2KP",  "3c6d4e", HOME_LAT + 0.62, HOME_LON + 0.38, 190, 845, 11580);
  samolot("SPDEMO",  "48c2b9", HOME_LAT - 0.18, HOME_LON - 0.70, 20,  230, 1250);
  samolot("THY1LX",  "4bab31", HOME_LAT - 0.66, HOME_LON - 0.10, 160, 860, 10360);
  samolot("",        "48d7e3", HOME_LAT + 0.20, HOME_LON + 0.95, 275, 520, 5490);
  aircraftCount = 7;
  drawAircraftRadar();
  save("aircraft-radar");

  // APRS stations around (the placeholder callsign N0CALL)
  stacja("N0CALL-9",  "/", ">",  0.55,  1.05);   // car: red
  stacja("N0CALL-7",  "/", "[", -0.45, -0.80);   // walker: blue
  stacja("N0CALL",    "/", "-",  0.95, -1.25);   // house: yellow
  stacja("N0CALL-6",  "/", "_", -0.95,  0.70);   // WX: green
  stacja("N0CALL-3",  "/", "&",  0.20, -1.65);   // iGate: green
  stacja("N0CALL-1",  "/", "#", -0.70,  1.55);   // digi: green
  tft = TFT_eSPI();
  drawAprsMap();
  save("aprs-map");
  return 0;
}
