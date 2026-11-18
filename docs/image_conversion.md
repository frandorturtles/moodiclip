# Agregar tus propias imágenes

La pantalla es de **240 x 135 px** (horizontal). `tools/image_converter.py` prepara cualquier imagen para ella.
Las imágenes quedan guardadas dentro del firmware: **no hace falta Internet** para mostrarlas.

## Requisitos

```bash
python -m pip install -r tools/requirements.txt     # solo Pillow
```

## Flujo básico (3 pasos)

1. Copia tus imágenes (`.png`, `.jpg`, `.webp`, `.gif`, `.bmp`) a `assets/images/`.
2. Desde la raíz del repo:
   ```bash
   python tools/image_converter.py
   ```
3. Compila y sube de nuevo: `pio run -t upload`.

Las imágenes aparecen solas intercaladas entre los mensajes (1 cada `IMAGE_EVERY` mensajes, ver `include/config.h`).
Cada conversión crea `assets/images/converted/<nombre>.h` y regenera `assets/images/converted/gallery.h`.

## Descargar desde Pinterest u otra web

Pinterest no permite bajar imágenes automáticamente desde la página del pin. Dos opciones:

- **Guardar la imagen** (clic derecho → *Guardar imagen como…*) en `assets/images/`.
- **URL directa:** clic derecho → *Copiar dirección de imagen* (debe terminar en `.jpg`/`.png`) y:
  ```bash
  python tools/image_converter.py download "https://i.pinimg.com/…/imagen.jpg" --name mi_imagen
  ```
  Se guarda en `assets/images/originals/` y se convierte.

> **Derechos de autor:** las imágenes de Pinterest suelen ser obra de otras personas. Para uso personal en tu clip está bien
> que las uses, pero **no las subas a un repositorio público** sin permiso de quien las creó. Por eso
> `assets/images/originals/` está en `.gitignore`. Los `.h` convertidos también contienen la imagen: si publicas el repo,
> deja solo tus propias creaciones o las de ejemplo.

## Opciones útiles

| Opción | Qué hace | Ejemplo |
|---|---|---|
| `--name NOMBRE` | nombre en la galería (1 imagen) | `--name gatito` |
| `--fit cover` (defecto) | llena la pantalla, recortando lo que sobre | |
| `--fit contain` | se ve completa, con bandas de color | `--fit contain --bg FFC0E0` |
| `--fit stretch` | deforma para llenar | |
| `--focus X,Y` | qué parte conservar al recortar (0–1) | `--focus 0.5,0.2` (parte alta) |
| `--pixelate N` | efecto pixel art con bloques de N px | `--pixelate 3` |
| `--colors N` | reduce la paleta a N colores | `--colors 16 --dither` |
| `--saturation`, `--brightness` | retoque de color/brillo | `--saturation 1.2` |
| `--preview` | guarda un PNG de cómo quedará | |

Receta para convertir una foto cualquiera en algo "pixel art coquette":

```bash
python tools/image_converter.py convert foto.jpg --name foto --pixelate 3 --colors 16 --saturation 1.2 --preview
```

## Quitar o listar imágenes

```bash
python tools/image_converter.py list
python tools/image_converter.py remove gatito
```

(`remove` regenera `gallery.h` solo. Si borras archivos `.h` a mano de `assets/images/converted/`,
ejecuta `python tools/image_converter.py rebuild`.)

## Pixel art propio

Dibuja en un lienzo de **80 x 45 px** (Aseprite, Piskel…), amplíalo **x3 con "nearest neighbor"** a 240 x 135 y ponlo
en `assets/images/`. Verás las ilustraciones de ejemplo en `assets/pixel_art/` (80 x 45) y `assets/images/` (240 x 135).

## Límite de memoria

Cada imagen de 240 x 135 ocupa **≈ 63 KB** de flash. Con la partición `huge_app` (≈ 3 MB) caben unas 30–40 imágenes;
el conversor avisa si te pasas.

## Si los colores salen raros

En `include/config.h` cambia `IMAGE_SWAP_BYTES` de `true` a `false` (o al revés), vuelve a compilar y sube.
