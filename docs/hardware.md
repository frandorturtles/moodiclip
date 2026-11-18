# Hardware

## Placa: LilyGO T-Display (ESP32)

Modelo confirmado por la persona que construye el proyecto: **LilyGO T-Display (ESP32), pantalla de 1.14" 135 x 240**.
Datos tomados de la documentación oficial de LilyGO (wiki.lilygo.cc y el repositorio
`Xinyuan-LilyGO/TTGO-T-Display`):

| Característica | Valor |
|---|---|
| Microcontrolador | ESP32-D0WDQ6, doble núcleo LX6 (hasta 240 MHz) |
| Flash | 4 MB (existe variante de 16 MB) |
| Pantalla | IPS TFT 1.14", controlador **ST7789V**, 240 x 135 px, interfaz SPI de 4 hilos |
| Batería | Conector **JST 1.25 mm** de 2 pines, soporta LiPo de 1 celda; circuito de carga integrado |
| USB | USB-C (programación y alimentación/carga) |
| Tensión de trabajo | 2.7 V – 4.2 V |
| Botones | 2 (GPIO0 y GPIO35) — **no se usan** en este proyecto |
| Tamaño de la placa | ≈ 51.4 x 25.2 mm (dato de una fuente de terceros: mídela con regla) |

### Pines de la pantalla (verificados)

| Señal ST7789V | GPIO ESP32 |
|---|---|
| MOSI | 19 |
| SCLK | 18 |
| CS | 5 |
| DC | 16 |
| RST | 23 |
| Backlight (BL) | 4 |
| MISO | no se usa |

Estos pines ya vienen conectados en la placa: no se suelda ni se cablea nada. Están declarados en
`platformio.ini` (`build_flags`) para la librería TFT_eSPI.

> Si en tu placa los pines o la pantalla fueran distintos (otra revisión o la T-Display **S3**, que es otra placa),
> este firmware **no** es compatible tal cual.

## Librerías y herramientas (versiones fijadas)

| Pieza | Versión | Para qué |
|---|---|---|
| `espressif32` (plataforma PlatformIO) | 6.9.0 | Núcleo Arduino-ESP32 2.0.x (usa `ledcSetup`/`ledcAttachPin` para el brillo) |
| `bodmer/TFT_eSPI` | 2.5.43 | Dibujo en el ST7789V |
| Pillow (solo PC) | ≥ 10 | `tools/image_converter.py` |

Con PlatformIO todo se descarga automáticamente. El proyecto también podría adaptarse a Arduino IDE
(placa "ESP32 Dev Module", librería TFT_eSPI y los mismos `#define` de pantalla), pero la guía oficial y
las pruebas de configuración de este repositorio son para PlatformIO.

## Batería

Componente previsto: **LiPo 3.7 V, 400–450 mAh**.

Compatibilidad eléctrica con la placa:

- **Tensión:** una LiPo va de ≈ 3.0 V (descargada) a 4.2 V (llena), dentro del rango 2.7–4.2 V que indica LilyGO. ✅
- **Cargador:** la placa trae circuito de carga para LiPo de 1 celda: se carga conectando el USB-C. ✅
- **Capacidad:** 400–450 mAh es una capacidad pequeña y habitual para esta placa. ✅

Lo que **debes comprobar tú, físicamente** (no se puede verificar desde aquí):

1. **Conector y polaridad.** El conector debe ser JST 1.25 mm de 2 pines. **Muchas baterías de otras marcas
   traen el rojo y el negro invertidos respecto a la placa.** Mira el `+` / `-` serigrafiado junto al conector
   de la placa y compáralo con el cable *antes* de conectar. Una polaridad invertida puede dañar la placa y la batería.
2. **Circuito de protección (PCM).** Usa una batería con protección contra sobrecarga, sobredescarga y cortocircuito
   (suele indicarse en la etiqueta o venir como una placa pequeña soldada a los cables).
3. **Corriente de carga.** La corriente máxima del cargador de la placa está en el esquemático oficial
   (`schematic/` del repo TTGO-T-Display). Como regla general, una LiPo no debe cargarse a más de 1C
   (≈ 400–450 mA para esta batería). Si el esquemático indica más, no uses esa batería.
4. **Estado físico.** Sin hinchazón, golpes, cables pelados ni envoltura rota.

## Consumo y autonomía

El firmware reduce el consumo así: CPU a 80 MHz, Wi-Fi y Bluetooth nunca se inician, 20 fps, brillo por PWM
configurable. **No se midió el consumo real** (no hay hardware aquí), así que no se promete una autonomía concreta:
mídela con un USB-meter o un multímetro en serie y ajusta `BACKLIGHT_LEVEL` y `FRAME_MS` en `include/config.h`.

## Peso

La placa y una LiPo de 400–450 mAh pesan cada una pocas decenas de gramos como máximo. Pesa tu conjunto con una
báscula de cocina antes de usarlo en el pelo; si es muy pesado para tu clip, usa un clip más grande y firme.
