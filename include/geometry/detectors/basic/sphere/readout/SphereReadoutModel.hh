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

#include "geometry/detectors/basic/sphere/readout/SphereReadoutParameters.hh"
#include "globals.hh"

class SphereReadoutModel {
public:
    static SphereReadoutParameters Build(const G4String& materialName,
                                         G4double sphereRadius,
                                         const G4String& envelopeMaterialName,
                                         G4double envelopeThickness);
};
