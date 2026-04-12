#include "geometry/detectors/basic/sphere/calibration/SphereDoseCalibrator.hh"

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"
#include "geometry/detectors/basic/sphere/readout/SphereCalibrationTable.hh"
#include "geometry/detectors/basic/sphere/readout/SphereDigit.hh"

SphereDoseCalibrator::SphereDoseCalibrator(const SphereCalibrationParameters& parameters)
    : fParameters(parameters) {
    MD1::ValidateBasicDosimeterCalibrationParameters(fParameters,
                                                     "SphereDoseCalibrator::SphereDoseCalibrator",
                                                     "SphereInvalidCalibrationFactor",
                                                     "SphereInvalidCalibrationFactorError",
                                                     "Sphere");
}

SphereCalibrationParameters SphereDoseCalibrator::BuildParameters(
    const G4String& materialName,
    G4double sphereRadius,
    const G4String& envelopeMaterialName,
    G4double envelopeThickness,
    G4double calibrationFactor,
    G4double calibrationFactorError) {
    SphereCalibrationParameters parameters;
    if (calibrationFactor > 0.) {
        parameters.calibrationFactor = calibrationFactor;
        parameters.calibrationFactorError = calibrationFactorError;
    } else {
        const auto calibrationData = SphereCalibrationTable::GetCalibrationData(
            materialName, sphereRadius, envelopeMaterialName, envelopeThickness);
        parameters.calibrationFactor = calibrationData.factor;
        parameters.calibrationFactorError = calibrationData.factorError;
    }

    if (sphereRadius <= 0.) {
        G4Exception("SphereDoseCalibrator::BuildParameters",
                    "SphereInvalidRadius",
                    FatalException,
                    "Sphere radius must be positive.");
    }
    if (parameters.calibrationFactor <= 0.) {
        G4Exception("SphereDoseCalibrator::BuildParameters",
                    "SphereInvalidCalibrationFactor",
                    FatalException,
                    "Sphere calibration factor must be defined by override or local calibration table and be positive.");
    }
    if (parameters.calibrationFactorError < 0.) {
        G4Exception("SphereDoseCalibrator::BuildParameters",
                    "SphereInvalidCalibrationFactorError",
                    FatalException,
                    "Sphere calibration factor uncertainty must be non-negative.");
    }

    return parameters;
}

G4bool SphereDoseCalibrator::HasCalibrationFactor(const G4String& materialName,
                                                  G4double sphereRadius,
                                                  const G4String& envelopeMaterialName,
                                                  G4double envelopeThickness,
                                                  G4double calibrationFactor) {
    if (calibrationFactor > 0.) {
        return true;
    }

    return SphereCalibrationTable::HasCalibrationFactor(
        materialName, sphereRadius, envelopeMaterialName, envelopeThickness);
}

SphereDoseCalibrationResult SphereDoseCalibrator::Calibrate(const SphereDigit& digit) const {
    return MD1::CalibrateBasicDosimeterCollectedCharge(digit.GetCollectedCharge(), fParameters);
}
