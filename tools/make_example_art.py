#!/usr/bin/env python3
"""
make_example_art.py — genera 4 ilustraciones pixel art ORIGINALES de ejemplo.

Cada una se dibuja en un lienzo de 80x45 px (assets/pixel_art/) y se amplía x3
con "nearest neighbor" a 240x135 px (assets/images/), que es justo la
resolución de la pantalla. Úsalas como plantilla o reemplázalas por las tuyas.

    python tools/make_example_art.py
    python tools/image_converter.py        # las convierte a RGB565
"""
import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
PIXEL_DIR = ROOT / "assets" / "pixel_art"
IMAGES_DIR = ROOT / "assets" / "images"
W, H, SCALE = 80, 45, 3

PAL = {
    "K": (74, 42, 106),    # contorno morado oscuro
    "P": (255, 105, 180),  # rosa fuerte
    "p": (255, 182, 217),  # rosa pastel
    "O": (219, 60, 140),   # rosa oscuro
    "W": (255, 255, 255),
    "L": (200, 170, 255),  # lavanda
    "l": (232, 218, 255),  # lavanda clara
    "B": (160, 205, 255),  # azul bebé
    "Y": (255, 240, 150),  # amarillo
    "G": (150, 220, 160),  # verde pastel
    "R": (255, 140, 165),  # mejilla
}

HEART = [".KKK.KKK.", "KPPPKPPPK", "KPWPPPPPK", "KPPPPPPPK",
         ".KPPPPPK.", "..KPPPK..", "...KPK...", "....K...."]
STAR = ["...Y...", "...Y...", "..YWY..", "YYWWWYY", "..YWY..", "...Y...", "...Y..."]
STAR_S = ["..Y..", ".YWY.", "YWWWY", ".YWY.", "..Y.."]
FLOWER = [".pp.pp.", "ppppppp", "pppYppp", ".ppppp.", "..pGp..", "...G...", "..GG..."]
BOW = [".PPP...PPP.", "PPPPPOPPPPP", "PWPPPOPPPWP", "PPPPPOPPPPP", ".PPP...PPP."]
FACE = ["..KKKKKKK..", ".KWWWWWWWK.", "KWWWWWWWWWK", "KWWKWWWKWWK",
        "KWRWWWWWRWK", "KWWWWKWWWWK", ".KWWWWWWWK.", "..KKKKKKK.."]
TURTLE = ["....KKKKK....", "...KGGPGGK.KK", "..KGPGGGPGKGK", "..KGGGPGGGKGK",
          "...KKKKKKKKK.", "...KK.....KK."]
MOON = ["..YYYY...", ".YYY.....", "YYY......", "YYY......", "YYY......", ".YYY.....", "..YYYY..."]
CLOUD = ["...WWWW....", ".WWWWWWWW..", "WWWWWWWWWWW", "WWWWWWWWWWW", ".lllllllll."]


def stamp(img, x, y, rows):
    for j, row in enumerate(rows):
        for i, ch in enumerate(row):
            if ch in PAL and 0 <= x + i < img.width and 0 <= y + j < img.height:
                img.putpixel((x + i, y + j), PAL[ch])


