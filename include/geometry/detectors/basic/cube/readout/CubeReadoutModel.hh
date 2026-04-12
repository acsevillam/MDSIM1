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

#ifndef CUBE_READOUT_MODEL_H
#define CUBE_READOUT_MODEL_H

#include "globals.hh"
#include "geometry/detectors/basic/cube/readout/CubeReadoutParameters.hh"

class CubeReadoutModel {
public:
    static CubeReadoutParameters Build(const G4String& materialName,
                                       G4double cubeSide,
                                       const G4String& envelopeMaterialName,
                                       G4double envelopeThickness);
};

#endif // CUBE_READOUT_MODEL_H
