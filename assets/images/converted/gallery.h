// Generado por tools/image_converter.py — no editar a mano.
// Índice de la galería: el firmware recorre GALLERY[] automáticamente.
#pragma once
#include <Arduino.h>
#include "dream_clouds.h"
#include "flower_field.h"
#include "hello_window.h"
#include "night_turtle.h"

struct GalleryImage {
  const char*     name;
  const uint16_t* data;   // píxeles RGB565 en flash
  uint16_t        w;
  uint16_t        h;
};

static const GalleryImage GALLERY[] = {
  {"dream_clouds", img_dream_clouds, 240, 135},
  {"flower_field", img_flower_field, 240, 135},
  {"hello_window", img_hello_window, 240, 135},
  {"night_turtle", img_night_turtle, 240, 135},
};
static const uint16_t GALLERY_COUNT = sizeof(GALLERY) / sizeof(GALLERY[0]);
