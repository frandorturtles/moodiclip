# assets/images/

Pon aquí tus imágenes (`.png`, `.jpg`, `.webp`, `.gif`, `.bmp`) y ejecuta desde la raíz del repo:

```bash
python tools/image_converter.py
```

El conversor las recorta a 240 x 135 px, las pasa a RGB565 y crea los archivos `.h` en
`assets/images/converted/`, junto con `gallery.h`, que es el índice que lee el firmware.
Las imágenes se intercalan solas entre los mensajes. Guía completa: `docs/image_conversion.md`.

Los PNG de este directorio son ilustraciones de ejemplo originales (ver `tools/make_example_art.py`).
