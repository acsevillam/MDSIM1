# MultiDetector1

Geant4 MultiDetector Simulation v1
Copyright (c) 2024 Andrés Camilo Sevilla
acsevillam@eafit.edu.co - acsevillam@gmail.com
All Rights Reserved.

`MultiDetector1` (MDSIM1) es una simulación Monte Carlo basada en Geant4 orientada a la evaluación y caracterización de múltiples detectores de radiación usados en radioterapia. MDSIM1 permite construir distintas configuraciones geométricas (linac, fantoma de agua, detectores), definir fuentes a partir de phase space o GPS, y calcular distribuciones de dosis mediante scorers de Geant4.

El flujo de trabajo del proyecto está enfocado a:

- Ejecutar casos de referencia con macros fijas (`input/*.in`);
- Correr barridos parametrizados con plantillas (`input/<dominio>/templates/*.intmp`) y scripts (`jobs/<dominio>/*.sh`);
- Guardar resultados para análisis posterior en `analysis/`.

## Jobs

La carpeta `jobs/` contiene lanzadores reproducibles para campañas batch.

En dosimetría, además del barrido de validación, existe:

- [launch_calibration_dmax_average.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/dosimetry/launch_calibration_dmax_average.sh): ejecuta 5 réplicas de `input/dosimetry/Calibration.in` con `-b on -v off -n 300000000` y calcula promedios para `Dose atDepth1.4cm` (`dmax` en este caso) y `Dose atDepth10cm`.

Este job escribe:

- una carpeta por réplica con `output.log`, `macro.in` y los archivos de `analysis/`
- `dmax_summary.csv` con una fila por réplica
- `dmax_average.txt` con media, desviación estándar entre réplicas, `err = stddev / sqrt(n)` y error relativo para `Dose atDepth1.4cm` y `Dose atDepth10cm`

Para el detector `cube`, existen jobs de calibración de carga para cuatro escenarios:

- [launch_calibration_water.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_calibration_water.sh): cubo de `G4_WATER`, lado `5.0 mm`, sin envolvente.
- [launch_calibration_air.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_calibration_air.sh): cubo de `G4_AIR`, lado `5.0 mm`, sin envolvente.
- [launch_calibration_water_pmma_2p5mm.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_calibration_water_pmma_2p5mm.sh): cubo de `G4_WATER`, lado `5.0 mm`, envolvente `G4_PLEXIGLASS` con `2.5 mm`.
- [launch_calibration_air_pmma_2p5mm.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_calibration_air_pmma_2p5mm.sh): cubo de `G4_AIR`, lado `5.0 mm`, envolvente `G4_PLEXIGLASS` con `2.5 mm`.

Estos jobs escriben:

- una carpeta por réplica con `output.log`, `macro.in` y los archivos de `analysis/`
- `calculated_total_collected_charge_summary.csv` con una fila por réplica
- `calculated_total_collected_charge_average.txt` con media, desviación estándar entre réplicas, `err = stddev / sqrt(n)` y error relativo para `(7) Scaled collected charge`, incluyendo material y configuración de envolvente

Tambien existen barridos `pdd_interface` para `cube`, usando [CubeCalibration.intmp](/Users/acsevillam/workspace/Geant4/MDSIM1/input/detectors/cube/templates/CubeCalibration.intmp), `setEOFPolicy synthetic` y `200000000` eventos por punto:

- [launch_pdd_interface_water.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_pdd_interface_water.sh): barrido para `G4_WATER`, sin envolvente
- [launch_pdd_interface_air.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_pdd_interface_air.sh): barrido para `G4_AIR`, sin envolvente
- [launch_pdd_interface_water_pmma_2p5mm.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_pdd_interface_water_pmma_2p5mm.sh): barrido para `G4_WATER` con envolvente `G4_PLEXIGLASS` de `2.5 mm`
- [launch_pdd_interface_air_pmma_2p5mm.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/detectors/cube/launch_pdd_interface_air_pmma_2p5mm.sh): barrido para `G4_AIR` con envolvente `G4_PLEXIGLASS` de `2.5 mm`

Estos jobs:

- barren `cube/translateTo 0 0 z cm` desde `11.0 cm` hasta `8.5 cm`
- usan paso de `0.10 cm` en todo el barrido
- ejecutan `5` réplicas por punto
- extraen `(8) Estimated absorbed dose in water (...UM)` del bloque de resultados del detector

La salida incluye:

- `pdd_by_replica.csv`: una fila por réplica y posición
- `pdd_summary.csv`: resumen por posición con media, desviación estándar entre réplicas, `err = stddev / sqrt(n)` y error relativo
- `pdd_summary.txt`: resumen legible del barrido bajo el mismo estándar textual de los demás jobs de `cube`

Los detectores `cylinder` y `sphere` replican el mismo flujo operativo de `cube`:

- calibración de carga en `jobs/detectors/cylinder/` y `jobs/detectors/sphere/`
- barridos `pdd_interface` con las mismas cuatro variantes material/envolvente
- salidas `calculated_total_collected_charge_summary.csv`, `calculated_total_collected_charge_average.txt`, `pdd_by_replica.csv`, `pdd_summary.csv` y `pdd_summary.txt`

Configuraciones nominales embebidas en los launchers:

- `cylinder`: radio `2.5 mm`, altura `5.0 mm`
- `sphere`: radio `2.5 mm`

## Manejo de errores

En los resúmenes de corrida y por detector, MDSIM1 distingue entre incertidumbre estadística Monte Carlo e incertidumbres paramétricas de escalado y calibración.

### Etiquetas

MDSIM1 usa el siguiente patrón de etiquetas para incertidumbres:

