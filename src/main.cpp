// ============================================================================
//  Digital Mood Hair Clip ♡  —  LilyGO T-Display (ESP32 + ST7789V 135x240)
//
//  Secuencia 100% automática y en bucle infinito:
//    1) Splash "frandorturtles.exe" (exactamente SPLASH_MS = 2 s, al encender)
//    2) Presentación: "Hi! I'm Francesca ♡" -> "Welcome to my little world!"
//    3) "My moods": frases de personalidad, con una imagen de la galería
//       intercalada cada IMAGE_EVERY mensajes
//    4) Vuelve al paso 2 (sin repetir el splash) y así para siempre.
//
//  Sin botones, sin Wi-Fi, sin Bluetooth, sin Internet.
//  Todo lo editable (mensajes, tiempos, imágenes) está en include/config.h.
//
//  Técnica: se dibuja cada fotograma en un "sprite" (buffer de 240x135 en RAM)
//  y se envía de una vez a la pantalla -> animación sin parpadeo.
//  Ahorro de batería: CPU a 80 MHz, 20 fps, brillo moderado por PWM.
//
//  NOTA: este código se escribió y revisó sin la placa a mano. Compila con
//  PlatformIO, pero hay que probarlo en el hardware real (ver README).
// ============================================================================
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "config.h"

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// ----------------------------------------------------------------------------
//  Colores y utilidades
// ----------------------------------------------------------------------------
struct Rgb8 { uint8_t r, g, b; };

constexpr uint16_t c565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
static inline uint16_t c565(const Rgb8& c) { return c565(c.r, c.g, c.b); }

static inline int iclamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

static Rgb8 mix(const Rgb8& a, const Rgb8& b, int t255) {
  Rgb8 o;
  o.r = a.r + ((int)(b.r - a.r) * t255) / 255;
  o.g = a.g + ((int)(b.g - a.g) * t255) / 255;
  o.b = a.b + ((int)(b.b - a.b) * t255) / 255;
  return o;
}

// Paleta coquette / Y2K
static const uint16_t COL_INK    = c565(74, 42, 106);     // morado oscuro (texto y contornos)
static const uint16_t COL_CREAM  = c565(255, 247, 252);   // cuerpo de las ventanitas
static const uint16_t COL_WHITE  = c565(255, 255, 255);
static const uint16_t COL_PINK   = c565(255, 105, 180);
static const uint16_t COL_PINK_D = c565(219, 60, 140);
static const uint16_t COL_PINK_P = c565(255, 182, 217);
static const uint16_t COL_LAV    = c565(200, 170, 255);
static const uint16_t COL_SHADOW = c565(150, 120, 205);

// ----------------------------------------------------------------------------
//  Pixel art en código: cada letra es un color, '.' = transparente
// ----------------------------------------------------------------------------
struct Bitmap { const char* const* rows; uint8_t w; uint8_t h; };

static uint16_t palette(char c) {
  switch (c) {
    case 'K': return COL_INK;
    case 'P': return COL_PINK;
    case 'p': return COL_PINK_P;
    case 'O': return COL_PINK_D;
    case 'W': return COL_WHITE;
    case 'L': return COL_LAV;
    case 'l': return c565(232, 218, 255);
    case 'B': return c565(160, 205, 255);
    case 'Y': return c565(255, 240, 150);
    case 'G': return c565(150, 220, 160);
    case 'R': return c565(255, 140, 165);
    default:  return 0;
  }
}

static const char* const HEART_R[]   = {".KKK.KKK.", "KPPPKPPPK", "KPWPPPPPK", "KPPPPPPPK",
                                        ".KPPPPPK.", "..KPPPK..", "...KPK...", "....K...."};
