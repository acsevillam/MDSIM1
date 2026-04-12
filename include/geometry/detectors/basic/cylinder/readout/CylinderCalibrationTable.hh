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

#pragma once

#include "globals.hh"

class CylinderCalibrationTable {
public:
    struct CalibrationData {
        G4double factor = 0.;
        G4double factorError = 0.;
    };

    static G4bool HasCalibrationFactor(const G4String& materialName,
                                       G4double cylinderRadius,
                                       G4double cylinderHeight,
                                       const G4String& envelopeMaterialName,
                                       G4double envelopeThickness);
    static G4double GetCalibrationFactor(const G4String& materialName,
                                         G4double cylinderRadius,
                                         G4double cylinderHeight,
                                         const G4String& envelopeMaterialName,
                                         G4double envelopeThickness);
    static CalibrationData GetCalibrationData(const G4String& materialName,
                                              G4double cylinderRadius,
                                              G4double cylinderHeight,
                                              const G4String& envelopeMaterialName,
                                              G4double envelopeThickness);
};