- `mc_err`: incertidumbre estadística Monte Carlo del estimador, obtenida como `rms / sqrt(N)` sobre la magnitud por evento.
- `mu_err`: incertidumbre propagada del factor de escala por monitor units, a partir de `fScaleFactorMUError`.
- `det_err`: incertidumbre propia del detector o de su calibración. Actualmente se usa para `Estimated absorbed dose in water`, a partir del error del factor de calibración aplicado fuera del readout en `cGy/nC`.
- `total_err`: combinación en cuadratura de las contribuciones independientes que aplican a la magnitud reportada.

### Regla base

Para una magnitud por evento `X_per_event`, MDSIM1 calcula:

```text
mean = mean(X_per_event)
rms = G4StatDouble::rms(N, N)
mc_err = rms / sqrt(N)
```

donde `N` es el número total de eventos del run.

Si luego la magnitud se escala como:

```text
X_scaled = X_per_event * fScaleFactorMU * simulatedMU
```

entonces:

```text
mc_err_scaled = mc_err(X_per_event) * fScaleFactorMU * simulatedMU
mu_err = |X_per_event| * simulatedMU * fScaleFactorMUError
```

Si además existe una incertidumbre relativa del detector `relative_detector_error`, entonces:

```text
det_err = |X_per_event| * relative_detector_error * fScaleFactorMU * simulatedMU
```

Cuando aplican varias contribuciones independientes, `total_err` se calcula en cuadratura.

### Magnitudes reportadas

Para la magnitud:

- `(6) Calculated total dose in detector sensitive volume (...UM)`

la salida combina:

- `mc_err`
- `mu_err`
- `total_err`

con:

```text
D_total = D_per_event * fScaleFactorMU * simulatedMU
mc_err_scaled = mc_err(D_per_event) * fScaleFactorMU * simulatedMU
mu_err = |D_per_event| * simulatedMU * fScaleFactorMUError
total_err = sqrt(mc_err_scaled^2 + mu_err^2)
```

Para la magnitud:

- `(7) Scaled collected charge (...UM)`

la salida combina:

- `mc_err`
- `mu_err`
- `total_err`

con:

```text
Q_total = Q_per_event * fScaleFactorMU * simulatedMU
mc_err_scaled = mc_err(Q_per_event) * fScaleFactorMU * simulatedMU
mu_err = |Q_per_event| * simulatedMU * fScaleFactorMUError
total_err = sqrt(mc_err_scaled^2 + mu_err^2)
```

Para la magnitud:

- `(8) Estimated absorbed dose in water (...UM)`

la salida combina:

- `mc_err`
- `mu_err`
- `det_err`
- `total_err`

con:

```text
D_water = D_water,per_event * fScaleFactorMU * simulatedMU
mc_err_scaled = mc_err(D_water,per_event) * fScaleFactorMU * simulatedMU
mu_err = |D_water,per_event| * simulatedMU * fScaleFactorMUError
det_err = |D_water,per_event| * relative_calibration_error * fScaleFactorMU * simulatedMU
total_err = sqrt(mc_err_scaled^2 + mu_err^2 + det_err^2)
```

En el bloque de resultados por detector, `det_err` se obtiene a partir de la incertidumbre relativa de calibración de ese detector.

Cuando un módulo tiene varias copias geométricas activas, el bloque de resultados se emite por copia usando el formato `---------------- <Detector> Results: <detector>[copyNo] ----------------`, por ejemplo `cube[0]`, `cube[1]` o `BB7[3]`.

### Lectura práctica

- `mc_err` describe el ruido estadístico de la simulación Monte Carlo.
- `mu_err` describe la incertidumbre del factor que convierte resultados por evento a resultados por `MU`.
- `det_err` describe la incertidumbre del modelo/calibración del detector.
- `total_err` no sustituye a las componentes individuales; las resume cuando se requiere una incertidumbre total propagada.

`mu_err` y `det_err` no deben reinterpretarse como más historia Monte Carlo. Se reportan por separado y luego se combinan en `total_err` para evitar mezclar incertidumbre estadística con incertidumbre de calibración o normalización.

### Validaciones y fallos explícitos

Para evitar resultados silenciosamente inconsistentes, MDSIM1 ahora aborta la corrida o la inicialización cuando detecta condiciones inválidas en el pipeline de incertidumbres y lectura:

- `/MultiDetector1/run/SetMU` exige `MU > 0`.
- `/MultiDetector1/run/SetScaleFactorMU` exige un factor estrictamente positivo.
- `/MultiDetector1/run/SetScaleFactorMUError` exige una incertidumbre no negativa.
- Si no se define un override explícito para `fScaleFactorMUError`, el valor por defecto se mantiene como `1 %` del `fScaleFactorMU` vigente. Si luego se cambia `fScaleFactorMU` sin override explícito, el error por defecto se actualiza automáticamente para seguir siendo `1 %`.
- Si un detector activo no encuentra su volumen sensible, digitizer, colección de dígitos o ntuple de análisis, la corrida aborta con `FatalException` en lugar de devolver `0` silenciosamente.
- La tabla [CubeCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/cube/geometry/CubeCalibrationTable.dat) rechaza entradas con lado no positivo, factor no positivo, incertidumbre negativa, unidades inválidas o entradas duplicadas para el mismo material y lado.
- La tabla [CylinderCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/cylinder/geometry/CylinderCalibrationTable.dat) rechaza entradas con radio/altura no positivos, factor no positivo, incertidumbre negativa, unidades inválidas o entradas duplicadas.
- La tabla [SphereCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/sphere/geometry/SphereCalibrationTable.dat) rechaza entradas con radio no positivo, factor no positivo, incertidumbre negativa, unidades inválidas o entradas duplicadas.
- La tabla de calibración del `cube` se parsea una sola vez por proceso con inicialización thread-safe y luego queda de solo lectura.
- Las tablas locales de `cylinder` y `sphere` también se parsean una sola vez por proceso con inicialización thread-safe y luego quedan de solo lectura.
- La ejecución de macros desde línea de comandos aborta si `/control/execute` devuelve error, en vez de continuar con una configuración parcial.