static const char* const HEARTS_R[]  = {".P.P.", "PPPPP", "PPPPP", ".PPP.", "..P.."};
static const char* const STAR_R[]    = {"...Y...", "...Y...", "..YWY..", "YYWWWYY", "..YWY..", "...Y...", "...Y..."};
static const char* const STARS_R[]   = {"..Y..", ".YWY.", "YWWWY", ".YWY.", "..Y.."};
static const char* const FLOWER_R[]  = {".pp.pp.", "ppppppp", "pppYppp", ".ppppp.", "..pGp..", "...G...", "..GG..."};
static const char* const BOW_R[]     = {".PPP...PPP.", "PPPPPOPPPPP", "PWPPPOPPPWP", "PPPPPOPPPPP", ".PPP...PPP."};
static const char* const FACE_R[]    = {"..KKKKKKK..", ".KWWWWWWWK.", "KWWWWWWWWWK", "KWWKWWWKWWK",
                                        "KWRWWWWWRWK", "KWWWWKWWWWK", ".KWWWWWWWK.", "..KKKKKKK.."};
static const char* const TURTLE_R[]  = {"....KKKKK....", "...KGGPGGK.KK", "..KGPGGGPGKGK", "..KGGGPGGGKGK",
                                        "...KKKKKKKKK.", "...KK.....KK."};
static const char* const ORN_R[]     = {"..PP...", ".P..P..", "....P..", "...P...", "..P....", ".P.....", "PPPPP.."};

static const Bitmap HEART  = {HEART_R, 9, 8};
static const Bitmap HEART_S = {HEARTS_R, 5, 5};
static const Bitmap STAR   = {STAR_R, 7, 7};
static const Bitmap STAR_S = {STARS_R, 5, 5};
static const Bitmap FLOWER = {FLOWER_R, 7, 7};
static const Bitmap BOW    = {BOW_R, 11, 5};
static const Bitmap FACE   = {FACE_R, 11, 8};
static const Bitmap TURTLE = {TURTLE_R, 13, 6};
static const Bitmap ORN    = {ORN_R, 7, 7};

static void drawBitmap(const Bitmap& b, int x, int y, int s = 1, bool flip = false) {
  for (int j = 0; j < b.h; j++) {
    const char* row = b.rows[j];
    for (int i = 0; i < b.w; i++) {
      char ch = row[flip ? (b.w - 1 - i) : i];
      if (ch == '.') continue;
      uint16_t col = palette(ch);
      if (s == 1) spr.drawPixel(x + i, y + j, col);
      else        spr.fillRect(x + i * s, y + j * s, s, s, col);
    }
  }
}

// Estrellita que parpadea: punto -> cruz -> estrella chica -> estrella grande...
static void sparkle(int x, int y, uint32_t t, int phase) {
  int s = (int)((t / 170 + phase) % 6);
  switch (s) {
    case 0: case 5:
      spr.drawPixel(x + 3, y + 3, COL_WHITE);
      break;
    case 1:
      spr.drawPixel(x + 3, y + 3, COL_WHITE);
      spr.drawPixel(x + 2, y + 3, COL_WHITE); spr.drawPixel(x + 4, y + 3, COL_WHITE);
      spr.drawPixel(x + 3, y + 2, COL_WHITE); spr.drawPixel(x + 3, y + 4, COL_WHITE);
      break;
    case 2: case 4: drawBitmap(STAR_S, x + 1, y + 1); break;
    default:        drawBitmap(STAR, x, y); break;
  }
}

static void floatingHearts(uint32_t t, int n) {
  for (int i = 0; i < n; i++) {
    int x = (i * 47 + 20) % (SCREEN_W - 10);
    int y = SCREEN_H - (int)((t / 35 + i * 29) % (SCREEN_H + 16));
    x += (int)(4.0f * sinf(t / 300.0f + i));
    drawBitmap(HEART_S, x, y);
  }
}

// ----------------------------------------------------------------------------
//  Fondos animados (cada pantalla de texto usa uno distinto)
// ----------------------------------------------------------------------------
static void gradientBg(const Rgb8& a, const Rgb8& b) {
  for (int y = 0; y < SCREEN_H; y++)
    spr.drawFastHLine(0, y, SCREEN_W, c565(mix(a, b, (y * 255) / (SCREEN_H - 1))));
}

