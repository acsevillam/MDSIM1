/*
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

#ifndef BB7_READOUT_PARAMETERS_H
#define BB7_READOUT_PARAMETERS_H

#include "globals.hh"

struct BB7ReadoutParameters {
    G4double meanEnergyPerIon = 0.;
    G4double elementaryCharge = 0.;
    G4double calibrationFactor = 0.;
    G4double calibrationFactorError = 0.;
};

#endif // BB7_READOUT_PARAMETERS_H