## Arquitectura de detectores

El proyecto quedó reorganizado para soportar múltiples detectores de radiación como módulos autocontenidos.

- `include/geometry/base/` y `src/geometry/base/`: contratos comunes, registry y utilidades base.
- `include/geometry/detectors/<detector>/` y `src/geometry/detectors/<detector>/`: implementación por detector.
- `include/geometry/detectors/basic/core/` y `src/geometry/detectors/basic/core/`: núcleo compartido de los detectores dosimétricos básicos.
- `include/geometry/detectors/basic/cube|cylinder|sphere/` y `src/geometry/detectors/basic/cube|cylinder|sphere/`: adapters públicos y geometrías específicas para `cube`, `cylinder` y `sphere`.
- `include/geometry/beamline/` y `src/geometry/beamline/`: acelerador y componentes del haz.
- `include/geometry/phantoms/` y `src/geometry/phantoms/`: fantomas y geometrías auxiliares.

Cada detector vive como un módulo con:

- geometría
- messenger
- sensitive detector
- hit
- digit
- digitizer
- calibration

El ensamblado ya no está hardcodeado en `MD1DetectorConstruction` ni en `MD1EventAction`. Ahora se realiza mediante `DetectorRegistry`, que conoce los detectores disponibles y cuáles están habilitados para la corrida actual.

### Detectores disponibles

- `cube`
- `cylinder`
- `sphere`
- `scintCube`
- `model11`
- `BB7`

### Namespaces por detector

Los comandos específicos de cada detector ahora viven bajo:

- `/MultiDetector1/detectors/cube/*`
- `/MultiDetector1/detectors/cylinder/*`
- `/MultiDetector1/detectors/sphere/*`
- `/MultiDetector1/detectors/scintCube/*`
- `/MultiDetector1/detectors/model11/*`
- `/MultiDetector1/detectors/BB7/*`

`scintCube` usa un pipeline distinto al de los detectores dosimétricos de ionización:

- el digitizer reporta sólo señal física del centellador
- la calibración a dosis se aplica en una clase separada
- el fotosensor se selecciona por detector como `PMT` o `SiPM`
- el bloque de resultados incluye energía visible, fotones producidos, fotoelectrones detectados, tiempo medio de señal y dosis calibrada

`model11` expone un detector centellador cilíndrico independiente para representar el Blue Physics Model 11:

- la geometría completa se importa desde GDML
- `PMT` como fotosensor nominal por defecto
- calibración a dosis separada del readout, igual que en `scintCube`
- el material y el tamaño del volumen sensible ya no se configuran por messenger
- uno o varios volúmenes del GDML pueden marcarse como sensibles por nombre desde el messenger

Para la geometría importada de `model11`:

- el runtime no lee `.FCStd` ni `.brep` directamente
- el runtime usa `G4GDMLParser` para leer un archivo GDML completo exportado desde FreeCAD
- el GDML importado contiene tanto el ensamble pasivo como los volúmenes potencialmente sensibles
- el comando de configuración es `/MultiDetector1/detectors/model11/setImportedGeometryGDML`
- opcionalmente puedes fijar un root explícito del GDML con `/MultiDetector1/detectors/model11/setImportedGeometryRoot`
- la selección de volúmenes sensibles se hace con `/MultiDetector1/detectors/model11/addSensitiveVolume`
- si no seleccionas ningún volumen sensible, `model11` se comporta como una geometría pasiva para inspección
- el GDML integrado del detector está en `models/detectors/model11/gdml/model11.gdml`
- los fixtures GDML de CI viven bajo `cmake/tests/data/model11/`

En `cube`, `cylinder`, `sphere` y `BB7` también se separó la calibración del readout:

- el `digitizer` conserva sólo la salida física (`edep`, carga recogida y `Dose[Gy]`)
- la conversión a `EstimatedDoseToWater` se aplica en una clase de calibración separada
- cada módulo detector escribe un ntuple físico y un ntuple de calibración por separado

Además, `cube`, `cylinder` y `sphere` ya no viven como implementaciones duplicadas completas. Internamente comparten el núcleo `basic/core` para el cálculo de carga recogida, masa sensible y calibración dosimétrica, mientras conservan:

- el mismo nombre público de detector
- los mismos comandos `/MultiDetector1/detectors/<shape>/*`
- las mismas tablas de calibración específicas por forma
- el mismo formato de salida por detector

Regla práctica:

- `addGeometryTo`: solicita y activa la colocación del detector; también habilita el módulo automáticamente
- configuración estructural del detector, como material o tamaño: `PreInit`
- transformaciones sobre volúmenes ya construidos (`translate`, `rotate`): `Idle`
- `addGeometryTo` y `removeGeometry`: `PreInit` o `Idle`

En la práctica:

- usa `addGeometryTo` en `PreInit` si quieres que el detector participe plenamente en la corrida (geometría, SD, digitizer y análisis)
- usa `addGeometryTo`/`removeGeometry` en `Idle` para manipular volúmenes ya activos o preparar una reconstrucción posterior de la geometría
- `enable/disable` ya no hace falta en las macros normales
- `enable` ya no es requerido si el detector se activa mediante `addGeometryTo`

### Cómo añadir un detector nuevo

