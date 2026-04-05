/*
 *
 * Geant4 MultiDetector Simulation
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

#ifndef CUBE_CALIBRATION_TABLE_H
#define CUBE_CALIBRATION_TABLE_H

#include "globals.hh"

class CubeCalibrationTable {
public:
    static G4bool HasCalibrationFactor(const G4String& materialName, G4double cubeSide);
    static G4double GetCalibrationFactor(const G4String& materialName, G4double cubeSide);
};

#endif // CUBE_CALIBRATION_TABLE_H
