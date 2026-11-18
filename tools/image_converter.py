#!/usr/bin/env python3
"""
image_converter.py — convierte imágenes a RGB565 para el Digital Mood Hair Clip.

Pantalla de la LilyGO T-Display: 240 x 135 px (horizontal), RGB565.

Uso rápido (desde la raíz del repositorio):

    pip install -r tools/requirements.txt

    # 1) Convierte TODAS las imágenes de assets/images/ (png, jpg, webp, gif, bmp)
    python tools/image_converter.py

    # 2) Convierte archivos concretos (recorte "cover" centrado por defecto)
    python tools/image_converter.py convert mi_foto.jpg --name mi_foto

    # 3) Descarga una imagen (URL directa) y la convierte
    python tools/image_converter.py download https://ejemplo.com/foto.jpg --name foto

    # 4) Quitar / listar imágenes de la galería
    python tools/image_converter.py remove foto
    python tools/image_converter.py list
    python tools/image_converter.py rebuild     # regenera gallery.h

Cada conversión crea  assets/images/converted/<nombre>.h  y regenera
assets/images/converted/gallery.h  (el índice que lee el firmware).
Las imágenes se intercalan solas entre los mensajes (ver include/config.h).
"""
from __future__ import annotations

import argparse
import re
import sys
import urllib.request
from pathlib import Path
from urllib.parse import urlparse

try:
    from PIL import Image, ImageEnhance, ImageOps
except ImportError:  # pragma: no cover
    sys.exit("Falta Pillow. Instala con:  pip install -r tools/requirements.txt")

ROOT = Path(__file__).resolve().parent.parent
IMAGES_DIR = ROOT / "assets" / "images"
ORIGINALS_DIR = IMAGES_DIR / "originals"      # ignorado por git (ver .gitignore)
OUT_DIR = IMAGES_DIR / "converted"
GALLERY_H = "gallery.h"
EXTS = {".png", ".jpg", ".jpeg", ".webp", ".gif", ".bmp"}
FLASH_WARN_BYTES = 2_400_000   # partición huge_app ≈ 3 MB, menos el código


# ----------------------------------------------------------------------------
# Utilidades
# ----------------------------------------------------------------------------
def slug(text: str) -> str:
    """Nombre seguro para identificador C: minúsculas, a-z 0-9 _"""
    s = re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")
    return s or "img"


def rel(path: Path) -> Path:
    """Ruta relativa a la raíz del repo (o la ruta tal cual si está fuera)."""
    try:
        return path.relative_to(ROOT)
    except ValueError:
        return path


def rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def parse_color(text: str) -> tuple[int, int, int]:
    text = text.lstrip("#")
    if len(text) != 6:
        raise argparse.ArgumentTypeError("Usa un color hexadecimal, p. ej. 000000 o FFC0E0")
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))  # type: ignore[return-value]


def parse_focus(text: str) -> tuple[float, float]:
    try:
        x, y = (float(v) for v in text.split(","))
        assert 0 <= x <= 1 and 0 <= y <= 1
        return x, y
    except Exception:
        raise argparse.ArgumentTypeError("Usa --focus X,Y con valores de 0 a 1 (ej. 0.5,0.3)")