static void bgCandy(uint32_t t) {          // rosa -> lavanda con lunares que se deslizan
  gradientBg({255, 214, 236}, {214, 190, 255});
  int off = (int)((t / 70) % 24);
  for (int gy = 0; gy < SCREEN_H + 24; gy += 24)
    for (int gx = 0; gx < SCREEN_W + 24; gx += 24) {
      int x = gx + (((gy / 24) & 1) ? 12 : 0) - off;
      int y = gy - off;
      if (x > -3 && x < SCREEN_W + 3 && y > -3 && y < SCREEN_H + 3)
        spr.fillCircle(x, y, 2, c565(255, 238, 247));
    }
}

static void bgNight(uint32_t t) {          // cielo nocturno con estrellitas
  gradientBg({28, 22, 78}, {96, 64, 160});
  for (int i = 0; i < 26; i++) {
    int x = (i * 53 + 17) % SCREEN_W, y = (i * 37 + 5) % 105;
    int tw = (int)((t / 220 + i) % 5);
    if (i % 6 == 0) { if (tw < 3) drawBitmap(STAR_S, x, y); }
    else spr.drawPixel(x, y, tw == 0 ? COL_LAV : COL_WHITE);
  }
}

static void bgGrid(uint32_t t) {           // rejilla Y2K que se desplaza en diagonal
  spr.fillRect(0, 0, SCREEN_W, SCREEN_H, c565(238, 226, 255));
  int off = (int)((t / 60) % 16);
  for (int x = -off; x < SCREEN_W; x += 16) spr.drawFastVLine(x, 0, SCREEN_H, c565(255, 196, 228));
  for (int y = -off; y < SCREEN_H; y += 16) spr.drawFastHLine(0, y, SCREEN_W, c565(255, 196, 228));
}

static void bgHearts(uint32_t t) {         // cielo pastel con corazones que suben
  gradientBg({170, 215, 255}, {255, 214, 236});
  floatingHearts(t, 7);
}

// ----------------------------------------------------------------------------
//  Ventanita estilo Y2K
// ----------------------------------------------------------------------------
static const int WIN_X = 10, WIN_Y = 12, WIN_W = 220, WIN_H = 111, BAR_H = 14, PAD = 8;

static void drawWindow(int x, int y, int w, int h, const char* title, const Rgb8& bar) {
  spr.fillRect(x + 3, y + 3, w, h, COL_SHADOW);            // sombra
  spr.fillRect(x, y, w, h, COL_INK);                       // contorno
  spr.fillRect(x + 2, y + 2, w - 4, h - 4, COL_CREAM);     // cuerpo
  spr.fillRect(x + 2, y + 2, w - 4, BAR_H, c565(bar));     // barra de título
  spr.drawFastHLine(x + 2, y + 2, w - 4, c565(mix(bar, {255, 255, 255}, 90)));
  spr.setTextFont(1); spr.setTextSize(1); spr.setTextColor(COL_WHITE);
  if (title && title[0]) spr.drawString(title, x + 7, y + 6);
  for (int i = 0; i < 3; i++) {                            // botoncitos _ [] x
    int bx = x + w - 4 - (3 - i) * 11, by = y + 4;
    spr.fillRect(bx, by, 9, 9, i == 2 ? COL_PINK : COL_LAV);
    spr.drawRect(bx, by, 9, 9, COL_INK);
    if (i == 0) spr.drawFastHLine(bx + 2, by + 6, 5, COL_INK);
    if (i == 1) spr.drawRect(bx + 2, by + 2, 5, 5, COL_INK);
    if (i == 2) for (int k = 0; k < 5; k++) { spr.drawPixel(bx + 2 + k, by + 2 + k, COL_WHITE); spr.drawPixel(bx + 6 - k, by + 2 + k, COL_WHITE); }
  }
}

// ----------------------------------------------------------------------------
//  Texto: ajuste automático de tamaño, saltos de línea, "máquina de escribir"
//  Fuente: la retro 6x8 integrada (GLCD), ampliada x1, x2 o x3 = pixel art real.
//  El símbolo ^ se dibuja como un corazón pixel art.
// ----------------------------------------------------------------------------
#define MAX_LINES 8
static char lines[MAX_LINES][40];

