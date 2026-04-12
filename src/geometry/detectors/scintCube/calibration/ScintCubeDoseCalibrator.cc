#include "geometry/detectors/scintCube/calibration/ScintCubeDoseCalibrator.hh"

#include <cmath>

#include "G4Exception.hh"

#include "geometry/detectors/scintCube/readout/ScintCubeDigit.hh"

ScintCubeDoseCalibrator::ScintCubeDoseCalibrator(const ScintCubeCalibrationParameters& parameters)
    : fParameters(parameters) {
    if (fParameters.doseCalibrationFactor <= 0.) {
        G4Exception("ScintCubeDoseCalibrator::ScintCubeDoseCalibrator",
                    "ScintCubeInvalidDoseCalibrationFactor",
                    FatalException,
                    "ScintCube dose calibration factor must be strictly positive.");
    }
    if (fParameters.doseCalibrationFactorError < 0.) {
        G4Exception("ScintCubeDoseCalibrator::ScintCubeDoseCalibrator",
                    "ScintCubeInvalidDoseCalibrationFactorError",
                    FatalException,
                    "ScintCube dose calibration factor uncertainty must be non-negative.");
    }
}

ScintCubeDoseCalibrationResult ScintCubeDoseCalibrator::Calibrate(const ScintCubeDigit& digit) const {
    ScintCubeDoseCalibrationResult result;
    result.estimatedDoseToWater =
        digit.GetDetectedPhotoelectrons() * fParameters.doseCalibrationFactor;
    result.calibrationError =
        std::abs(digit.GetDetectedPhotoelectrons()) * fParameters.doseCalibrationFactorError;
    return result;
}
