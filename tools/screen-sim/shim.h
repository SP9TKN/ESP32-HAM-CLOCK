// A minimal Arduino / TFT_eSPI / SD stand-in, so the HAM CLOCK drawing functions run on a PC.
// The primitives (drawCircle, fillCircle, drawLine, GLCD drawChar, write/print with wrapping) follow TFT_eSPI 2.5.
// glcdfont.c (the classic 5x7 font) comes from the TFT_eSPI library: copy TFT_eSPI/Fonts/glcdfont.c here.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <type_traits>

#define PROGMEM
#define pgm_read_byte(a) (*(const uint8_t *)(a))
#define pgm_read_word(a) (*(const uint16_t *)(a))
#define MALLOC_CAP_SPIRAM 0
static inline void *heap_caps_malloc(size_t, int) { return nullptr; }   // the CYD has no PSRAM
template <class A, class B> static inline typename std::common_type<A, B>::type min(A a, B b) { return a < b ? a : b; }
template <class A, class B> static inline typename std::common_type<A, B>::type max(A a, B b) { return a > b ? a : b; }
static inline float radians(float d) { return d * (float)M_PI / 180.0f; }
extern unsigned long g_millis;
static inline unsigned long millis() { return g_millis; }
static inline void yield() {}

class String {
public:
  std::string s;
  String() {}
  String(const char *c) : s(c ? c : "") {}
  String(const std::string &x) : s(x) {}
  String(char c) : s(1, c) {}
  String(int v) : s(std::to_string(v)) {}
  String(long v) : s(std::to_string(v)) {}
  String(unsigned int v) : s(std::to_string(v)) {}
  String(unsigned long v) : s(std::to_string(v)) {}
  String(float v, int dec = 2) { char b[48]; snprintf(b, sizeof b, "%.*f", dec, (double)v); s = b; }
  String(double v, int dec = 2) { char b[48]; snprintf(b, sizeof b, "%.*f", dec, v); s = b; }
  unsigned int length() const { return (unsigned)s.size(); }
  bool isEmpty() const { return s.empty(); }
  const char *c_str() const { return s.c_str(); }
  char charAt(unsigned i) const { return i < s.size() ? s[i] : 0; }
  String substring(unsigned a, unsigned b) const { if (a > s.size()) return String(); return String(s.substr(a, (b > s.size() ? s.size() : b) - a)); }
  String substring(unsigned a) const { return a > s.size() ? String() : String(s.substr(a)); }
  void trim() { size_t a = s.find_first_not_of(" \t\r\n"); size_t b = s.find_last_not_of(" \t\r\n"); s = a == std::string::npos ? "" : s.substr(a, b - a + 1); }
  bool startsWith(const char *p) const { return s.rfind(p, 0) == 0; }
  int indexOf(char c) const { size_t p = s.find(c); return p == std::string::npos ? -1 : (int)p; }
  int indexOf(const char *c) const { size_t p = s.find(c); return p == std::string::npos ? -1 : (int)p; }
  void toUpperCase() { for (auto &ch : s) ch = (char)toupper((unsigned char)ch); }
  String &operator+=(const String &o) { s += o.s; return *this; }
  String &operator+=(const char *o) { s += o; return *this; }
  String &operator+=(char o) { s += o; return *this; }
  bool operator==(const String &o) const { return s == o.s; }
  bool operator==(const char *o) const { return s == o; }
  bool operator!=(const char *o) const { return s != o; }
};
static inline String operator+(const String &a, const String &b) { return String(a.s + b.s); }
static inline String operator+(const String &a, const char *b) { return String(a.s + b); }
static inline String operator+(const char *a, const String &b) { return String(std::string(a) + b.s); }
static inline String operator+(const String &a, char b) { return String(a.s + b); }

#include "glcdfont.c"