static inline int glyphAdvance(char c, int s) { return c == '^' ? 11 * s : 6 * s; }

static int lineWidth(const char* l, int s) {
  int w = 0;
  for (; *l; ++l) w += glyphAdvance(*l, s);
  return w;
}

// Reparte el texto en líneas que quepan en maxW. Devuelve el nº de líneas.
static int layout(const char* text, int s, int maxW) {
  int n = 0, len = 0, w = 0;
  lines[0][0] = 0;
  const char* p = text;
  while (*p) {
    const char* q = p;
    int ww = 0;
    while (*q && *q != ' ') { ww += glyphAdvance(*q, s); q++; }
    int wl = (int)(q - p);
    if (len > 0 && w + 6 * s + ww > maxW) {          // no cabe: nueva línea
      lines[n][len] = 0;
      if (++n >= MAX_LINES) return MAX_LINES;
      len = 0; w = 0;
    }
    if (len > 0) { lines[n][len++] = ' '; w += 6 * s; }
    if (len + wl > 38) wl = 38 - len;                // protección de buffer
    memcpy(&lines[n][len], p, wl);
    len += wl; w += ww;
    p = q;
    while (*p == ' ') p++;
  }
  lines[n][len] = 0;
  return n + 1;
}

// Prueba tamaños 3, 2, 1 y se queda con el más grande que quepa.
static int fitText(const char* text, int maxW, int maxH, int& size) {
  for (int s = 3; s >= 1; s--) {
    int n = layout(text, s, maxW);
    if (s == 1 || n * 10 * s - 2 * s <= maxH) { size = s; return n; }
  }
  size = 1;
  return layout(text, 1, maxW);
}

// Dibuja una línea; 'remaining' = cuántos caracteres faltan por mostrar. Devuelve x final.
static int drawLine(const char* l, int x, int y, int s, uint16_t col, int remaining, bool hearts) {
  spr.setTextFont(1); spr.setTextSize(s); spr.setTextColor(col);
  char run[40];
  int rl = 0;
  for (const char* p = l; *p && remaining > 0; ++p, --remaining) {
    if (*p == '^') {
      if (rl) { run[rl] = 0; spr.drawString(run, x, y); x += rl * 6 * s; rl = 0; }
      if (hearts) drawBitmap(HEART, x + 2 * s, y, s);
      x += 11 * s;
    } else {
      run[rl++] = *p;
    }
  }
  if (rl) { run[rl] = 0; spr.drawString(run, x, y); x += rl * 6 * s; }
  return x;
}

static void drawGauge(int pct, uint32_t t, int gx, int gy, int gw, int gh) {
  int shown = (t > 1400) ? pct : (int)(pct * t / 1400);
  spr.fillRect(gx, gy, gw, gh, COL_INK);
  spr.fillRect(gx + 2, gy + 2, gw - 4, gh - 4, COL_WHITE);
  bool low = pct <= 20 && ((t / 500) & 1);
  spr.fillRect(gx + 2, gy + 2, (gw - 4) * shown / 100, gh - 4, low ? COL_PINK_D : COL_PINK);
  spr.fillRect(gx + gw, gy + 3, 3, gh - 6, COL_INK);   // "polo" de la pila
}

static void drawTextBlock(const char* text, int gauge, uint32_t t) {
  const int bodyX = WIN_X + PAD, bodyY = WIN_Y + 2 + BAR_H + 4;
  const int bodyW = WIN_W - 2 * PAD, bodyH = WIN_H - 2 - BAR_H - 4 - 6;
  int availH = bodyH - (gauge >= 0 ? 20 : 0);
  int size;
  int n = fitText(text, bodyW, availH, size);
  int lineH = 10 * size, blockH = n * lineH - 2 * size;
  int y0 = bodyY + (availH - blockH) / 2;
  int sh = size > 1 ? 2 : 1;

  int remaining = (int)(t / TYPE_MS_PER_CHAR);
  int cursorX = -1, cursorY = 0;
  for (int i = 0; i < n; i++) {
    int len = (int)strlen(lines[i]);
    int x = bodyX + (bodyW - lineWidth(lines[i], size)) / 2;
    int y = y0 + i * lineH;
    if (remaining > 0) {
      drawLine(lines[i], x + sh, y + sh, size, COL_PINK_P, remaining, false);   // sombrita
      cursorX = drawLine(lines[i], x, y, size, COL_INK, remaining, true);
      cursorY = y;
    }
    remaining -= len;
  }
  if (cursorX >= 0 && ((t / 400) & 1) == 0)                                      // cursor parpadeante
    spr.fillRect(cursorX + 2, cursorY + 8 * size - size, 4 * size, size, COL_PINK);

  if (gauge >= 0) drawGauge(gauge, t, bodyX + (bodyW - 130) / 2, bodyY + bodyH - 14, 130, 12);
}

