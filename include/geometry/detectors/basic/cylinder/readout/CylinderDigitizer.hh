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
#include "geometry/detectors/basic/cylinder/readout/CylinderDigit.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderHit.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderReadoutParameters.hh"

class CylinderDigitizer : public BaseDigitizer {
public:
    CylinderDigitizer(const G4String& name, const CylinderReadoutParameters& readoutParameters);
    ~CylinderDigitizer() override;

    void Digitize() override;

private:
    const CylinderHitsCollection* fHitsCollection;
    CylinderDigitsCollection* fDigitsCollection;
    G4int fDCID;
    CylinderReadoutParameters fReadoutParameters;
};