1. Crear una carpeta nueva en `geometry/detectors/<name>/`.
2. Implementar un `DetectorModule` para ese detector.
3. Añadir su geometría, messenger y pipeline de readout dentro del mismo módulo.
4. Registrar el módulo en `DetectorRegistry::RegisterDefaults()`.
5. Definir sus ntuples/histogramas en `CreateAnalysis()` y su lectura de evento en `ProcessEvent()`.

Si el detector nuevo pertenece a la familia de dosímetros geométricos simples, conviene reutilizar `geometry/detectors/basic/core/` en vez de duplicar otra vez el pipeline de `cube`, `cylinder` o `sphere`.

## Requisitos

- CMake 3.16 o superior.
- Geant4 instalado y disponible para CMake (`find_package(Geant4 REQUIRED ...)`).
- Entorno de Geant4 cargado antes de compilar/ejecutar (por ejemplo, `geant4.sh`).
- Git LFS si se van a usar los phase spaces binarios incluidos como `.IAEAphsp`.
- Un directorio de datos accesible para los phase spaces (`beam/*.IAEAphsp` y `beam/*.IAEAheader`), ya sea en el árbol fuente o mediante `MDSIM1_DATA_DIR`.

## Compilación

Desde la carpeta raíz del proyecto:

```bash
cmake -S . -B MDSIM-build
cmake --build MDSIM-build -jN
```

N corresponde con el número de núcleos disponibles en el equipo.

Esto genera el ejecutable:

- `MDSIM-build/MultiDetector1`

Durante la configuración se copian macros, archivos `.in`, scripts y recursos ligeros al directorio de compilación (`MDSIM-build/`).
Los datasets pesados (`.IAEAphsp`, `.IAEAheader` y referencias grandes) ya no se copian por defecto.

Por defecto, el ejecutable busca phase spaces en este orden:

- la ruta dada en la macro/comando, relativa al directorio actual
- el directorio definido por la variable de entorno `MDSIM1_DATA_DIR`
- el directorio configurado en CMake mediante `MDSIM1_DATA_DIR` (por defecto, la raíz del proyecto)

Si quieres fijar explícitamente un directorio externo de datos al compilar:

```bash
cmake -S . -B MDSIM-build -DMDSIM1_DATA_DIR=/ruta/a/mdsim-data
cmake --build MDSIM-build -jN
```

Si prefieres un build autocontenido que también copie los phase spaces al árbol de compilación:

```bash
cmake -S . -B MDSIM-build -DMDSIM1_COPY_RUNTIME_DATA=ON
cmake --build MDSIM-build -jN
```

## Pruebas rápidas

El proyecto incluye pruebas CTest para regresiones del flujo PHSP y del ciclo de vida de geometría en `Idle`:

- un `setEOFPolicy` inválido debe fallar en el momento del comando
- una corrida MT con `-n 0` no debe abrir readers PHSP
- agregar y remover detectores en `Idle` no debe dejar estado stale en SD, digitizers ni análisis
- el `cube` debe poder reconstruirse en `Idle` bajo MT al pasar por volumen único dentro de agua
- el `cube` debe poder reconstruirse en `Idle` bajo MT al pasar por `split at interface`
- el `cube` debe poder reconstruirse en `Idle` bajo MT al pasar por volumen completo fuera del agua

Se ejecutan así:

```bash
ctest --test-dir MDSIM-build --output-on-failure
```

Para correr solo la batería rápida de detectores, ahora puedes usar cualquiera de estos dos flujos:

```bash
ctest --test-dir MDSIM-build -L detectors_quick --output-on-failure
```

o, desde el árbol de build:

```bash
cmake --build MDSIM-build --target detector-tests
```

Si quieres separarlas por tipo, también quedan disponibles estos filtros:

```bash
ctest --test-dir MDSIM-build -L phsp_quick --output-on-failure
ctest --test-dir MDSIM-build -L geometry_quick --output-on-failure
ctest --test-dir MDSIM-build -L signal_quick --output-on-failure
```

o como targets:

```bash
cmake --build MDSIM-build --target phsp-tests
cmake --build MDSIM-build --target geometry-tests
cmake --build MDSIM-build --target signal-tests
```

Y para correr toda la batería rápida del proyecto en una sola pasada:

```bash
ctest --test-dir MDSIM-build -L quick --output-on-failure
cmake --build MDSIM-build --target quick-tests
```

También queda reservado un carril para pruebas futuras más lentas o de tipo nightly:

```bash
ctest --test-dir MDSIM-build -L nightly --output-on-failure
cmake --build MDSIM-build --target nightly-tests
```

Por ahora no hay pruebas marcadas con `nightly`, así que el target `nightly-tests`
termina de forma limpia informando que todavía no hay casos en ese grupo.

Actualmente el conjunto incluye, entre otras, estas verificaciones:

- `mdsim1_zero_events_skip_phsp_open`
- `mdsim1_idle_detector_refresh`
- `mdsim1_idle_detector_remove`
- `mdsim1_idle_detector_remove_mt`
- `mdsim1_idle_cube_split_rebuild_mt`
- `mdsim1_cube_signal_smoke`
- `mdsim1_bb7_signal_smoke`

Los dos smoke tests de señal usan GPS con electrones monoenergéticos y verifican el pipeline completo:

- geometría activa del detector
- `SensitiveDetector`
- digitizer
- bloque de resultados específico del detector

Estas pruebas no requieren phase spaces binarios reales.

Si los archivos `beam/*.IAEAphsp` aparecen como punteros de Git LFS en vez de binarios reales, la simulación abortará con un mensaje claro. Antes de ejecutar casos PHSP, descarga los objetos reales con:

```bash
git lfs pull
```

## Ejecución

