#!/usr/bin/env python3
"""
make_diagrams.py — genera los diagramas de docs/img/ (conexiones y capas de montaje)
y el collage de la galería de ejemplo.

    python tools/make_diagrams.py
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "docs" / "img"

INK = (74, 42, 106)
PINK = (255, 182, 217)
HOT = (232, 72, 150)
LAV = (214, 190, 255)
CREAM = (255, 247, 252)
GREEN = (120, 200, 150)
BLUE = (150, 190, 245)
GRAY = (190, 190, 200)
AMBER = (255, 205, 120)


def font(size):
    """Fuente con acentos: prueba varias comunes y cae en la integrada de Pillow."""
    for name in ("DejaVuSans.ttf", "LiberationSans-Regular.ttf", "Arial.ttf", "arial.ttf",
                 "Helvetica.ttc", "Verdana.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default(size=size)


def center_text(d, box, text, size=22, fill=INK):
    f = font(size)
    x0, y0, x1, y1 = box
    lines = text.split("\n")
    lh = size + 6
    y = (y0 + y1) / 2 - lh * len(lines) / 2
    for ln in lines:
        w = d.textlength(ln, font=f)
        d.text(((x0 + x1) / 2 - w / 2, y), ln, fill=fill, font=f)
        y += lh


def rbox(d, box, fill, text="", size=22, outline=INK, r=14, tfill=INK):
    d.rounded_rectangle(box, radius=r, fill=fill, outline=outline, width=3)
    if text:
        center_text(d, box, text, size, tfill)


def wiring():
    img = Image.new("RGB", (1400, 800), CREAM)
    d = ImageDraw.Draw(img)
    d.text((40, 24), "Diagrama de conexiones — Digital Mood Hair Clip", fill=HOT, font=font(34))
    d.text((40, 72), "No hay cableado externo salvo la batería: sin botones, sin sensores.", fill=INK, font=font(22))

    # Placa
    rbox(d, (520, 150, 1360, 640), LAV, "", r=24)
    d.text((545, 160), "LilyGO T-Display (ESP32-D0WDQ6 + pantalla ST7789V 1.14\" 240x135)", fill=INK, font=font(22))
    rbox(d, (560, 220, 860, 540), (255, 255, 255), "ESP32\n(80 MHz)\nsin Wi-Fi\nsin Bluetooth", 24)
    rbox(d, (1060, 220, 1330, 540), (60, 50, 90), "", r=10)
    rbox(d, (1075, 240, 1315, 520), PINK, "pantalla\nST7789V\n240 x 135", 22)
    # Señales internas
    sig = [("GPIO19  MOSI", 250), ("GPIO18  SCLK", 290), ("GPIO5   CS", 330),
           ("GPIO16  DC", 370), ("GPIO23  RST", 410), ("GPIO4   BL (PWM)", 450)]
    for label, y in sig:
        d.line([(860, y + 20), (1060, y + 20)], fill=INK, width=3)
        d.text((872, y - 6), label, fill=INK, font=font(19))
    d.text((545, 560), "Cableado interno de fábrica (verificado en la documentación de LilyGO).", fill=INK, font=font(19))
    d.text((545, 590), "No hace falta soldar ni conectar nada a estos pines.", fill=INK, font=font(19))

    # Conector JST
    rbox(d, (440, 330, 520, 440), (255, 255, 255), "JST\n1.25\nBAT", 17, r=8)
    # Batería
    rbox(d, (40, 300, 300, 470), BLUE, "LiPo 3.7 V\n400-450 mAh\n(con circuito de\nprotección)", 21)
    d.line([(300, 345), (440, 345)], fill=(220, 40, 60), width=8)
    d.line([(300, 425), (440, 425)], fill=(30, 30, 30), width=8)
    d.text((330, 305), "+ (rojo)", fill=(200, 30, 50), font=font(20))
    d.text((330, 440), "- (negro)", fill=INK, font=font(20))
    rbox(d, (40, 520, 440, 640), AMBER, "¡Comprueba la polaridad!\nEl + del cable debe coincidir\ncon el + marcado en la placa.", 19)

    # USB
    rbox(d, (760, 660, 1120, 760), GRAY, "USB-C: programar y cargar\n(la placa tiene cargador de LiPo)", 20)
    d.line([(940, 660), (940, 640)], fill=INK, width=6)
    img.save(OUT / "wiring.png")


def stack():
    img = Image.new("RGB", (1500, 760), CREAM)
    d = ImageDraw.Draw(img)
    d.text((40, 24), "Montaje en capas (vista lateral, sin escala)", fill=HOT, font=font(34))

    x0, x1 = 80, 760
    layers = [  # (alto, color, texto) de abajo hacia arriba
        (60, GRAY, "Hair clip (superficie plana)"),
        (28, AMBER, "Cinta Kapton / aislante eléctrico"),
        (34, (255, 255, 255), "Espuma doble cara, capa 1"),
        (100, BLUE, "Batería LiPo (sin presión, rodeada por marco de espuma)"),
        (34, (255, 255, 255), "Espuma separadora (sin apretar la batería)"),
        (70, GREEN, "Placa T-Display"),
        (24, (60, 50, 90), "Pantalla hacia afuera"),
    ]
    y = 560
    spans = []
    for h, col, label in layers:
        y -= h
        spans.append((y, h, col, label))
    for (ly, h, col, label) in spans:
        if "Batería" in label:       # batería más corta + marcos de espuma a los lados
            d.rectangle([x0, ly, x1, ly + h], fill=(255, 255, 255), outline=INK, width=3)
            d.rectangle([x0 + 170, ly + 6, x1 - 170, ly + h], fill=col, outline=INK, width=3)
            center_text(d, (x0 + 170, ly + 6, x1 - 170, ly + h), "batería", 22)
            center_text(d, (x0, ly, x0 + 170, ly + h), "marco de\nespuma", 18)
            center_text(d, (x1 - 170, ly, x1, ly + h), "marco de\nespuma", 18)
        else:
            d.rectangle([x0, ly, x1, ly + h], fill=col, outline=INK, width=3)
        d.line([(x1 + 10, ly + h / 2), (x1 + 50, ly + h / 2)], fill=INK, width=2)
        d.text((x1 + 60, ly + h / 2 - 13), label, fill=INK, font=font(21))

    d.text((40, 620), "Reglas: la batería nunca se dobla, perfora ni queda apretada.", fill=INK, font=font(22))
    d.text((40, 656), "El puerto USB-C queda libre en el borde del clip.", fill=INK, font=font(22))
    d.text((40, 692), "Cable de batería con holgura y sin pliegues.", fill=INK, font=font(22))
    img.save(OUT / "assembly_stack.png")


def top_view():
    img = Image.new("RGB", (1400, 600), CREAM)
    d = ImageDraw.Draw(img)
    d.text((40, 24), "Vista superior sugerida (clip plano ≥ 8 cm)", fill=HOT, font=font(34))
    rbox(d, (60, 160, 1340, 420), GRAY, "", r=60)
    # placa
    rbox(d, (330, 190, 830, 390), GREEN, "", r=10)
    rbox(d, (360, 205, 700, 375), PINK, "pantalla 240x135", 22, r=8)
    d.text((340, 395), "T-Display (≈ 51 x 25 mm)", fill=INK, font=font(20))
    # USB
    rbox(d, (800, 260, 850, 320), (255, 255, 255), "", r=6)
    d.line([(850, 290), (1010, 290)], fill=HOT, width=4)
    d.text((880, 255), "USB-C libre", fill=HOT, font=font(22))
    d.text((880, 305), "(borde exterior)", fill=HOT, font=font(20))
    d.text((180, 440), "Batería debajo de la placa, dentro de un marco de espuma", fill=INK, font=font(22))
    d.text((180, 476), "Cable JST con holgura hacia el conector de la placa", fill=INK, font=font(22))
    img.save(OUT / "assembly_top.png")


def collage():
    names = ["night_turtle", "hello_window", "flower_field", "dream_clouds"]
    m = Image.new("RGB", (480 + 30, 270 + 30), CREAM)
    for i, n in enumerate(names):
        im = Image.open(ROOT / "assets" / "images" / f"{n}.png")
        m.paste(im, (10 + (i % 2) * 250, 10 + (i // 2) * 145))
    m.save(OUT / "example_gallery.png")


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    wiring()
    stack()
    top_view()
    collage()
    print("✓ Diagramas en docs/img/")
