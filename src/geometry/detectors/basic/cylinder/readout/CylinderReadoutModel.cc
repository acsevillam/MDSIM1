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

#include "geometry/detectors/basic/cylinder/readout/CylinderReadoutModel.hh"

#include "geometry/detectors/basic/core/BasicDosimeterReadout.hh"

CylinderReadoutParameters CylinderReadoutModel::Build(const G4String& materialName,
                                                      G4double cylinderRadius,
                                                      G4double cylinderHeight,
                                                      const G4String& envelopeMaterialName,
                                                      G4double envelopeThickness) {
    static_cast<void>(envelopeMaterialName);
    static_cast<void>(envelopeThickness);
    return MD1::BuildBasicDosimeterReadoutParameters(
        materialName,
        {{"cylinderRadius", cylinderRadius}, {"cylinderHeight", cylinderHeight}},
        "CylinderReadoutModel::Build");
}
