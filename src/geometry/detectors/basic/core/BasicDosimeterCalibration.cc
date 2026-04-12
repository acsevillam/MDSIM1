#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"

#include <cmath>

#include "G4Exception.hh"

namespace MD1 {

void ValidateBasicDosimeterCalibrationParameters(
    const BasicDosimeterCalibrationParameters& parameters,
    const G4String& exceptionOrigin,
    const G4String& invalidFactorCode,
    const G4String& invalidFactorErrorCode,
    const G4String& detectorName) {
    if (parameters.calibrationFactor <= 0.) {
        G4Exception(exceptionOrigin.c_str(),
                    invalidFactorCode.c_str(),
                    FatalException,
                    (detectorName + " calibration factor must be strictly positive.").c_str());
    }
    if (parameters.calibrationFactorError < 0.) {
        G4Exception(exceptionOrigin.c_str(),
                    invalidFactorErrorCode.c_str(),
                    FatalException,
                    (detectorName + " calibration factor uncertainty must be non-negative.").c_str());
    }
}

BasicDosimeterCalibrationResult CalibrateBasicDosimeterCollectedCharge(
    G4double collectedCharge,
    const BasicDosimeterCalibrationParameters& parameters) {
    BasicDosimeterCalibrationResult result;
    result.estimatedDoseToWater = collectedCharge * parameters.calibrationFactor;
    result.calibrationError = std::abs(collectedCharge) * parameters.calibrationFactorError;
    return result;
}

} // namespace MD1