// "Pegatina" que rebota suavemente en una esquina de la ventana
static void sticker(int kind, uint32_t t) {
  int bob = (int)(2.0f * sinf(t / 250.0f));
  switch (kind % 5) {
    case 0: drawBitmap(BOW,    WIN_X - 8,             WIN_Y - 6 + bob,             2); break;
    case 1: drawBitmap(FLOWER, WIN_X + WIN_W - 12,    WIN_Y - 9 + bob,             2); break;
    case 2: drawBitmap(FACE,   WIN_X - 8,             WIN_Y + WIN_H - 12 + bob,    2); break;
    case 3: drawBitmap(TURTLE, WIN_X + WIN_W - 24,    WIN_Y + WIN_H - 7 + bob,     2); break;
    default: drawBitmap(HEART, WIN_X + WIN_W - 10,    WIN_Y - 8 + bob,             2); break;
  }
}

static void renderTextScene(const char* title, const char* text, int gauge, int variant, uint32_t t) {
  static const Rgb8 BAR[4] = {{150, 120, 235}, {110, 90, 200}, {255, 120, 185}, {120, 150, 235}};
  switch (variant & 3) {
    case 0: bgCandy(t);  break;
    case 1: bgNight(t);  break;
    case 2: bgGrid(t);   break;
    default: bgHearts(t); break;
  }
  drawWindow(WIN_X, WIN_Y, WIN_W, WIN_H, title, BAR[variant & 3]);
  drawTextBlock(text, gauge, t);
  sticker(variant, t);
  sparkle(2, 2, t, 0);   sparkle(224, 3, t, 2);
  sparkle(2, 122, t, 4); sparkle(226, 124, t, 1);
}

// ----------------------------------------------------------------------------
//  Splash: frandorturtles.exe
//  (♡ y ୨୧ no existen en la fuente integrada: se dibujan como pixel art)
// ----------------------------------------------------------------------------
static void renderSplash(uint32_t t) {
  gradientBg({255, 214, 236}, {214, 190, 255});
  const int wx = 4, wy = 6, ww = 232, wh = 123;
  drawWindow(wx, wy, ww, wh, "", {150, 120, 235});
  sparkle(10, 28, t, 0);  sparkle(222, 30, t, 3);
  sparkle(10, 96, t, 2);  sparkle(222, 96, t, 5);
  sparkle(14, 4, t, 1);   sparkle(220, 2, t, 4);

  spr.setTextFont(1);
  // ♡ hello, little world ♡
  spr.setTextSize(1); spr.setTextColor(COL_INK);
  spr.drawString("hello, little world", 120 - 57, 29);
  drawBitmap(HEART, 120 - 57 - 13, 28);
  drawBitmap(HEART, 120 + 57 + 4, 28);

  // ୨୧
  int bob = (int)(1.5f * sinf(t / 200.0f));
  drawBitmap(ORN, 120 - 23, 44 + bob, 2);
  drawBitmap(ORN, 120 + 9,  44 - bob, 2, true);

  // frandorturtles.exe
  spr.setTextSize(2);
  spr.setTextColor(COL_LAV);   spr.drawString("frandorturtles.exe", 12 + 2, 64 + 2);
  spr.setTextColor(COL_PINK_D); spr.drawString("frandorturtles.exe", 12, 64);

  // loading a little bit of magic...
  spr.setTextSize(1); spr.setTextColor(COL_INK);
  spr.drawString("loading a little bit of magic...", 120 - 96, 85);

  // barra de progreso (llega al 100% justo al final del splash)
  float p = (float)t / (float)(SPLASH_MS - FRAME_MS);
  if (p > 1.0f) p = 1.0f;
  const int bx = 40, by = 108, bw = 160, bh = 11;
  int fw = (int)((bw - 4) * p);
  spr.fillRect(bx, by, bw, bh, COL_INK);
  spr.fillRect(bx + 2, by + 2, bw - 4, bh - 4, COL_WHITE);
  spr.fillRect(bx + 2, by + 2, fw, bh - 4, COL_PINK);
  int off = (int)((t / 30) % 8);
  for (int sx = -8 + off; sx < fw; sx += 8) {          // rayitas que avanzan
    int x0 = iclamp(sx, 0, fw), x1 = iclamp(sx + 3, 0, fw);
    if (x1 > x0) spr.fillRect(bx + 2 + x0, by + 2, x1 - x0, bh - 4, COL_PINK_P);
  }
  // la tortuguita camina sobre la barra
  drawBitmap(TURTLE, bx + 2 + fw - 20, 95 + (int)((t / 150) & 1), 2);
}