Entrar al directorio de compilación:

```bash
cd MDSIM-build
```

Si los phase spaces viven fuera del árbol fuente, exporta antes:

```bash
export MDSIM1_DATA_DIR=/ruta/a/mdsim-data
```

### 1) Modo interactivo (con visualización)

```bash
./MultiDetector1
```

Este modo carga por defecto:

- macro de inicialización: `mac/init.mac`
- macro de visualización: `mac/vis.mac`

### 2) Modo batch (sin visualización)

Ejemplo con macro de entrada y número de historias:

```bash
./MultiDetector1 -m input/dosimetry/Calibration.in -v off -b on -n 1000000
```

Parámetros disponibles:

- `-m <macro>`: macro principal (por defecto `mac/init.mac`).
- `-v <on|off>`: activa/desactiva visualización. Si se usa `on`, el programa abre sesión interactiva.
- `-vm <vis_macro>`: macro de visualización (por defecto `mac/vis.mac`).
- `-b <on|off>`: activa/desactiva biasing.
- `-n <eventos>`: número de eventos para `BeamOn` en modo no interactivo. Debe ser un entero no negativo.

La interfaz oficial del binario usa únicamente flags `-m`, `-v`, `-vm`, `-b` y `-n`. No se soporta el formato implícito `./MultiDetector1 macro.mac`.

## Ejemplos útiles

Batch sin visualización y sin biasing:

```bash
./MultiDetector1 -m input/dosimetry/Validation.in -v off -b off -n 500000
```

Interactivo forzando macro de visualización:

```bash
./MultiDetector1 -v on -vm mac/vis.mac -m mac/init.mac
```

## Parametrizacion de macros (`input/*`)

La carpeta `input/` contiene dos tipos de macros:

- macros fijas (`*.in`) listas para ejecutar.
- plantillas parametrizables (`*.intmp`) con placeholders tipo `**Variable**`.

## Macros incluidos en `input/`

### Macros base (`.in`)

- `input/dosimetry/Calibration.in`: configuración de calibración con `WaterBox` y scorers puntuales en profundidad (incluye mallas a 1.4 cm y 10 cm, y región de buildup).
- `input/dosimetry/Validation.in`: caso de validación dosimétrica en fantoma de agua con perfiles laterales (`x`, `y`) y curva de profundidad (`PDD`).
- `input/dosimetry/DoseMap3D.in`: cálculo volumétrico 3D de dosis en `WaterBox` mediante una malla cúbica.
- `input/detectors/BB7/BB7.in`: configuración con detector `DualBB7`, jaws y rotaciones de gantry/colimador para estudios geométricos del detector.
- `input/detectors/BB7/BB7Calibration.in`: configuración de calibración BB7 con fuente Co-60 (GPS) y malla 2D para mapa de dosis.
- `input/detectors/cube/Cube.in`: configuración base del detector `cube` con PHSP, jaws y geometría simple dentro del `WaterBox`.
- `input/detectors/cube/CubeCalibration.in`: configuración de calibración del detector `cube` con fuente Co-60 (GPS) y malla 2D para mapa de dosis.
- `input/detectors/cylinder/Cylinder.in`: configuración base del detector `cylinder` con PHSP, jaws y geometría cilíndrica dentro del `WaterBox`.
- `input/detectors/cylinder/CylinderCalibration.in`: configuración de calibración del detector `cylinder`.
- `input/detectors/sphere/Sphere.in`: configuración base del detector `sphere` con PHSP, jaws y geometría esférica dentro del `WaterBox`.
- `input/detectors/sphere/SphereCalibration.in`: configuración de calibración del detector `sphere`.

### Plantillas parametrizables (`.intmp`)

- `input/dosimetry/templates/Calibration.intmp`: plantilla de calibración con reemplazo principal de `**PhspFileName**`.
- `input/dosimetry/templates/Validation.intmp`: plantilla de validación con reemplazo de `**PhspFileName**`.
- `input/detectors/BB7/templates/BB7.intmp`: plantilla BB7 para barridos de `**PhspFileName**`, aperturas de jaws, `**GantryAngle**`, `**CollimatorAngle**` y `**DetectorAngle**`.
- `input/detectors/BB7/templates/BB7Calibration.intmp`: plantilla de calibración BB7/Co-60 con parámetros geométricos del haz (`**BeamCentre**`, `**BeamRot1**`, `**BeamRot2**`, `**BeamDirection**`) y ángulos de sistema.
- `input/detectors/cube/templates/Cube.intmp`: plantilla del detector `cube` para barridos PHSP con `**PhspPrefix**`, jaws, material, lado del cubo y posición/rotación básicas.
- `input/detectors/cube/templates/CubeCalibration.intmp`: plantilla de calibración dosimétrica del `cube` con PHSP, jaws, ángulos del linac y scorers puntuales en profundidad.
- `input/detectors/cylinder/templates/Cylinder.intmp`: plantilla del detector `cylinder` con placeholders nativos `**CylinderRadius**`, `**CylinderHeight**`, material y posición.
- `input/detectors/cylinder/templates/CylinderCalibration.intmp`: plantilla de calibración dosimétrica del `cylinder`.
- `input/detectors/sphere/templates/Sphere.intmp`: plantilla del detector `sphere` con placeholder nativo `**SphereRadius**`, material y posición.
- `input/detectors/sphere/templates/SphereCalibration.intmp`: plantilla de calibración dosimétrica del `sphere`.

### Macros de benchmark

- `input/benchmark/set1.in`: punto de referencia para caso TrueBeam (medición puntual en profundidad).
- `input/benchmark/set2.in`: conjunto de puntos de referencia con campo rectangular y SSD específico.
- `input/benchmark/set3.in`: conjunto de referencia para campo pequeño, evaluando dosis en distintas profundidades.

