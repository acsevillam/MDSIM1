#pragma once

#include "globals.hh"

namespace MD1 {

struct BasicDosimeterCalibrationParameters {
    G4double calibrationFactor = 0.;
    G4double calibrationFactorError = 0.;
};

struct BasicDosimeterCalibrationResult {
    G4double estimatedDoseToWater = 0.;
    G4double calibrationError = 0.;
};

void ValidateBasicDosimeterCalibrationParameters(
    const BasicDosimeterCalibrationParameters& parameters,
    const G4String& exceptionOrigin,
    const G4String& invalidFactorCode,
    const G4String& invalidFactorErrorCode,
    const G4String& detectorName);

BasicDosimeterCalibrationResult CalibrateBasicDosimeterCollectedCharge(
    G4double collectedCharge,
    const BasicDosimeterCalibrationParameters& parameters);

} // namespace MD1