#define TFT_BLACK     0x0000
#define TFT_NAVY      0x000F
#define TFT_LIGHTGREY 0xD69A
#define TFT_DARKGREY  0x7BEF
#define TFT_BLUE      0x001F
#define TFT_GREEN     0x07E0
#define TFT_CYAN      0x07FF
#define TFT_RED       0xF800
#define TFT_YELLOW    0xFFE0
#define TFT_WHITE     0xFFFF
#define TFT_ORANGE    0xFDA0

class TFT_eSPI {
public:
  static const int W = 320, H = 240;
  uint16_t fb[H][W];
  uint32_t textcolor = 0xFFFF, textbgcolor = 0xFFFF;
  uint8_t textsize = 1;
  int32_t cursor_x = 0, cursor_y = 0;
  bool textwrapX = true, textwrapY = false;
  int32_t winX = 0, winY = 0, winW = 0, winH = 0, winPos = 0;

  TFT_eSPI() { memset(fb, 0, sizeof fb); }
  int16_t width() const { return W; }
  int16_t height() const { return H; }
  void startWrite() {}
  void endWrite() {}
  void drawPixel(int32_t x, int32_t y, uint32_t c) { if (x >= 0 && y >= 0 && x < W && y < H) fb[y][x] = (uint16_t)c; }
  void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t c) {
    for (int32_t j = y; j < y + h; j++) for (int32_t i = x; i < x + w; i++) drawPixel(i, j, c);
  }
  void fillScreen(uint32_t c) { fillRect(0, 0, W, H, c); }
  void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t c) { fillRect(x, y, w, 1, c); }
  void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t c) { fillRect(x, y, 1, h, c); }
  void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }
  void setTextColor(uint16_t c, uint16_t b) { textcolor = c; textbgcolor = b; }
  void setTextSize(uint8_t s) { textsize = s ? s : 1; }
  void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
  void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h) { winX = x; winY = y; winW = w; winH = h; winPos = 0; }
  void pushColors(uint16_t *data, uint32_t len, bool swap = true) {
    (void)swap;   // on the ESP32 swap only reorders bytes for SPI; the colour value is the same
    for (uint32_t i = 0; i < len; i++, winPos++) drawPixel(winX + winPos % winW, winY + winPos / winW, data[i]);
  }

  // ---- as in TFT_eSPI.cpp ----
  void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
    if (r <= 0) return;
    int32_t f = 1 - r, ddF_y = -2 * r, ddF_x = 1, xs = -1, xe = 0, len = 0;
    bool first = true;
    do {
      while (f < 0) { ++xe; f += (ddF_x += 2); }
      f += (ddF_y += 2);
      if (xe - xs > 1) {
        if (first) {
          len = 2 * (xe - xs) - 1;
          drawFastHLine(x0 - xe, y0 + r, len, color); drawFastHLine(x0 - xe, y0 - r, len, color);
          drawFastVLine(x0 + r, y0 - xe, len, color); drawFastVLine(x0 - r, y0 - xe, len, color);
          first = false;
        } else {
          len = xe - xs++;
          drawFastHLine(x0 - xe, y0 + r, len, color); drawFastHLine(x0 - xe, y0 - r, len, color);
          drawFastHLine(x0 + xs, y0 - r, len, color); drawFastHLine(x0 + xs, y0 + r, len, color);
          drawFastVLine(x0 + r, y0 + xs, len, color); drawFastVLine(x0 + r, y0 - xe, len, color);
          drawFastVLine(x0 - r, y0 - xe, len, color); drawFastVLine(x0 - r, y0 + xs, len, color);
        }
      } else {
        ++xs;
        drawPixel(x0 - xe, y0 + r, color); drawPixel(x0 - xe, y0 - r, color);
        drawPixel(x0 + xs, y0 - r, color); drawPixel(x0 + xs, y0 + r, color);
        drawPixel(x0 + r, y0 + xs, color); drawPixel(x0 + r, y0 - xe, color);
        drawPixel(x0 - r, y0 - xe, color); drawPixel(x0 - r, y0 + xs, color);
      }
      xs = xe;
    } while (xe < --r);
  }
  void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
    int32_t x = 0, dx = 1, dy = r + r, p = -(r >> 1);
    drawFastHLine(x0 - r, y0, dy + 1, color);
    while (x < r) {
      if (p >= 0) {
        drawFastHLine(x0 - x, y0 + r, dx, color); drawFastHLine(x0 - x, y0 - r, dx, color);
        dy -= 2; p -= dy; r--;
      }
      dx += 2; p += dx; x++;
      drawFastHLine(x0 - r, y0 + x, dy + 1, color); drawFastHLine(x0 - r, y0 - x, dy + 1, color);
    }
  }
  void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
    int32_t dx = x1 - x0, dy = abs(y1 - y0);
    int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;
    if (y0 < y1) ystep = 1;
    if (steep) {
      for (; x0 <= x1; x0++) {
        dlen++; err -= dy;
        if (err < 0) {
          if (dlen == 1) drawPixel(y0, xs, color); else drawFastVLine(y0, xs, dlen, color);
          dlen = 0; y0 += ystep; xs = x0 + 1; err += dx;
        }
      }
      if (dlen) drawFastVLine(y0, xs, dlen, color);
    } else {
      for (; x0 <= x1; x0++) {
        dlen++; err -= dy;
        if (err < 0) {
          if (dlen == 1) drawPixel(xs, y0, color); else drawFastHLine(xs, y0, dlen, color);
          dlen = 0; y0 += ystep; xs = x0 + 1; err += dx;
        }
      }
      if (dlen) drawFastHLine(xs, y0, dlen, color);
    }
  }
  void drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size) {
    if (x >= W || y >= H || (x + 6 * size - 1) < 0 || (y + 8 * size - 1) < 0) return;
    if (c > 255) return;
    if (c > 175) c++;                       // _cp437 = false (TFT_eSPI default)
    bool fillbg = (bg != color);
    for (int8_t i = 0; i < 6; i++) {
      uint8_t line = (i == 5) ? 0 : pgm_read_byte(font + (c * 5) + i);
      if (size == 1 && !fillbg) {
        for (int8_t j = 0; j < 8; j++) { if (line & 1) drawPixel(x + i, y + j, color); line >>= 1; }
      } else {
        for (int8_t j = 0; j < 8; j++) {
          if (line & 1) fillRect(x + i * size, y + j * size, size, size, color);
          else if (fillbg) fillRect(x + i * size, y + j * size, size, size, bg);
          line >>= 1;
        }
      }
    }
  }
  size_t write(uint8_t ch) {
    if (ch == '\r') return 1;
    uint16_t uni = ch;
    if (uni == '\n') uni += 22;
    int32_t cwidth = 6, cheight = 8 * textsize;
    if (ch == '\n') { cursor_y += cheight; cursor_x = 0; }
    else {
      if (textwrapX && (cursor_x + cwidth * textsize > W)) { cursor_y += cheight; cursor_x = 0; }
      if (textwrapY && cursor_y >= H) cursor_y = 0;
      drawChar(cursor_x, cursor_y, uni, textcolor, textbgcolor, textsize);
      cursor_x += cwidth * textsize;
    }
    return 1;
  }
  void print(const char *s) { while (s && *s) write((uint8_t)*s++); }
  void print(const String &s) { print(s.c_str()); }
};

// ---- SD card = a local folder ----
#define FILE_READ "rb"
class File {
  FILE *f = nullptr;
public:
  File() {}
  explicit File(FILE *x) : f(x) {}
  explicit operator bool() const { return f != nullptr; }
  bool seek(uint32_t p) { return fseek(f, (long)p, SEEK_SET) == 0; }
  int read(uint8_t *b, size_t n) { return (int)fread(b, 1, n, f); }
  void close() { if (f) fclose(f); f = nullptr; }
};
struct SDClass {
  std::string root;
  File open(const char *p, const char *mode) { std::string full = root + p; return File(fopen(full.c_str(), mode)); }
};
extern SDClass SD;