def gradient(img, y0, y1, c0, c1):
    d = ImageDraw.Draw(img)
    for y in range(y0, y1):
        t = (y - y0) / max(1, y1 - y0 - 1)
        d.line([(0, y), (img.width, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(c0, c1)))


def save(img, name):
    PIXEL_DIR.mkdir(parents=True, exist_ok=True)
    IMAGES_DIR.mkdir(parents=True, exist_ok=True)
    img.save(PIXEL_DIR / f"{name}.png")
    img.resize((W * SCALE, H * SCALE), Image.NEAREST).save(IMAGES_DIR / f"{name}.png")
    print("✓", name)


def night_turtle():
    rnd = random.Random(7)
    img = Image.new("RGB", (W, H))
    gradient(img, 0, 34, (28, 22, 78), (110, 76, 170))
    for _ in range(40):
        img.putpixel((rnd.randrange(W), rnd.randrange(30)), (255, 255, 255))
    for x, y in [(8, 5), (30, 12), (50, 4), (70, 16), (20, 20)]:
        stamp(img, x, y, STAR_S)
    stamp(img, 58, 6, MOON)
    d = ImageDraw.Draw(img)
    for x in range(W):                                  # colina
        top = 33 + int(2.5 * math.sin(x / 8))
        for y in range(top, H):
            t = (y - top) / max(1, H - top)
            img.putpixel((x, y), (int(255 - 55 * t), int(182 - 12 * t), int(217 + 38 * t)))
    stamp(img, 33, 29, TURTLE)
    for x in (6, 18, 52, 66, 74):
        stamp(img, x, 33 + int(2.5 * math.sin(x / 8)) - 3, FLOWER)
    stamp(img, 14, 22, HEART)
    stamp(img, 60, 22, BOW)
    save(img, "night_turtle")


def hello_window():
    img = Image.new("RGB", (W, H))
    for y in range(H):                                  # fondo ajedrezado
        for x in range(W):
            img.putpixel((x, y), (232, 218, 255) if ((x // 4) + (y // 4)) % 2 else (255, 232, 246))
    d = ImageDraw.Draw(img)
    d.fontmode = "1"                                    # texto sin suavizado = pixel puro
    font = ImageFont.load_default(size=8)
    d.rectangle([10, 6, 69, 36], fill=PAL["K"])         # sombra/contorno
    d.rectangle([11, 7, 68, 35], fill=(255, 245, 252))
    d.rectangle([11, 7, 68, 14], fill=(130, 108, 225))  # barra de título
    d.text((14, 6), "hello??", fill=PAL["W"], font=font)
    for i, c in enumerate(("L", "L", "P")):             # botoncitos
        d.rectangle([54 + i * 5, 9, 57 + i * 5, 12], fill=PAL[c], outline=PAL["K"])
    d.text((14, 16), "still here :3", fill=PAL["K"], font=font)
    d.rectangle([28, 26, 50, 33], fill=PAL["p"], outline=PAL["K"])
    d.text((34, 24), "ok", fill=PAL["K"], font=font)
    stamp(img, 2, 36, BOW)
    stamp(img, 66, 37, HEART)
    stamp(img, 4, 2, STAR_S)
    stamp(img, 72, 2, STAR_S)
    save(img, "hello_window")


def flower_field():
    img = Image.new("RGB", (W, H))
    gradient(img, 0, 28, (170, 215, 255), (255, 214, 236))
    stamp(img, 8, 6, CLOUD)
    stamp(img, 52, 10, CLOUD)
    gradient(img, 28, H, (170, 232, 170), (110, 200, 150))
    for i, x in enumerate(range(2, 78, 9)):
        stamp(img, x, 30 + (i % 3) * 3, FLOWER)
        stamp(img, x + 4, 38 + (i % 2) * 2, FLOWER)
    stamp(img, 34, 9, HEART)
    stamp(img, 66, 3, STAR)
    stamp(img, 3, 18, STAR_S)
    save(img, "flower_field")


def dream_clouds():
    img = Image.new("RGB", (W, H))
    gradient(img, 0, H, (186, 160, 245), (255, 205, 230))
    d = ImageDraw.Draw(img)
    cols = [(255, 120, 150), (255, 190, 120), (255, 245, 150), (150, 225, 160), (150, 200, 255), (190, 160, 255)]
    for i, c in enumerate(cols):                        # arcoíris
        r = 36 - i * 2
        d.arc([40 - r, 52 - r, 40 + r, 52 + r], 180, 360, fill=c, width=2)
    for x, y in [(2, 28), (62, 30), (30, 34), (14, 6)]:
        stamp(img, x, y, CLOUD)
    stamp(img, 34, 26, FACE)
    for x, y in [(10, 12), (60, 8), (70, 20), (46, 4)]:
        stamp(img, x, y, STAR_S)
    stamp(img, 20, 20, HEART)
    stamp(img, 52, 18, HEART)
    save(img, "dream_clouds")


if __name__ == "__main__":
    for fn in (night_turtle, hello_window, flower_field, dream_clouds):
        fn()
