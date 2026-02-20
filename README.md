# MultiDetector1

Geant4 MultiDetector Simulation v1
Copyright (c) 2024 Andrés Camilo Sevilla
acsevillam@eafit.edu.co - acsevillam@gmail.com
All Rights Reserved.

`MultiDetector1` (MDSIM1) es una simulación Monte Carlo basada en Geant4 orientada a la evaluación y caracterización de múltiples detectores de radiación usados en radioterapia. MDSIM1 permite construir distintas configuraciones geométricas (linac, fantoma de agua, detectores), definir fuentes a partir de phase space o GPS, y calcular distribuciones de dosis mediante scorers de Geant4.

El flujo de trabajo del proyecto está enfocado a:

- Ejecutar casos de referencia con macros fijas (`input/*.in`);
- Correr barridos parametrizados con plantillas (`input/*.intmp`) y scripts (`jobs/*.sh`);
- Guardar resultados para análisis posterior en `analysis/`.

## Requisitos

- CMake (el proyecto declara `cmake_minimum_required(VERSION 4.2)` en `CMakeLists.txt`).
- Geant4 instalado y disponible para CMake (`find_package(Geant4 REQUIRED ...)`).
- Entorno de Geant4 cargado antes de compilar/ejecutar (por ejemplo, `geant4.sh`).

## Compilación

Desde la carpeta contenedora del proyecto:

```bash
mkdir MDSIM-build
cd MDSIM-build
cmake ../MDSIM
make build -jN
```

N corresponde con el número de núcleos disponibles en el equipo.

Esto genera el ejecutable:

- `MDSIM-build/MultiDetector1`

Nota: durante la configuración se copian macros y archivos de entrada al directorio de compilación (`MDSIM-build/`). Por está razón se recomienda ejecutar la simulación desde `MDSIM-build/`.

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
./MultiDetector1 -m input/Calibration.in -v off -b on -n 1000000
```

Parámetros disponibles:

- `-m <macro>`: macro principal (por defecto `mac/init.mac`).
- `-v <on|off>`: activa/desactiva visualización.
- `-vm <vis_macro>`: macro de visualización (por defecto `mac/vis.mac`).
- `-b <on|off>`: activa/desactiva biasing.
- `-n <eventos>`: número de eventos para `BeamOn` en modo no interactivo.

## Ejemplos útiles

Batch sin visualización y sin biasing:

```bash
./MultiDetector1 -m input/Validation.in -v off -b off -n 500000
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

- `input/Calibration.in`: configuración de calibración con `WaterBox` y scorers puntuales en profundidad (incluye mallas a 1.4 cm y 10 cm, y región de buildup).
- `input/Validation.in`: caso de validación dosimétrica en fantoma de agua con perfiles laterales (`x`, `y`) y curva de profundidad (`PDD`).
- `input/DoseMap3D.in`: cálculo volumétrico 3D de dosis en `WaterBox` mediante una malla cúbica.
- `input/BB7.in`: configuración con detector `DualBB7`, jaws y rotaciones de gantry/colimador para estudios geométricos del detector.
- `input/BB7Calibration.in`: configuración de calibración BB7 con fuente Co-60 (GPS) y malla 2D para mapa de dosis.

### Plantillas parametrizables (`.intmp`)

- `input/Calibration.intmp`: plantilla de calibración con reemplazo principal de `**PhspFileName**`.
- `input/Validation.intmp`: plantilla de validación con reemplazo de `**PhspFileName**`.
- `input/BB7.intmp`: plantilla BB7 para barridos de `**PhspFileName**`, aperturas de jaws, `**GantryAngle**`, `**CollimatorAngle**` y `**DetectorAngle**`.
- `input/BB7Calibration.intmp`: plantilla de calibración BB7/Co-60 con parámetros geométricos del haz (`**BeamCentre**`, `**BeamRot1**`, `**BeamRot2**`, `**BeamDirection**`) y ángulos de sistema.

### Macros de benchmark

- `input/benchmark/set1.in`: punto de referencia para caso TrueBeam (medición puntual en profundidad).
- `input/benchmark/set2.in`: conjunto de puntos de referencia con campo rectangular y SSD específico.
- `input/benchmark/set3.in`: conjunto de referencia para campo pequeño, evaluando dosis en distintas profundidades.

## Comandos específicos `MultiDetector1/*`

Los siguientes comandos de macro son propios del proyecto y complementan los comandos estándar de Geant4 (`/run/*`, `/control/*`, `/gps/*`, `/score/*`).

