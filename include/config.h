// ============================================================================
//  config.h — TODO lo que quieras personalizar vive aquí ♡
//
//  Edita los tiempos, los mensajes y cómo se intercalan las imágenes.
//  No hace falta tocar src/main.cpp.
//
//  Texto: solo caracteres ASCII (la fuente retro integrada no tiene acentos
//  ni emojis). Usa el símbolo  ^  para dibujar un corazoncito pixel art ♡
// ============================================================================
#pragma once
#include <Arduino.h>
#include "../assets/images/converted/gallery.h"   // imágenes convertidas (lo genera tools/image_converter.py)

// ---------------------------------------------------------------------------
//  Pantalla y batería
// ---------------------------------------------------------------------------
#define SCREEN_ROTATION   1      // horizontal. Si ves la pantalla al revés, cambia a 3 (gira 180°)
#define SCREEN_W          240    // ancho lógico tras rotar
#define SCREEN_H          135    // alto lógico tras rotar
#define PIN_BACKLIGHT     4      // GPIO4 = backlight en la T-Display (según LilyGO)
#define BACKLIGHT_LEVEL   90     // brillo 0-255 (PWM). Menos brillo = menos consumo y mejor
                                 // para los ojos; 60-110 se ve bien en interiores.
#define CPU_MHZ           80     // 80 es suficiente; 240 gastaría más sin beneficio
#define FRAME_MS          50     // 50 ms = 20 fps. Súbelo (ej. 70) para ahorrar más batería.

// ---------------------------------------------------------------------------
//  Tiempos (milisegundos)
// ---------------------------------------------------------------------------
#define SPLASH_MS         2000   // pantalla frandorturtles.exe: exactamente 2 s
#define INTRO_MS          4000   // "Hi! I'm Francesca" y "Welcome..."
#define MESSAGE_MS        4000   // cada frase de "my moods"
#define IMAGE_MS          4000   // cada imagen de la galería
#define TRANSITION_MS     350    // duración de la cortina rosa (entrada y salida)
#define TYPE_MS_PER_CHAR  45     // velocidad de "máquina de escribir"

// ---------------------------------------------------------------------------
//  Galería
// ---------------------------------------------------------------------------
#define IMAGE_EVERY          3      // 1 imagen cada N mensajes (0 = no mostrar imágenes)
#define SPARKLES_ON_IMAGES   true   // estrellitas parpadeando sobre las imágenes
#define IMAGE_SWAP_BYTES     true   // si una imagen sale con colores raros, cambia a false

// ---------------------------------------------------------------------------
//  Presentación: las 2 primeras pantallas tras el logo
// ---------------------------------------------------------------------------
struct IntroScreen {
  const char* title;   // texto de la barra de la ventanita (máx. ~22 caracteres)
  const char* text;
};

static const IntroScreen INTRO[] = {
  {"hello.exe",   "Hi! I'm Francesca^"},
  {"welcome.exe", "Welcome to my little world!"},
};

// ---------------------------------------------------------------------------
//  My moods: frases de personalidad
//    title : texto de la barra de título (máx. ~22 caracteres)
//    text  : la frase (se ajusta sola al tamaño de la ventana)
//    gauge : 0-100 dibuja una barrita de batería animada; -1 = sin barrita
//  Para quitar una frase, borra su línea. Para agregar, copia una línea.
// ---------------------------------------------------------------------------
struct Message {
  const char* title;
  const char* text;
  int8_t      gauge;
};

static const Message MESSAGES[] = {
  {"mood_01.exe",   "I'm introverted^",                        -1},
  {"battery.exe",   "Social battery: 12%",                     12},
  {"daydream.exe",  "Currently daydreaming...",                -1},
  {"loading.exe",   "Please be patient, I'm processing.",      -1},
  {"thoughts.exe",  "Overthinking everything.",                -1},
  {"curiosity.exe", "Powered by curiosity.",                   -1},
  {"awkward.exe",   "A little socially awkward.",              -1},
  {"quiet.exe",     "I need a quiet place.",                   -1},
  {"romance.exe",   "Romanticizing my life.",                  -1},
  {"pro.exe",       "Professional overthinker.",               -1},
  {"my_world.exe",  "Currently in my own world.",              -1},
  {"ram.exe",       "Too many thoughts, too little RAM.",      -1},
  {"science.exe",   "I love science and pretty things.",       -1},
  {"sleepy.exe",    "Sleepy but ambitious.",                   -1},
  {"space.exe",     "Mentally somewhere in space.",            -1},
  {"dnd.exe",       "Do not disturb: recharging.",             -1},
  {"ideas.exe",     "Just a girl with too many ideas.",        -1},
  // --- frases originales extra (bórralas o cámbialas a tu gusto) ---
  {"tabs.exe",      "Brain: 47 tabs open.",                    -1},
  {"shy.exe",       "Be gentle, I'm shy^",                     -1},
  {"fairy.exe",     "Cyber-fairy in training.",                -1},
  {"error404.exe",  "Error 404: small talk not found.",        -1},
  {"pastel.exe",    "Dreaming in pastel.",                     -1},
  {"tea.exe",       "Fueled by tea and starlight.",            -1},
  {"soft.exe",      "Soft heart, loud thoughts.",              -1},
};

static const uint16_t INTRO_COUNT   = sizeof(INTRO) / sizeof(INTRO[0]);
static const uint16_t MESSAGE_COUNT = sizeof(MESSAGES) / sizeof(MESSAGES[0]);