// ----------------------------------------------------------------------------
//  Imágenes de la galería
// ----------------------------------------------------------------------------
static uint16_t wrapIndex(uint16_t i) { return GALLERY_COUNT ? (uint16_t)(i % (GALLERY_COUNT ? GALLERY_COUNT : 1)) : 0; }

static void renderImage(uint16_t index, uint32_t t) {
  spr.fillSprite(TFT_BLACK);
  if (GALLERY_COUNT == 0) return;
  const GalleryImage& g = GALLERY[wrapIndex(index)];
  spr.setSwapBytes(IMAGE_SWAP_BYTES);
  spr.pushImage((SCREEN_W - g.w) / 2, (SCREEN_H - g.h) / 2, g.w, g.h, (uint16_t*)g.data);
#if SPARKLES_ON_IMAGES
  sparkle(4, 4, t, 0);    sparkle(228, 6, t, 2);
  sparkle(6, 124, t, 4);  sparkle(226, 122, t, 1);
#endif
}

// ----------------------------------------------------------------------------
//  Transición: cortina rosa/lavanda a rayas que baja y sube
// ----------------------------------------------------------------------------
static void curtain(float p, bool closing) {
  const int cw = 8, cols = SCREEN_W / cw;
  for (int c = 0; c < cols; c++) {
    float stag = closing ? (float)c / (cols - 1) : (float)(cols - 1 - c) / (cols - 1);
    float cp = p * 1.6f - stag * 0.6f;
    if (cp <= 0.0f) continue;
    if (cp > 1.0f) cp = 1.0f;
    int h = (int)(cp * SCREEN_H);
    spr.fillRect(c * cw, 0, cw, h, (c & 1) ? COL_PINK_P : COL_LAV);
    if (h < SCREEN_H - 1) spr.fillRect(c * cw, h, cw, 2, COL_WHITE);
  }
}

// ----------------------------------------------------------------------------
//  Lista de escenas (se genera sola a partir de config.h)
// ----------------------------------------------------------------------------
enum SceneKind : uint8_t { K_SPLASH, K_INTRO, K_MESSAGE, K_IMAGE };
struct Scene { SceneKind kind; uint16_t index; uint16_t variant; };

static Scene nextScene() {
  static uint16_t intro = 0, msg = 0, sinceImage = 0, image = 0;
  static uint16_t textCount = 0;

  if (msg >= MESSAGE_COUNT) { msg = 0; intro = 0; }       // vuelve a empezar (sin splash)

  if (intro < INTRO_COUNT) {
    Scene s = {K_INTRO, intro++, textCount++};
    return s;
  }
  if (IMAGE_EVERY > 0 && GALLERY_COUNT > 0 && sinceImage >= IMAGE_EVERY) {
    sinceImage = 0;
    Scene s = {K_IMAGE, wrapIndex(image), 0};
    image++;
    return s;
  }
  sinceImage++;
  Scene s = {K_MESSAGE, msg++, textCount++};
  return s;
}