## Interpretacion de `analysis/*.out`

Los archivos `analysis/<mesh>.out` que salen de los primitive scorers de Geant4 no guardan directamente la media por evento. Guardan la suma acumulada del scorer en cada voxel, ya escalada por el factor configurado en `RunAction`.

En [MD1RunAction.cc](/Users/acsevillam/workspace/Geant4/MDSIM1/src/MD1RunAction.cc), antes de escribir los `.out`, se hace:

```cpp
G4double scale_factor = fScaleFactorMU * fSimulatedMU;
scoringManager->SetFactor(scale_factor);
scoringManager->DumpAllQuantitiesToFile(meshName, "analysis/" + meshName + ".out");
```

Por eso el encabezado del archivo incluye:

```text
# multiplied factor : <scale_factor>
```

Geant4 escribe cada fila del `.out` como:

- `total(value)`: `sum_wx / unitValue * multiplied_factor`
- `total(val^2)`: `sum_wx2 / unitValue^2 * multiplied_factor^2`
- `entry`: numero de entradas no nulas del voxel

Para el scorer `dose`, `unitValue = gray`, asi que:

- `total(value)` queda en `Gy`
- `total(val^2)` queda en `Gy^2`

Si una fila del archivo contiene:

```text
iX,iY,iZ,S,Q,nEntries
```

entonces el bloque impreso en consola como:

```text
------------------ Dose atDepth1.4cm ---------------------
0    <meanDose> cGy rms = <rmsDose> cGy nEntries = <nEntries>
```

se obtiene usando `N = nofEvents` del run completo:

```text
meanDose [Gy] = S / N
meanDose [cGy] = (S / N) / 1e-2
```

```text
rms [Gy] = sqrt( N/(N-1) * ( Q/N - (S/N)^2 ) )
rms [cGy] = rms[Gy] / 1e-2
```

Esta es exactamente la misma convencion que usa `MD1RunAction` al imprimir `Dose at...`, via `G4StatDouble::mean(nofEvents)` y `G4StatDouble::rms(nofEvents, nofEvents)`.

Ejemplo:

```text
# mesh name: atDepth1.4cm
# multiplied factor : 4.25264e+12
# primitive scorer name: dose
# iX, iY, iZ, total(value) [Gy], total(val^2), entry
0,0,0,3015015.320034376,1254115529.820403,21677
```

Si la corrida tuvo `N = 300000000` eventos:

```text
meanDose = 3015015.320034376 / 300000000 = 0.0100500511 Gy = 1.00501 cGy
rms = sqrt(300000000/299999999 * (1254115529.820403/300000000 - (3015015.320034376/300000000)^2))
    = 2.04457431 Gy = 204.457 cGy
```

que coincide con:

```text
------------------ Dose atDepth1.4cm ---------------------
0    1.00501 cGy rms = 204.457 cGy nEntries = 21677
```

## Comandos específicos `MultiDetector1/*`

Los siguientes comandos de macro son propios del proyecto y complementan los comandos estándar de Geant4 (`/run/*`, `/control/*`, `/gps/*`, `/score/*`).

### Control global

- `/MultiDetector1/control/SetPrimaryGeneratorType <int>`: selecciona el generador primario.

### Run

- `/MultiDetector1/run/SetMU <int>`: define las unidades monitor (MU) de la corrida.

### Linac (`clinac`)

- `/MultiDetector1/beamline/clinac/setJaw1X <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/setJaw2X <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/setJaw1Y <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/setJaw2Y <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/rotateGantry <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/rotateGantryTo <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/rotateCollimator <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/rotateCollimatorTo <valor> <unidad>`
- `/MultiDetector1/beamline/clinac/rotateGantryPhSp <valor> <unidad>` (`Idle` con la arquitectura actual)
- `/MultiDetector1/beamline/clinac/rotateGantryPhSpTo <valor> <unidad>` (`Idle` con la arquitectura actual)
- `/MultiDetector1/beamline/clinac/rotateCollimatorPhSp <valor> <unidad>` (`Idle` con la arquitectura actual)
- `/MultiDetector1/beamline/clinac/rotateCollimatorPhSpTo <valor> <unidad>` (`Idle` con la arquitectura actual)
- `/MultiDetector1/beamline/clinac/phsp/addFile <ruta>` (`PreInit`)
- `/MultiDetector1/beamline/clinac/phsp/clearFiles` (`PreInit`)
- `/MultiDetector1/beamline/clinac/phsp/setPrefix <prefijo>` (`PreInit`)
- `/MultiDetector1/beamline/clinac/phsp/clearPrefix` (`PreInit`)
- `/MultiDetector1/beamline/clinac/phsp/setEOFPolicy <abort|restart|stop|synthetic>` (`PreInit` o `Idle`)
- `/MultiDetector1/beamline/clinac/phsp/listFiles` (`PreInit` o `Idle`)

Los comandos `rotate*PhSp` no se soportan en `PreInit` mientras sigan registrados dentro de `MD1PrimaryGeneratorAction1`.
Para PHSP multiarchivo, la precedencia es: lista explícita > prefijo autodetectado.
En corridas MT, la fuente PHSP se selecciona de forma determinista como `eventID % numSources`. La asignación de fuente por evento no depende del scheduling de workers.
Cada worker crea sus propios `G4IAEAphspReader`, uno por fuente configurada, pero ahora los materializa de forma lazy en el primer evento que realmente necesita esa fuente.
Cada reader consume una partición disjunta del archivo usando `parallelRun = workerIndex + 1` y `totalParallelRuns = numberOfThreads`.
`setEOFPolicy` valida el valor en el momento del comando; un typo falla de inmediato en vez de quedar diferido hasta la lectura del primer evento.

