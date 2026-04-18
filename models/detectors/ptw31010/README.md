# PTW Semiflex Chamber 31010

Geometría GDML axial del detector `PTW 31010` construida a partir de:

- Catálogo de RPD, sección E, figura del `Semiflex Chamber 31010`, página 56:
  `https://www.rpdinc.com/index.php?controller=attachment&id_attachment=804`
- Ficha oficial PTW 31010:
  `https://www.ptwdosimetry.com/en/products/semiflex-ionization-chamber-31010/`
- Página de producto RPD 31010, que reitera medidas y notas mecánicas:
  `https://www.rpdinc.com/ptw-31010-0125cc-semiflex-chamber-972.html`
- Fuente secundaria con referencia al plano PTW, que reporta `6.9 mm` y `6.5 mm` para la cabeza sensible:
  `https://es.scribd.com/document/422798138/SBRT-pdf`

## Convención geométrica

- Eje del detector sobre `z`.
- Origen en el punto de referencia del fabricante.
- `+z` apunta hacia el cuerpo rígido y el cable.
- La punta del detector sin capuchón queda en `z = -4.5 mm`.
- El cable flexible queda fuera del modelo por la regla de la skill.

## Dimensiones tomadas directamente

- Ficha PTW:
  - punto de referencia a `4.5 mm` desde la punta;
  - volumen sensible nominal `0.125 cm3`;
  - radio interno sensible `2.75 mm`;
  - longitud interna sensible `6.5 mm`;
  - pared sensible `0.55 mm` de PMMA + `0.15 mm` de grafito;
  - electrodo central de Al, diámetro `1.1 mm`;
  - build-up cap de PMMA, espesor nominal `3 mm`.
- Página RPD del producto:
  - cámara con diámetro interno `5.5 mm`;
  - `31 mm` de rigid stem for mounting.
- Fuente secundaria basada en el plano PTW:
  - cabeza sensible exterior de aproximadamente `6.9 mm` de diámetro;
  - axial sensible de `6.5 mm`.

## Inferencias usadas en el GDML

- La cavidad sensible se modeló como un `thimble` de revolución:
  - un cilindro de `3.75 mm`;
  - una semiesfera frontal de radio `2.75 mm`.
  Esta reconstrucción conserva la cota axial `6.5 mm` y aproxima mejor el carácter “approximately spherical” descrito por PTW que un cilindro puro.
- La pared de grafito y la protección de PMMA siguen la misma topología `cilindro + semiesfera`, con el mismo plano trasero de cierre de la cavidad.
- La envolvente exterior rígida se cerró con:
  - diámetro exterior `7.0 mm`;
  - longitud rígida total `31.0 mm`.
  Esto combina la evidencia accesible de `31 mm` de vástago rígido con la referencia secundaria a una cabeza de `6.9 mm` de diámetro.
- El capuchón se modeló montado y coaxial, como una envolvente PMMA de espesor radial/frontal `3 mm` alrededor de la cabeza rígida.
  De ello resulta un diámetro exterior aproximado de `12.9 mm` y una longitud total aproximada de `33.95 mm`.
- No se introdujeron escalones, chaflanes ni alivios menores en la nariz o en la transición al cable porque no quedaron legibles con suficiente respaldo dimensional en la evidencia disponible.
- La longitud axial activa del electrodo central no aparece acotada en las fuentes accesibles.
  Se fijó en `6.0 mm`, dejando un gap frontal aproximado de `0.5 mm` dentro del gas sensible, y se reporta como supuesto.
- El cuerpo rígido externo se deja con material placeholder `PMMA-like`.
  Las fuentes consultadas especifican bien la pared sensible y el capuchón, pero no los materiales del vástago completo.

## Fuera de alcance

- Cable flexible externo.
- Roscas o microdetalles del feedthrough trasero.
- Holguras mecánicas reales entre detector y capuchón, no acotadas explícitamente.

## Optimización GDML

- Se validó la estructura con `gdml-creator/scripts/check_gdml.py`.
- Se analizó el archivo con `gdml-optimizer/scripts/analyze_gdml.py`.
- La versión final se dejó deliberadamente simple:
  - sin booleans de sustracción;
  - con capas anidadas por materiales;
  - con el capuchón como madre geométrica del cuerpo rígido para evitar cutters coplanares innecesarios.

## Opciones de mejora identificadas

- Reemplazar `PTW31010RigidBodyPlaceholder` por materiales reales por tramo si aparece una sección oficial legible del mango rígido.
- Refinar la nariz exterior y la longitud exacta del capuchón si se consigue una copia legible de la página 56 del catálogo o un dibujo de fabricación equivalente.
- Sustituir el electrodo activo inferido de `6.0 mm` por la longitud real en cuanto aparezca una cota axial interna fiable.
- Añadir una variante sin capuchón o con capuchón separado si el caso de uso requiere comparar medición en agua y en aire.
