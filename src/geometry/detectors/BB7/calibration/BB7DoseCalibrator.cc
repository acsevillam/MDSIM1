#include "geometry/detectors/BB7/calibration/BB7DoseCalibrator.hh"

#include <cmath>

#include "G4Exception.hh"

#include "geometry/detectors/BB7/readout/BB7Digit.hh"

BB7DoseCalibrator::BB7DoseCalibrator(const BB7CalibrationParameters& parameters)
    : fParameters(parameters) {
    if (fParameters.calibrationFactor <= 0.) {
        G4Exception("BB7DoseCalibrator::BB7DoseCalibrator",
                    "BB7InvalidCalibrationFactor",
                    FatalException,
                    "BB7 calibration factor must be strictly positive.");
    }
    if (fParameters.calibrationFactorError < 0.) {
        G4Exception("BB7DoseCalibrator::BB7DoseCalibrator",
                    "BB7InvalidCalibrationFactorError",
                    FatalException,
                    "BB7 calibration factor uncertainty must be non-negative.");
    }
}

BB7DoseCalibrationResult BB7DoseCalibrator::Calibrate(const BB7Digit& digit) const {
    BB7DoseCalibrationResult result;
    result.estimatedDoseToWater = digit.GetCollectedCharge() * fParameters.calibrationFactor;
    result.calibrationError =
        std::abs(digit.GetCollectedCharge()) * fParameters.calibrationFactorError;
    return result;
}