### Fantoma de agua (`waterbox`)

- `/MultiDetector1/phantoms/waterbox/phantomID <int>`
- `/MultiDetector1/phantoms/waterbox/translate <dx> <dy> <dz> <unidad>`
- `/MultiDetector1/phantoms/waterbox/translateTo <x> <y> <z> <unidad>`
- `/MultiDetector1/phantoms/waterbox/rotateX <valor> <unidad>`
- `/MultiDetector1/phantoms/waterbox/rotateY <valor> <unidad>`
- `/MultiDetector1/phantoms/waterbox/rotateZ <valor> <unidad>`
- `/MultiDetector1/phantoms/waterbox/rotateTo <theta> <phi> <psi> <unidad>`
- `/MultiDetector1/phantoms/waterbox/addGeometryTo <logicalVolumeName> <copyNo>`
- `/MultiDetector1/phantoms/waterbox/removeGeometry <phantomID>`

### Registry de detectores

- `/MultiDetector1/detectors/list`

### Detector Cube (`cube`)

- `/MultiDetector1/detectors/cube/setSide <valor> <unidad>` (`PreInit`)
- `/MultiDetector1/detectors/cube/setMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/cube/setEnvelopeThickness <valor> <unidad>` (`PreInit`, `0` desactiva la envolvente)
- `/MultiDetector1/detectors/cube/setEnvelopeMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/cube/setSplitAtInterface <bool>` (`PreInit`)
- `/MultiDetector1/detectors/cube/setCalibrationFactor <valor_en_cGy_por_nC>` (`PreInit`, override opcional de la tabla local del calibrador)
- `/MultiDetector1/detectors/cube/setCalibrationFactorError <valor_en_cGy_por_nC>` (`PreInit`, override opcional del error de la tabla local del calibrador)
- `/MultiDetector1/detectors/cube/detectorID <int>`
- `/MultiDetector1/detectors/cube/translate <dx> <dy> <dz> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/translateTo <x> <y> <z> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateX <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateY <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateZ <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateTo <theta> <phi> <psi> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/addGeometryTo <logicalVolumeName> <copyNo>` (`PreInit` o `Idle`, recomendado `PreInit` para activar el detector en la corrida)
- `/MultiDetector1/detectors/cube/removeGeometry <detectorID>` (`PreInit` o `Idle`)

La tabla externa por defecto del cubo se lee desde [CubeCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/cube/geometry/CubeCalibrationTable.dat).
Formato por linea:
`<material> <lado> <unidad> <calibrationFactor_en_cGy_por_nC> <calibrationFactorError_en_cGy_por_nC>`

La envolvente del `cube` es opcional:

- `setEnvelopeThickness 0` mantiene el comportamiento actual sin envolvente
- si `setEnvelopeThickness > 0`, se construye un volumen cúbico externo de lado `cubeSide + 2 * envelopeThickness`
- el volumen sensible sigue siendo el cubo interno `DetectorCube`; la envolvente no cambia el SD ni el readout

El `cube` tambien soporta `split at interface` automatico contra la cara superior del `WaterBox`:

- `setSplitAtInterface true` divide automaticamente el detector si sobresale de `WaterBox` hacia aire
- la parte interna queda en `WaterBox` y la parte externa se recoloca en `world`
- en esta v1 solo se soporta el caso sin rotacion del detector y cruce por la cara superior en `Z`
- si el detector sobresale por otra cara, queda totalmente por fuera o esta rotado, la corrida aborta con un error explicito

### Detector Cylinder (`cylinder`)

- `/MultiDetector1/detectors/cylinder/setRadius <valor> <unidad>` (`PreInit`)
- `/MultiDetector1/detectors/cylinder/setHeight <valor> <unidad>` (`PreInit`)
- `/MultiDetector1/detectors/cylinder/setMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/cylinder/setEnvelopeThickness <valor> <unidad>` (`PreInit`, `0` desactiva la envolvente)
- `/MultiDetector1/detectors/cylinder/setEnvelopeMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/cylinder/setSplitAtInterface <bool>` (`PreInit`)
- `/MultiDetector1/detectors/cylinder/setCalibrationFactor <valor_en_cGy_por_nC>` (`PreInit`, override opcional del calibrador)
- `/MultiDetector1/detectors/cylinder/setCalibrationFactorError <valor_en_cGy_por_nC>` (`PreInit`, override opcional del calibrador)
- `/MultiDetector1/detectors/cylinder/detectorID <int>`
- `/MultiDetector1/detectors/cylinder/translate <dx> <dy> <dz> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/translateTo <x> <y> <z> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/rotateX <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/rotateY <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/rotateZ <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/rotateTo <theta> <phi> <psi> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cylinder/addGeometryTo <logicalVolumeName> <copyNo>` (`PreInit` o `Idle`)
- `/MultiDetector1/detectors/cylinder/removeGeometry <detectorID>` (`PreInit` o `Idle`)

La tabla externa por defecto del cilindro se lee desde [CylinderCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/cylinder/geometry/CylinderCalibrationTable.dat).

- cubre la geometría nominal `radius = 2.5 mm`, `height = 5.0 mm`
- los factores actuales son provisionales y se derivan de `cube` escalando por volumen sensible relativo, conservando el error relativo por escenario material/envolvente
- si configuras otro radio o altura, debes usar `setCalibrationFactor` y `setCalibrationFactorError`
- `split at interface` sigue las mismas restricciones del cubo: solo `WaterBox`, cruce por la cara superior en `Z` y sin rotación

### Detector Sphere (`sphere`)

