# PTW Semiflex 3D Chamber 31021

Geometría GDML axial del detector `PTW 31021` construida a partir de:

- Catálogo de RPD, sección E, figura del `Semiflex 3D Chamber 31021`, página 56:
  `https://www.rpdinc.com/index.php?controller=attachment&id_attachment=804`
- Ficha oficial PTW 31021 con materiales, referencia geométrica y build-up cap:
  `https://www.ptw-usa.com/en/products/semiflex-3d-ion-chamber-31021/`
- Folleto oficial PTW 31021:
  `https://www.ptw-usa.com/en/products/semiflex-3d-ion-chamber-31021?cHash=bb242fce0f588fb25aed57671ed68d7c&downloadfile=1137&type=3451`
- Artículo de caracterización geométrica y Monte Carlo del 31021:
  `https://doi.org/10.1016/j.zemedi.2018.05.001`

## Convención geométrica

- Eje del detector sobre `z`.
- Origen en el punto de referencia del fabricante.
- `+z` apunta hacia el vástago rígido y el cable.
- El tip del detector sin capuchón queda en `z = -3.45 mm`.
- El cable flexible de `1300 mm` queda fuera del modelo por la regla de la skill.

## Dimensiones tomadas directamente

- Longitud rígida detector-cuerpo desde el fin del tramo flexible hasta la punta: `48.25 mm`.
- Diámetro del vástago rígido: `7.0 mm`.
- Detalle inferior derecho:
  - longitud del cuerpo delantero del detector: `30.375 mm`;
  - longitud total con capuchón: `33.375 mm`;
  - diámetro exterior del capuchón: `12.5 mm`.
- Ficha PTW / folleto:
  - punto de referencia a `3.45 mm` desde la punta;
  - volumen sensible: radio `2.4 mm`, longitud `4.8 mm`;
  - electrodo central de Al: diámetro `0.8 mm`;
  - pared sensible: `0.09 mm` de grafito + `0.57 mm` de PMMA;
  - build-up cap en PMMA con espesor nominal `3 mm`.
- Artículo Monte Carlo:
  - longitud axial del electrodo central activo: `2.8 mm`;
  - la punta interna es esférica y el detector se trata como cámara 3D con frente redondeado.

## Inferencias usadas en el GDML

- El volumen sensible se modeló como un `thimble` de revolución:
  - un cilindro de `2.4 mm` de longitud;
  - una semiesfera frontal de radio `2.4 mm`.
  Esta reconstrucción es la que cierra consistentemente la cota oficial `4.8 mm` con el volumen nominal `0.07 cm3`.
- La pared de grafito y la envolvente de PMMA siguen la misma topología `cilindro + semiesfera`, manteniendo el plano trasero común de la cavidad.
- El cuerpo delantero que entra en el capuchón se fijó con diámetro exterior `6.5 mm`.
  Esto no aparece de forma inequívoca en la figura, pero se deduce al imponer simultáneamente:
  - diámetro exterior del capuchón `12.5 mm`;
  - espesor nominal PTW del build-up cap `3 mm`.
- La cota ambigua `Ø7.2` del detalle inferior derecho se interpretó como diámetro del hombro/vástago rígido, no del morro cubierto por el capuchón.
- El capuchón se entrega montado sobre el detector.
  Se modela como volumen PMMA externo que contiene al cuerpo delantero, evitando booleans coplanares innecesarios.
- El cuerpo rígido y el vástago se dejan con material placeholder `PMMA-like`.
  El plano dimensional consultado no especifica materiales reales por tramo.
- No se modelaron roscas, alivios pequeños ni el detalle negro de transición al cable porque no están acotados de forma suficiente.

## Fuera de alcance

- Cable flexible externo.
- Microdetalles del feedthrough trasero.
- Posibles escalones finos en la nariz frontal que el scan no resuelve de forma confiable.

## Optimización GDML

- Se validó la estructura con `gdml-creator/scripts/check_gdml.py`.
- Se analizó el archivo con `gdml-optimizer/scripts/analyze_gdml.py`.
- La versión final ya quedó en una forma estable y relativamente plana:
  - sin booleans profundos;
  - con capas anidadas en vez de sustracciones innecesarias;
  - con el capuchón modelado como envolvente/madre del cuerpo delantero.

## Opciones de mejora identificadas

- Sustituir `PTW31021RigidBodyPlaceholder` por materiales reales por tramo cuando aparezca documentación de la caña rígida y del feedthrough.
- Refinar la nariz exterior si se consigue una figura de mayor resolución que aclare si existe un pequeño escalón entre `6.5 mm`, `6.9 mm` o `7.2 mm`.
- Añadir una variante sin capuchón o una variante con el capuchón separado si el caso de uso requiere comparar configuraciones de medida en agua y en aire.