### Control global

- `/MultiDetector1/control/SetPrimaryGeneratorType <int>`: selecciona el generador primario.
- `/MultiDetector1/control/SetPhspFileName <ruta>`: define el archivo phase space (usado en modo PHSP).

### Run

- `/MultiDetector1/run/SetMU <int>`: define las unidades monitor (MU) de la corrida.

### Linac (`Clinac`)

- `/MultiDetector1/Clinac/setJaw1X <valor> <unidad>`
- `/MultiDetector1/Clinac/setJaw2X <valor> <unidad>`
- `/MultiDetector1/Clinac/setJaw1Y <valor> <unidad>`
- `/MultiDetector1/Clinac/setJaw2Y <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateGantry <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateGantryTo <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateCollimator <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateCollimatorTo <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateGantryPhSp <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateGantryPhSpTo <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateCollimatorPhSp <valor> <unidad>`
- `/MultiDetector1/Clinac/rotateCollimatorPhSpTo <valor> <unidad>`

### Fantoma de agua (`WaterBox`)

- `/MultiDetector1/WaterBox/phantomID <int>`
- `/MultiDetector1/WaterBox/translate <dx> <dy> <dz> <unidad>`
- `/MultiDetector1/WaterBox/translateTo <x> <y> <z> <unidad>`
- `/MultiDetector1/WaterBox/rotateX <valor> <unidad>`
- `/MultiDetector1/WaterBox/rotateY <valor> <unidad>`
- `/MultiDetector1/WaterBox/rotateZ <valor> <unidad>`
- `/MultiDetector1/WaterBox/rotateTo <theta> <phi> <psi> <unidad>`
- `/MultiDetector1/WaterBox/addGeometryTo <logicalVolumeName> <copyNo>`
- `/MultiDetector1/WaterBox/removeGeometry <phantomID>`

### Detector BB7 (`DualBB7`)

- `/MultiDetector1/DualBB7/detectorID <int>`
- `/MultiDetector1/DualBB7/translate <dx> <dy> <dz> <unidad>`
- `/MultiDetector1/DualBB7/translateTo <x> <y> <z> <unidad>`
- `/MultiDetector1/DualBB7/rotateX <valor> <unidad>`
- `/MultiDetector1/DualBB7/rotateY <valor> <unidad>`
- `/MultiDetector1/DualBB7/rotateZ <valor> <unidad>`
- `/MultiDetector1/DualBB7/rotateTo <theta> <phi> <psi> <unidad>`
- `/MultiDetector1/DualBB7/addGeometryTo <logicalVolumeName> <copyNo>`
- `/MultiDetector1/DualBB7/removeGeometry <detectorID>`

### Biasing

- `/MultiDetector1/biasing/setSplittingFactor <int>`
- `/MultiDetector1/biasing/setApplyProbability <double>`

### Ejemplos de uso en macro

Configuración PHSP + geometría base:

```tcl
/MultiDetector1/control/SetPrimaryGeneratorType 1
/MultiDetector1/control/SetPhspFileName beam/Varian_TrueBeam6MV_01
/MultiDetector1/Clinac/setJaw1X 5 cm
/MultiDetector1/Clinac/setJaw2X 5 cm
/MultiDetector1/Clinac/setJaw1Y 5 cm
/MultiDetector1/Clinac/setJaw2Y 5 cm
/MultiDetector1/WaterBox/addGeometryTo world_log 0
/MultiDetector1/WaterBox/translateTo 0 0 -10 cm
```

Configuración con detector BB7 y rotaciones:

```tcl
/MultiDetector1/DualBB7/addGeometryTo WaterBox 0
/MultiDetector1/DualBB7/detectorID 0
/MultiDetector1/DualBB7/translateTo 0 0 -10 cm
/MultiDetector1/DualBB7/rotateX -90 deg
/MultiDetector1/Clinac/rotateGantry 30 deg
/MultiDetector1/Clinac/rotateCollimator 45 deg
/MultiDetector1/Clinac/rotateGantryPhSp 30 deg
/MultiDetector1/Clinac/rotateCollimatorPhSp 45 deg
```

Biasing y corrida:

```tcl
/run/initialize
/MultiDetector1/biasing/setSplittingFactor 20
/MultiDetector1/biasing/setApplyProbability 1.0
/MultiDetector1/run/SetMU 1
/run/beamOn 1000000
```
