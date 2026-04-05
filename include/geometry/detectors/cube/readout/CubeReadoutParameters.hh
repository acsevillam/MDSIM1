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

#ifndef CUBE_READOUT_PARAMETERS_H
#define CUBE_READOUT_PARAMETERS_H

#include "globals.hh"

struct CubeReadoutParameters {
    G4double meanEnergyPerIon = 3.6;
    G4double elementaryCharge = 1.60217663e-19;
    G4double calibrationFactor = 0.;
};

#endif // CUBE_READOUT_PARAMETERS_H
