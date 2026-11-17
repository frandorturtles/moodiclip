# assets/pixel_art/

Fuentes de pixel art **originales** de ejemplo, dibujadas en un lienzo de **80 x 45 px**.
`tools/make_example_art.py` las amplía x3 (sin suavizado) a 240 x 135 px y las guarda en
`assets/images/`, que es la resolución exacta de la pantalla.

Para hacer las tuyas: dibuja en 80 x 45 px (Aseprite, Piskel, Pixilart, etc.), guarda el PNG
aquí, amplíalo x3 con "nearest neighbor" y ponlo en `assets/images/`. Luego:

```bash
python tools/image_converter.py
```
