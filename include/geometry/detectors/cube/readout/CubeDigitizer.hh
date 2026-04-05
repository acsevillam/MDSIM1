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

#ifndef CUBE_DIGITIZER_H
#define CUBE_DIGITIZER_H

// Geant4 Headers
#include "G4SystemOfUnits.hh"
#include "geometry/base/BaseDigitizer.hh"

// MultiDetector Headers
#include "geometry/detectors/cube/readout/CubeDigit.hh"
#include "geometry/detectors/cube/readout/CubeHit.hh"
#include "geometry/detectors/cube/readout/CubeReadoutParameters.hh"

class CubeDigitizer : public BaseDigitizer {
public:
    CubeDigitizer(const G4String& name, const CubeReadoutParameters& readoutParameters);
    ~CubeDigitizer() override;

    void Digitize() override;

private:
    const CubeHitsCollection* fHitsCollection;
    CubeDigitsCollection* fDigitsCollection;
    G4int fDCID;
    CubeReadoutParameters fReadoutParameters;
};

#endif // CUBE_DIGITIZER_H
