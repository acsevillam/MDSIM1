#include "geometry/detectors/model11/calibration/Model11DoseCalibrator.hh"

#include <cmath>

#include "G4Exception.hh"

#include "geometry/detectors/model11/readout/Model11Digit.hh"

Model11DoseCalibrator::Model11DoseCalibrator(const Model11CalibrationParameters& parameters)
    : fParameters(parameters) {
    if (fParameters.doseCalibrationFactor <= 0.) {
        G4Exception("Model11DoseCalibrator::Model11DoseCalibrator",
                    "Model11InvalidDoseCalibrationFactor",
                    FatalException,
                    "Model11 dose calibration factor must be strictly positive.");
    }
    if (fParameters.doseCalibrationFactorError < 0.) {
        G4Exception("Model11DoseCalibrator::Model11DoseCalibrator",
                    "Model11InvalidDoseCalibrationFactorError",
                    FatalException,
                    "Model11 dose calibration factor uncertainty must be non-negative.");
    }
}

Model11DoseCalibrationResult Model11DoseCalibrator::Calibrate(const Model11Digit& digit) const {
    Model11DoseCalibrationResult result;
    result.estimatedDoseToWater =
        digit.GetDetectedPhotoelectrons() * fParameters.doseCalibrationFactor;
    result.calibrationError =
        std::abs(digit.GetDetectedPhotoelectrons()) * fParameters.doseCalibrationFactorError;
    return result;
}