- `/MultiDetector1/detectors/sphere/setRadius <valor> <unidad>` (`PreInit`)
- `/MultiDetector1/detectors/sphere/setMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/sphere/setEnvelopeThickness <valor> <unidad>` (`PreInit`, `0` desactiva la envolvente)
- `/MultiDetector1/detectors/sphere/setEnvelopeMaterial <NISTMaterial>` (`PreInit`)
- `/MultiDetector1/detectors/sphere/setSplitAtInterface <bool>` (`PreInit`)
- `/MultiDetector1/detectors/sphere/setCalibrationFactor <valor_en_cGy_por_nC>` (`PreInit`, override opcional del calibrador)
- `/MultiDetector1/detectors/sphere/setCalibrationFactorError <valor_en_cGy_por_nC>` (`PreInit`, override opcional del calibrador)
- `/MultiDetector1/detectors/sphere/detectorID <int>`
- `/MultiDetector1/detectors/sphere/translate <dx> <dy> <dz> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/translateTo <x> <y> <z> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/rotateX <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/rotateY <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/rotateZ <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/rotateTo <theta> <phi> <psi> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/sphere/addGeometryTo <logicalVolumeName> <copyNo>` (`PreInit` o `Idle`)
- `/MultiDetector1/detectors/sphere/removeGeometry <detectorID>` (`PreInit` o `Idle`)

La tabla externa por defecto de la esfera se lee desde [SphereCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/basic/sphere/geometry/SphereCalibrationTable.dat).

- cubre la geometría nominal `radius = 2.5 mm`
- los factores actuales son provisionales y se derivan de `cube` escalando por volumen sensible relativo, conservando el error relativo por escenario material/envolvente
- si configuras otro radio, debes usar `setCalibrationFactor` y `setCalibrationFactorError`
- `split at interface` sigue las mismas restricciones del cubo: solo `WaterBox`, cruce por la cara superior en `Z` y sin rotación

### Detector BB7 (`BB7`)

- `/MultiDetector1/detectors/BB7/detectorID <int>`
- `/MultiDetector1/detectors/BB7/setCalibrationFactor <valor_en_cGy_por_nC>` (`PreInit`, override opcional del calibrador para el `detectorID` seleccionado)
- `/MultiDetector1/detectors/BB7/setCalibrationFactorError <valor_en_cGy_por_nC>` (`PreInit`, override opcional del error del calibrador para el `detectorID` seleccionado)
- `/MultiDetector1/detectors/BB7/translate <dx> <dy> <dz> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/translateTo <x> <y> <z> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/rotateX <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/rotateY <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/rotateZ <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/rotateTo <theta> <phi> <psi> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/BB7/addGeometryTo <logicalVolumeName> <copyNo>` (`PreInit` o `Idle`, recomendado `PreInit` para activar el detector en la corrida)
- `/MultiDetector1/detectors/BB7/removeGeometry <detectorID>` (`PreInit` o `Idle`)

### Biasing

- `/MultiDetector1/biasing/setSplittingFactor <int>`
- `/MultiDetector1/biasing/setApplyProbability <double>`

### Ejemplos de uso en macro

Configuración PHSP + geometría base:

```tcl
/MultiDetector1/control/SetPrimaryGeneratorType 1
/MultiDetector1/beamline/clinac/phsp/clearFiles
/MultiDetector1/beamline/clinac/phsp/addFile beam/Varian_TrueBeam6MV_01
/MultiDetector1/beamline/clinac/setJaw1X 5 cm
/MultiDetector1/beamline/clinac/setJaw2X 5 cm
/MultiDetector1/beamline/clinac/setJaw1Y 5 cm
/MultiDetector1/beamline/clinac/setJaw2Y 5 cm
/MultiDetector1/phantoms/waterbox/addGeometryTo world_log 0
/MultiDetector1/phantoms/waterbox/translateTo 0 0 -10 cm
```

Configuración PHSP multiarchivo en round-robin:

```tcl
/MultiDetector1/control/SetPrimaryGeneratorType 1
/MultiDetector1/beamline/clinac/phsp/clearFiles
/MultiDetector1/beamline/clinac/phsp/addFile beam/Varian_TrueBeam6MV_01
/MultiDetector1/beamline/clinac/phsp/addFile beam/Varian_TrueBeam6MV_02
/MultiDetector1/beamline/clinac/phsp/listFiles
/run/initialize
```

Con dos fuentes, la política efectiva queda:

- `eventID 0, 2, 4, ... -> source 0`
- `eventID 1, 3, 5, ... -> source 1`

Esto mantiene estable la asignación `evento -> fuente` entre corridas idénticas; lo que cambia entre workers es únicamente la partición disjunta del archivo consumida por cada reader local.

Configuración con registry de detectores y detector BB7:

```tcl
/MultiDetector1/detectors/list
/MultiDetector1/detectors/BB7/addGeometryTo WaterBox 0
/run/initialize
/MultiDetector1/detectors/BB7/detectorID 0
/MultiDetector1/detectors/BB7/translateTo 0 0 -10 cm
/MultiDetector1/detectors/BB7/rotateX -90 deg
/MultiDetector1/beamline/clinac/rotateGantry 30 deg
/MultiDetector1/beamline/clinac/rotateCollimator 45 deg
/MultiDetector1/run/SetMU 1
/MultiDetector1/beamline/clinac/rotateGantryPhSp 30 deg
/MultiDetector1/beamline/clinac/rotateCollimatorPhSp 45 deg
```

Biasing y corrida:

```tcl
/MultiDetector1/biasing/setSplittingFactor 20
/MultiDetector1/biasing/setApplyProbability 1.0
/run/beamOn 1000000
```