static uint32_t sceneDuration(const Scene& s) {
  switch (s.kind) {
    case K_SPLASH:  return SPLASH_MS;
    case K_INTRO:   return INTRO_MS;
    case K_MESSAGE: return MESSAGE_MS;
    default:        return IMAGE_MS;
  }
}

static void renderScene(const Scene& s, uint32_t t) {
  switch (s.kind) {
    case K_SPLASH:  renderSplash(t); break;
    case K_INTRO:   renderTextScene(INTRO[s.index].title, INTRO[s.index].text, -1, s.variant, t); break;
    case K_MESSAGE: renderTextScene(MESSAGES[s.index].title, MESSAGES[s.index].text,
                                    MESSAGES[s.index].gauge, s.variant, t); break;
    case K_IMAGE:   renderImage(s.index, t); break;
  }
}

// ----------------------------------------------------------------------------
//  setup / loop
// ----------------------------------------------------------------------------
static const uint8_t BL_CHANNEL = 0;

void setup() {
  setCpuFrequencyMhz(CPU_MHZ);          // sin Wi-Fi ni Bluetooth: nunca se encienden

  // Backlight por PWM (núcleo Arduino-ESP32 2.x). Empieza apagado para evitar un destello.
  ledcSetup(BL_CHANNEL, 5000, 8);
  ledcAttachPin(PIN_BACKLIGHT, BL_CHANNEL);
  ledcWrite(BL_CHANNEL, 0);

  tft.init();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(TFT_BLACK);

  spr.setColorDepth(16);
  if (spr.createSprite(SCREEN_W, SCREEN_H) == nullptr) {   // ~63 KB de RAM
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Sin memoria :(", 10, 10);
    ledcWrite(BL_CHANNEL, BACKLIGHT_LEVEL);
    delay(3000);
    ESP.restart();
  }
  spr.setTextFont(1);
  spr.setTextWrap(false);
}

void loop() {
  static Scene    scene;
  static uint32_t sceneStart = 0;
  static bool     started = false;
  static bool     blDone = false;

  uint32_t frameStart = millis();

  if (!started) {                                           // primer fotograma: splash
    scene = {K_SPLASH, 0, 0};
    renderScene(scene, 0);
    spr.pushSprite(0, 0);
    sceneStart = millis();                                  // el reloj de 2 s empieza al mostrar el logo
    started = true;
    return;
  }

  uint32_t t   = frameStart - sceneStart;
  uint32_t dur = sceneDuration(scene);
  if (t >= dur) {                                           // siguiente pantalla
    scene = nextScene();
    sceneStart = frameStart;
    t = 0;
    dur = sceneDuration(scene);
  }

  renderScene(scene, t);

  if (scene.kind != K_SPLASH) {                             // cortina de transición
    if (t < TRANSITION_MS)
      curtain(1.0f - (float)t / TRANSITION_MS, false);
    else if (t + TRANSITION_MS > dur)
      curtain((float)(t + TRANSITION_MS - dur) / TRANSITION_MS, true);
  }
  spr.pushSprite(0, 0);

  if (!blDone) {                                            // encendido suave del backlight (300 ms)
    uint32_t bt = millis() - sceneStart;
    if (bt >= 300) { ledcWrite(BL_CHANNEL, BACKLIGHT_LEVEL); blDone = true; }
    else           ledcWrite(BL_CHANNEL, (uint32_t)BACKLIGHT_LEVEL * bt / 300);
  }

  // Ritmo de fotogramas. Durante el splash se ajusta para cambiar exactamente a los 2 s.
  uint32_t used = millis() - frameStart;
  uint32_t wait = used < FRAME_MS ? FRAME_MS - used : 0;
  if (scene.kind == K_SPLASH) {
    uint32_t sinceStart = millis() - sceneStart;
    uint32_t left = sinceStart < dur ? dur - sinceStart : 0;
    if (left < wait) wait = left;
  }
  if (wait) delay(wait);
}
