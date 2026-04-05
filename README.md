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

- [launch_calibration_dmax_average.sh](/Users/acsevillam/workspace/Geant4/MDSIM1/jobs/dosimetry/launch_calibration_dmax_average.sh): ejecuta 5 réplicas de `input/dosimetry/Calibration.in` con `-b on -v off -n 300000000` y calcula el promedio de `Dose atDepth1.4cm` (`dmax` en este caso).

Este job escribe:

- una carpeta por réplica con `output.log`, `macro.in` y los archivos de `analysis/`
- `dmax_summary.csv` con una fila por réplica
- `dmax_average.txt` con el promedio final de `Dose atDepth1.4cm`

## Arquitectura de detectores

El proyecto quedó reorganizado para soportar múltiples detectores de radiación como módulos autocontenidos.

- `include/geometry/base/` y `src/geometry/base/`: contratos comunes, registry y utilidades base.
- `include/geometry/detectors/<detector>/` y `src/geometry/detectors/<detector>/`: implementación por detector.
- `include/geometry/beamline/` y `src/geometry/beamline/`: acelerador y componentes del haz.
- `include/geometry/phantoms/` y `src/geometry/phantoms/`: fantomas y geometrías auxiliares.

Cada detector vive como un módulo con:

- geometría
- messenger
- sensitive detector
- hit
- digit
- digitizer

El ensamblado ya no está hardcodeado en `MD1DetectorConstruction` ni en `MD1EventAction`. Ahora se realiza mediante `DetectorRegistry`, que conoce los detectores disponibles y cuáles están habilitados para la corrida actual.

### Detectores disponibles

- `cube`
- `BB7`

### Namespaces por detector

Los comandos específicos de cada detector ahora viven bajo:

- `/MultiDetector1/detectors/cube/*`
- `/MultiDetector1/detectors/BB7/*`

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

## Requisitos

- CMake 3.16 o superior.
- Geant4 instalado y disponible para CMake (`find_package(Geant4 REQUIRED ...)`).
- Entorno de Geant4 cargado antes de compilar/ejecutar (por ejemplo, `geant4.sh`).
- Git LFS si se van a usar los phase spaces binarios incluidos como `.IAEAphsp`.

## Compilación

Desde la carpeta raíz del proyecto:

```bash
cmake -S . -B MDSIM-build
cmake --build MDSIM-build -jN
```

N corresponde con el número de núcleos disponibles en el equipo.

Esto genera el ejecutable:

- `MDSIM-build/MultiDetector1`

Nota: durante la configuración se copian macros y archivos de entrada al directorio de compilación (`MDSIM-build/`). Por esta razón se recomienda ejecutar la simulación desde `MDSIM-build/`.

Si los archivos `beam/*.IAEAphsp` aparecen como punteros de Git LFS en vez de binarios reales, la simulación abortará con un mensaje claro. Antes de ejecutar casos PHSP, descarga los objetos reales con:

```bash
git lfs pull
```

## Ejecución

Entrar al directorio de compilación:

```bash
cd MDSIM-build
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

### Plantillas parametrizables (`.intmp`)

- `input/dosimetry/templates/Calibration.intmp`: plantilla de calibración con reemplazo principal de `**PhspFileName**`.
- `input/dosimetry/templates/Validation.intmp`: plantilla de validación con reemplazo de `**PhspFileName**`.
- `input/detectors/BB7/templates/BB7.intmp`: plantilla BB7 para barridos de `**PhspFileName**`, aperturas de jaws, `**GantryAngle**`, `**CollimatorAngle**` y `**DetectorAngle**`.
- `input/detectors/BB7/templates/BB7Calibration.intmp`: plantilla de calibración BB7/Co-60 con parámetros geométricos del haz (`**BeamCentre**`, `**BeamRot1**`, `**BeamRot2**`, `**BeamDirection**`) y ángulos de sistema.
- `input/detectors/cube/templates/Cube.intmp`: plantilla del detector `cube` para barridos PHSP con `**PhspPrefix**`, jaws, material, lado del cubo y posición/rotación básicas.
- `input/detectors/cube/templates/CubeCalibration.intmp`: plantilla de calibración dosimétrica del `cube` con PHSP, jaws, ángulos del linac y scorers puntuales en profundidad.

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
- `/MultiDetector1/beamline/clinac/phsp/listFiles` (`PreInit` o `Idle`)

Los comandos `rotate*PhSp` no se soportan en `PreInit` mientras sigan registrados dentro de `MD1PrimaryGeneratorAction1`.
Para PHSP multiarchivo, la precedencia es: lista explícita > prefijo autodetectado.

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
- `/MultiDetector1/detectors/cube/setCalibrationFactor <valor_en_Gy_por_C>` (`PreInit`, override opcional de la tabla local)
- `/MultiDetector1/detectors/cube/detectorID <int>`
- `/MultiDetector1/detectors/cube/translate <dx> <dy> <dz> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/translateTo <x> <y> <z> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateX <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateY <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateZ <valor> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/rotateTo <theta> <phi> <psi> <unidad>` (`Idle`)
- `/MultiDetector1/detectors/cube/addGeometryTo <logicalVolumeName> <copyNo>` (`PreInit` o `Idle`, recomendado `PreInit` para activar el detector en la corrida)
- `/MultiDetector1/detectors/cube/removeGeometry <detectorID>` (`PreInit` o `Idle`)

La tabla externa por defecto del cubo se lee desde [CubeCalibrationTable.dat](/Users/acsevillam/workspace/Geant4/MDSIM1/src/geometry/detectors/cube/geometry/CubeCalibrationTable.dat).
Formato por linea:
`<material> <lado> <unidad> <calibrationFactor_en_Gy_por_C>`

### Detector BB7 (`BB7`)

- `/MultiDetector1/detectors/BB7/detectorID <int>`
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
