# Guía de montaje

Materiales: LilyGO T-Display, LiPo 3.7 V 400–450 mAh con circuito de protección, **cinta de espuma doble cara**,
un hair clip convencional. Recomendado además (muy barato): **cinta Kapton o cinta aislante fina** y un
trocito de cartulina gruesa o plástico fino rígido.

> ⚠️ **Seguridad con LiPo:** no la dobles, perfores, aprietes ni expongas al calor. No cargues el accesorio puesto
> en el pelo ni sin supervisión, y no lo dejes cargando mientras duermes. Si la batería se hincha, huele raro o se
> calienta, deja de usarla y deséchala en un punto de reciclaje de baterías.

## 1. Elige el clip

- Superficie **plana y rígida** de al menos **≈ 6 x 3 cm** para la configuración apilada (opción A);
  ≈ 9 cm de largo para la opción B.
- Mejor un clip de pinza grande o un *barrette* de metal/plástico firme. Evita clips muy delgados o flexibles
  (la placa se despegaría o se doblaría).
- Comprueba dónde está el muelle o las partes metálicas del clip: ahí **no** va la batería.

## 2. Prueba todo antes de pegar

1. Sube el firmware (ver README) y deja la placa funcionando por USB.
2. Conecta la batería **verificando la polaridad** (ver `docs/hardware.md`) y desconecta el USB: debe seguir encendida.
3. Observa unos minutos que nada se calienta. Si algo se calienta, desconecta y revisa.

## 3. Opción A (recomendada): pila en capas

Ver `docs/img/assembly_stack.png` y `docs/img/assembly_top.png`.

De abajo hacia arriba:

1. **Clip.** Límpialo con alcohol isopropílico y déjalo secar para que la espuma pegue bien.
2. **Aislante.** Pon una capa de Kapton/cinta aislante sobre la zona del clip que quedará bajo la batería y la placa:
   así ningún metal del clip toca contactos de la placa ni la batería.
3. **Espuma capa 1.** Pega una tira de espuma doble cara sobre el aislante.
4. **Protección rígida (opcional pero recomendada).** Un trozo fino de cartulina gruesa o plástico entre el clip y la
   batería la protege de aristas o del muelle.
5. **Batería en un "nido" de espuma.** Rodea la batería con un **marco de espuma de su mismo grosor** (sin cubrir la
   batería): la placa se apoyará en el marco, no sobre la batería. La batería solo se sujeta con un par de puntos
   suaves de espuma por sus bordes: **nunca la pegues apretada ni la dobles**.
6. **Cable de batería.** Déjalo con un poco de holgura, sin pliegues cerrados ni tensión, saliendo hacia el borde donde irá
   el conector de la placa.
7. **Espuma separadora.** Sobre el marco (no sobre la batería) pon otra capa de espuma para la placa.
8. **Placa T-Display.** Conecta el conector JST de la batería (polaridad verificada) y baja la placa sobre el marco:
   pantalla hacia fuera, **sin apretar** donde está la batería.
9. Si tu placa trae **pines soldados**, comprueba hacia dónde sobresalen; cúbrelos con Kapton o elige un grosor de
   espuma que los deje sin tocar nada.

## 4. Opción B: lado a lado (más delgada)

Placa y batería una junto a la otra sobre un clip largo (≈ 9 cm o más). Mismas reglas: aislante bajo todo,
batería sin presión y fuera del muelle del clip, cable con holgura.

## 5. Acceso al puerto de carga

- Orienta la placa para que el **USB-C quede en un borde libre** del clip, con ≈ 5 mm sin espuma ni cinta alrededor.
- Comprueba que tu cable USB-C entra sin forzar. Si el conector queda tapado, gira la placa 180° y cambia
  `SCREEN_ROTATION` entre `1` y `3` en `include/config.h` para que la imagen se vea derecha.

## 6. Revisión final

- [ ] La batería no está doblada, perforada ni bajo presión.
- [ ] Ningún cable está pellizcado entre capas.
- [ ] Todo metal del clip está aislado de la placa y la batería.
- [ ] El puerto USB-C está accesible.
- [ ] La polaridad se verificó antes de conectar.
- [ ] El conjunto es ligero y firme (agítalo suavemente: nada se mueve).
- [ ] Se carga fuera del pelo y con supervisión.

## 7. Cómo sacarlo para cargar o reparar

Usa una tira de hilo dental o un palillo plano para cortar la espuma desde el borde, despacio. **Nunca hagas palanca
bajo la batería.**
