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


// Geant4 Headers
#include "G4SystemOfUnits.hh"
#include "geometry/base/BaseDigitizer.hh"

// MultiDetector Headers
#include "geometry/detectors/basic/sphere/readout/SphereDigit.hh"
#include "geometry/detectors/basic/sphere/readout/SphereHit.hh"
#include "geometry/detectors/basic/sphere/readout/SphereReadoutParameters.hh"

class SphereDigitizer : public BaseDigitizer {
public:
    SphereDigitizer(const G4String& name, const SphereReadoutParameters& readoutParameters);
    ~SphereDigitizer() override;

    void Digitize() override;

private:
    const SphereHitsCollection* fHitsCollection;
    SphereDigitsCollection* fDigitsCollection;
    G4int fDCID;
    SphereReadoutParameters fReadoutParameters;
};

