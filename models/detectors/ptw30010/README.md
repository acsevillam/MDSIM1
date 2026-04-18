# PTW Farmer Chamber 30010

Geometría GDML axial del detector `PTW 30010` construida a partir de:

- Catálogo de RPD, sección E, figura del `Farmer Chamber 30010`, página 56:
  `https://www.rpdinc.com/index.php?controller=attachment&id_attachment=804`
- Ficha de producto PTW 30010 con materiales y medidas:
  `https://www.ptw-usa.com/en/products/farmer-ionization-chambers-30010`
- Manual PTW, figura 4 del `Type 30010`, reproducida en ManualsLib página 24:
  `https://www.manualslib.com/manual/1312914/Ptw-30010.html?page=24#manual`

## Convención geométrica

- Eje del detector sobre `z`.
- Origen en el centro efectivo de la región sensible.
- `+z` apunta hacia el cuerpo y el cable.
- El cable flexible queda fuera del modelo por falta de relevancia mecánica para el cuerpo rígido y por la regla de la skill.

## Dimensiones tomadas directamente

- Perfil exterior rígido del cuerpo: `43.6 + 9.5 + 53.6 + 25.9 = 132.6 mm`.
- Diámetros exteriores principales del alzado: `12.6 mm`, `M11x1`, `8.59 mm`, `6.95 mm`.
- Volumen sensible efectivo: radio `3.05 mm`, longitud `21.2 mm`.
- Electrodo central: diámetro `1.15 mm`.
- Detalle frontal de la cámara: `23.6 mm` de longitud exterior y `6.95 mm` de diámetro exterior.
- Detalle longitudinal del capuchón: `93.9 mm`, `89.25 mm`, `4.65 mm` y diámetro exterior `16.4 mm`.
- Capuchón PMMA: diámetro exterior `16.4 mm`, cavidad interior `7.3 mm`, longitud total `93.9 mm`, profundidad útil `89.25 mm` y cierre frontal `4.65 mm`.

## Inferencias usadas en el GDML

- El origen geométrico se fijó en el centro efectivo de la región sensible, siguiendo la nota PTW que ubica ese punto a `13 mm` de la punta del detector.
- El detalle axial de punta se reinterpretó con el siguiente datum común en el extremo posterior de la cámara:
  `23.0 mm` para la región sensible, `21.2 mm` para el electrodo activo y `23.6 mm` hasta la punta.
- Al contrastar ese detalle con el corte esquemático publicado para la PTW30010, se reinterpretó `21.2 mm` como longitud efectiva del volumen sensible anular.
- La cámara física interior se modeló con tres tramos:
  - volumen sensible cilíndrico de `21.2 mm` centrado en el origen;
  - tramo frontal de aire muerto cilíndrico de `1.2 mm`;
  - punta cónica frontal de `1.2 mm`.
- La pared de grafito y la envolvente interna de PMMA siguen esa misma punta, en línea con el esquema coloreado del artículo.
- El electrodo central se separó jerárquicamente del volumen sensible para poder avanzar hacia la punta sin quedar artificialmente recortado por el gas sensible.
- El electrodo activo se alargó a `23.0 mm` y se desplazó `0.3 mm` hacia la punta, dejando un gap frontal aproximado de `0.6 mm` hasta el vértice.
- La sección `M11x1` se simplificó como un cilindro liso de diámetro mayor `11 mm`; no se modeló el filete de la rosca.
- El material del cuerpo rígido externo es un placeholder `PMMA-like` porque el plano dimensional no especifica el material completo del mango y la zona roscada.
- El vástago aislante trasero ya no se trata como aire: se corrigió a `PCTFE` mediante una definición material explícita en GDML.
- El detalle inferior izquierdo del plano se interpretó como la región frontal de la cámara y el electrodo, no como un capuchón accesorio.
- El cuerpo exterior rígido se reescribió como una unión explícita de cilindros coaxiales.
  Se evitó el `polycone` porque en la práctica estaba representando el detector como una secuencia de tramos cónicos cuando el plano muestra hombros cilíndricos.
- La punta frontal se hizo más pronunciada con un cono axial de `1.2 mm`.
  Esto redistribuye la diferencia entre la cota total `23.6 mm` y la longitud útil frontal, en mejor acuerdo con el esquema axial coloreado.
- El capuchón se rehízo como la zona achurada del corte.
  La punta exterior se corrigió como una semiesfera de radio `8.2 mm`, solidaria al cuerpo `Ø16.4`.
  La cota `4.65 mm` se reinterpretó como espesor axial frontal hasta la punta, no como diámetro de una nariz reducida.
- La cavidad del capuchón ya no usa todo el detector como cutter.
  Se limitó al tramo frontal realmente cubierto por el accesorio: punta, cuerpo `Ø6.95`, vástago `Ø8.59` y zona `M11`.
- El capuchón se monta coaxial con el detector en el `world`, cubriendo la punta y llegando hasta el final del tramo roscado.
  La profundidad útil `89.25 mm` frente a los `89.0 mm` del perfil cubierto deja una holgura axial residual de `0.25 mm`.
- Los demás chaflanes visibles pero no acotados no se inventaron.
  Quedan pendientes de incorporar solo si aparece una cota axial o radial clara para cada uno.

## Fuera de alcance

- Cable flexible externo.
- Rosca helicoidal real.
- Holgura radial real del capuchón respecto al detector, no indicada explícitamente en el plano.

## Optimizacion GDML

- Se analizó el archivo con `gdml-optimizer/scripts/analyze_gdml.py`.
- El análisis no detectó riesgos de transforms casi degenerados, inconsistencias booleanas ni árboles booleanos para aplanar.
  Solo reportó la rotación identidad como normalización numérica trivial.
- Se aplicó una limpieza conservadora:
  - placements normalizados hacia `define` + `positionref`;
  - eliminación del estilo mixto entre posiciones implícitas e inline;
  - normalización menor de literales numéricos sin cambio geométrico.
  - sustitución del cuerpo exterior `polycone` por `multiUnion` de cilindros coaxiales para evitar falsos tramos cónicos.
  - incorporación de una punta cónica corta donde el plano sí sugiere terminación en punta.

## Opciones de mejora identificadas

- Sustituir `PTW30010RigidBodyPlaceholder` por materiales reales por tramo cuando haya evidencia del mango rígido, la zona roscada y la transición mecánica.
- Modelar la rosca `M11x1` real si el caso de uso requiere interacción mecánica detallada; para dosimetría general la aproximación cilíndrica suele ser suficiente.
- Introducir una holgura radial explícita en la cavidad del capuchón si se necesita evitar contacto exacto entre superficies coincidentes.
- Si más adelante se necesita una exportación más “CAD-like”, puede derivarse una variante con cavidades explícitas por booleanos; no se aplicó aquí porque la versión actual es más simple y estable para Geant4.
