#include "geometry/detectors/basic/cube/calibration/CubeDoseCalibrator.hh"

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"
#include "geometry/detectors/basic/cube/readout/CubeCalibrationTable.hh"
#include "geometry/detectors/basic/cube/readout/CubeDigit.hh"

CubeDoseCalibrator::CubeDoseCalibrator(const CubeCalibrationParameters& parameters)
    : fParameters(parameters) {
    MD1::ValidateBasicDosimeterCalibrationParameters(fParameters,
                                                     "CubeDoseCalibrator::CubeDoseCalibrator",
                                                     "CubeInvalidCalibrationFactor",
                                                     "CubeInvalidCalibrationFactorError",
                                                     "Cube");
}

CubeCalibrationParameters CubeDoseCalibrator::BuildParameters(
    const G4String& materialName,
    G4double cubeSide,
    const G4String& envelopeMaterialName,
    G4double envelopeThickness,
    G4double calibrationFactor,
    G4double calibrationFactorError) {
    CubeCalibrationParameters parameters;
    if (calibrationFactor > 0.) {
        parameters.calibrationFactor = calibrationFactor;
        parameters.calibrationFactorError = calibrationFactorError;
    } else {
        const auto calibrationData = CubeCalibrationTable::GetCalibrationData(
            materialName, cubeSide, envelopeMaterialName, envelopeThickness);
        parameters.calibrationFactor = calibrationData.factor;
        parameters.calibrationFactorError = calibrationData.factorError;
    }

    if (cubeSide <= 0.) {
        G4Exception("CubeDoseCalibrator::BuildParameters",
                    "CubeInvalidCubeSide",
                    FatalException,
                    "Cube side length must be positive.");
    }
    if (parameters.calibrationFactor <= 0.) {
        G4Exception("CubeDoseCalibrator::BuildParameters",
                    "CubeInvalidCalibrationFactor",
                    FatalException,
                    "Cube calibration factor must be defined by override or local calibration table and be positive.");
    }
    if (parameters.calibrationFactorError < 0.) {
        G4Exception("CubeDoseCalibrator::BuildParameters",
                    "CubeInvalidCalibrationFactorError",
                    FatalException,
                    "Cube calibration factor uncertainty must be non-negative.");
    }

    return parameters;
}

G4bool CubeDoseCalibrator::HasCalibrationFactor(const G4String& materialName,
                                                G4double cubeSide,
                                                const G4String& envelopeMaterialName,
                                                G4double envelopeThickness,
                                                G4double calibrationFactor) {
    if (calibrationFactor > 0.) {
        return true;
    }

    return CubeCalibrationTable::HasCalibrationFactor(
        materialName, cubeSide, envelopeMaterialName, envelopeThickness);
}

CubeDoseCalibrationResult CubeDoseCalibrator::Calibrate(const CubeDigit& digit) const {
    return MD1::CalibrateBasicDosimeterCollectedCharge(digit.GetCollectedCharge(), fParameters);
}