# ----------------------------------------------------------------------------
# Procesado de imagen
# ----------------------------------------------------------------------------
def prepare(img: Image.Image, args) -> Image.Image:
    """Recorta/redimensiona la imagen al tamaño de pantalla y aplica efectos."""
    w, h = args.width, args.height

    # Transparencia -> fondo plano
    if img.mode in ("RGBA", "LA", "P"):
        img = img.convert("RGBA")
        bg = Image.new("RGBA", img.size, args.bg + (255,))
        bg.alpha_composite(img)
        img = bg
    img = img.convert("RGB")

    # Método de remuestreo: pixel art pequeño -> NEAREST; fotos -> LANCZOS
    if args.resample == "auto":
        small = img.width <= w and img.height <= h
        resample = Image.NEAREST if small else Image.LANCZOS
    else:
        resample = Image.NEAREST if args.resample == "nearest" else Image.LANCZOS

    pix = max(1, args.pixelate)
    tw, th = max(1, w // pix), max(1, h // pix)   # tamaño "lógico" antes de pixelar

    if args.fit == "cover":        # llena la pantalla recortando lo que sobre
        out = ImageOps.fit(img, (tw, th), resample, centering=args.focus)
    elif args.fit == "contain":    # se ve completa, con bandas del color --bg
        out = ImageOps.contain(img, (tw, th), resample)
        canvas = Image.new("RGB", (tw, th), args.bg)
        canvas.paste(out, ((tw - out.width) // 2, (th - out.height) // 2))
        out = canvas
    else:                          # stretch: deforma para llenar
        out = img.resize((tw, th), resample)

    if args.colors and args.colors > 1:   # paleta reducida = look pixel art
        dither = Image.FLOYDSTEINBERG if args.dither else Image.NONE
        out = out.quantize(args.colors, dither=dither).convert("RGB")

    if args.saturation != 1.0:
        out = ImageEnhance.Color(out).enhance(args.saturation)
    if args.brightness != 1.0:
        out = ImageEnhance.Brightness(out).enhance(args.brightness)

    if pix > 1:                    # pixelado: reescala con bloques duros
        out = out.resize((tw * pix, th * pix), Image.NEAREST)
        if out.size != (w, h):     # relleno si w/h no son múltiplos de pix
            canvas = Image.new("RGB", (w, h), args.bg)
            canvas.paste(out, (0, 0))
            out = canvas
    return out


def write_header(name: str, img: Image.Image, out_dir: Path) -> Path:
    w, h = img.size
    raw = img.tobytes()   # RGB, 3 bytes por píxel
    data = [rgb565(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, len(raw), 3)]
    up = name.upper()
    lines = [
        "// Generado por tools/image_converter.py — no editar a mano.",
        f"// Imagen: {name}  ({w}x{h}, RGB565, {len(data) * 2} bytes)",
        "#pragma once",
        "#include <Arduino.h>",
        f"#define IMG_{up}_W {w}",
        f"#define IMG_{up}_H {h}",
        f"static const uint16_t img_{name}[{w * h}] PROGMEM = {{",
    ]
    for i in range(0, len(data), 12):
        lines.append("  " + ", ".join(f"0x{v:04X}" for v in data[i:i + 12]) + ",")
    lines.append("};")
    path = out_dir / f"{name}.h"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def rebuild_gallery(out_dir: Path) -> None:
    """Regenera gallery.h a partir de los .h que haya en la carpeta."""
    entries = []
    for hdr in sorted(out_dir.glob("*.h")):
        if hdr.name == GALLERY_H:
            continue
        text = hdr.read_text(encoding="utf-8", errors="ignore")
        up = hdr.stem.upper()
        mw = re.search(rf"#define IMG_{re.escape(up)}_W (\d+)", text)
        mh = re.search(rf"#define IMG_{re.escape(up)}_H (\d+)", text)
        if mw and mh:
            entries.append((hdr.stem, int(mw.group(1)), int(mh.group(1))))

    L = [
        "// Generado por tools/image_converter.py — no editar a mano.",
        "// Índice de la galería: el firmware recorre GALLERY[] automáticamente.",
        "#pragma once",
        "#include <Arduino.h>",
    ]
    L += [f'#include "{n}.h"' for n, _, _ in entries]
    L += [
        "",
        "struct GalleryImage {",
        "  const char*     name;",
        "  const uint16_t* data;   // píxeles RGB565 en flash",
        "  uint16_t        w;",
        "  uint16_t        h;",
        "};",
        "",
    ]
    if entries:
        L.append("static const GalleryImage GALLERY[] = {")
        for n, w, h in entries:
            L.append(f'  {{"{n}", img_{n}, {w}, {h}}},')
        L.append("};")
        L.append("static const uint16_t GALLERY_COUNT = sizeof(GALLERY) / sizeof(GALLERY[0]);")
    else:  # galería vacía: array de relleno para que compile
        L.append('static const GalleryImage GALLERY[] = {{"", nullptr, 0, 0}};')
        L.append("static const uint16_t GALLERY_COUNT = 0;")
    (out_dir / GALLERY_H).write_text("\n".join(L) + "\n", encoding="utf-8")

    total = sum(w * h * 2 for _, w, h in entries)
    print(f"Galería: {len(entries)} imagen(es), {total / 1024:.0f} KB en flash.")
    if total > FLASH_WARN_BYTES:
        print("⚠  Mucha imagen: puede no caber en la flash. Quita alguna o reduce el tamaño.")


# ----------------------------------------------------------------------------
# Comandos
# ----------------------------------------------------------------------------
def cmd_convert(args) -> int:
    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    files: list[Path] = []
    if args.files:
        for f in args.files:
            p = Path(f)
            files += sorted(q for q in p.iterdir() if q.suffix.lower() in EXTS) if p.is_dir() else [p]
    else:
        files = sorted(p for p in IMAGES_DIR.iterdir() if p.is_file() and p.suffix.lower() in EXTS)
        if ORIGINALS_DIR.exists():
            files += sorted(p for p in ORIGINALS_DIR.glob("*") if p.suffix.lower() in EXTS)

    if not files:
        print("No encontré imágenes. Pon archivos .png/.jpg en assets/images/ o usa 'download'.")
        rebuild_gallery(out_dir)
        return 1
    if args.name and len(files) > 1:
        sys.exit("--name solo se puede usar con una imagen.")

    for f in files:
        if not f.exists():
            print(f"✗ No existe: {f}")
            continue
        name = slug(args.name or f.stem)
        with Image.open(f) as im:
            im.seek(0)                   # GIF animado: primer fotograma
            result = prepare(im.copy(), args)
        hdr = write_header(name, result, out_dir)
        if args.preview:
            (out_dir / "previews").mkdir(exist_ok=True)
            result.save(out_dir / "previews" / f"{name}.png")
        print(f"✓ {f.name} -> {rel(hdr)}  ({result.width}x{result.height})")
    rebuild_gallery(out_dir)
    return 0


def cmd_download(args) -> int:
    ORIGINALS_DIR.mkdir(parents=True, exist_ok=True)
    parsed = urlparse(args.url)
    if parsed.scheme not in ("http", "https"):
        sys.exit("La URL debe empezar con http:// o https://")
    req = urllib.request.Request(args.url, headers={"User-Agent": "Mozilla/5.0 (hair-clip-converter)"})
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            ctype = resp.headers.get_content_type()
            body = resp.read()
    except Exception as exc:
        sys.exit(f"No pude descargar la imagen: {exc}\n"
                 "Tip: en Pinterest abre la imagen, clic derecho -> 'Copiar dirección de imagen' "
                 "(debe terminar en .jpg/.png), o guárdala y ponla en assets/images/.")
    if not ctype.startswith("image/"):
        sys.exit(f"La URL no es una imagen directa (tipo: {ctype}).")
    ext = {"image/jpeg": ".jpg", "image/png": ".png", "image/webp": ".webp",
           "image/gif": ".gif", "image/bmp": ".bmp"}.get(ctype, ".img")
    name = slug(args.name or Path(parsed.path).stem or "descarga")
    dest = ORIGINALS_DIR / f"{name}{ext}"
    dest.write_bytes(body)
    print(f"✓ Descargada en {rel(dest)}")
    print("  (Recuerda: respeta los derechos de autor; 'originals/' no se sube a GitHub.)")
    args.files, args.name = [str(dest)], name
    return cmd_convert(args)


def cmd_remove(args) -> int:
    out_dir = Path(args.out)
    name = slug(args.name)
    hdr = out_dir / f"{name}.h"
    if not hdr.exists():
        sys.exit(f"No existe {rel(hdr)}. Usa 'list' para ver los nombres.")
    hdr.unlink()
    rebuild_gallery(out_dir)
    print(f"✓ Quitada '{name}' de la galería.")
    return 0


def cmd_list(args) -> int:
    out_dir = Path(args.out)
    hdrs = [h for h in sorted(out_dir.glob("*.h")) if h.name != GALLERY_H]
    print(f"{len(hdrs)} imagen(es) en la galería:")
    for h in hdrs:
        print(f"  - {h.stem}")
    return 0


def cmd_rebuild(args) -> int:
    rebuild_gallery(Path(args.out))
    return 0


# ----------------------------------------------------------------------------
def add_convert_options(p: argparse.ArgumentParser) -> None:
    p.add_argument("--width", type=int, default=240, help="ancho de salida (240)")
    p.add_argument("--height", type=int, default=135, help="alto de salida (135)")
    p.add_argument("--fit", choices=["cover", "contain", "stretch"], default="cover",
                   help="cover: recorta | contain: entera con bandas | stretch: deforma")
    p.add_argument("--focus", type=parse_focus, default=(0.5, 0.5),
                   help="punto a conservar al recortar, X,Y de 0 a 1 (ej. 0.5,0.3)")
    p.add_argument("--bg", type=parse_color, default=(0, 0, 0),
                   help="color de fondo/bandas/transparencias, hex (000000)")
    p.add_argument("--pixelate", type=int, default=1, metavar="N",
                   help="efecto pixel art: bloques de N x N píxeles (1 = sin efecto)")
    p.add_argument("--colors", type=int, default=0,
                   help="reduce la paleta a N colores (0 = sin reducir)")
    p.add_argument("--dither", action="store_true", help="tramado al reducir colores")
    p.add_argument("--saturation", type=float, default=1.0, help="1.0 = igual, 1.2 = más color")
    p.add_argument("--brightness", type=float, default=1.0, help="1.0 = igual")
    p.add_argument("--resample", choices=["auto", "nearest", "lanczos"], default="auto")
    p.add_argument("--preview", action="store_true",
                   help="guarda un PNG de cómo quedará en converted/previews/")
    p.add_argument("--out", default=str(OUT_DIR), help="carpeta de salida")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Convierte imágenes a RGB565 (T-Display 240x135).")
    sub = parser.add_subparsers(dest="cmd")

    p_conv = sub.add_parser("convert", help="convierte imágenes (por defecto, todas las de assets/images)")
    p_conv.add_argument("files", nargs="*", help="archivos o carpetas (vacío = assets/images/)")
    p_conv.add_argument("--name", help="nombre en la galería (solo con 1 imagen)")
    add_convert_options(p_conv)
    p_conv.set_defaults(func=cmd_convert)

    p_dl = sub.add_parser("download", help="descarga una imagen por URL directa y la convierte")
    p_dl.add_argument("url")
    p_dl.add_argument("--name")
    add_convert_options(p_dl)
    p_dl.set_defaults(func=cmd_download)

    p_rm = sub.add_parser("remove", help="quita una imagen de la galería")
    p_rm.add_argument("name")
    p_rm.add_argument("--out", default=str(OUT_DIR))
    p_rm.set_defaults(func=cmd_remove)

    p_ls = sub.add_parser("list", help="lista las imágenes de la galería")
    p_ls.add_argument("--out", default=str(OUT_DIR))
    p_ls.set_defaults(func=cmd_list)

    p_rb = sub.add_parser("rebuild", help="regenera gallery.h (útil si borraste .h a mano)")
    p_rb.add_argument("--out", default=str(OUT_DIR))
    p_rb.set_defaults(func=cmd_rebuild)

    if not argv or argv[0] not in {"convert", "download", "remove", "list", "rebuild", "-h", "--help"}:
        argv = ["convert"] + argv          # sin subcomando = convert
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
