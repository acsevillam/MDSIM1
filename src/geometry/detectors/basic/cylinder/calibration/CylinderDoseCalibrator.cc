#include "geometry/detectors/basic/cylinder/calibration/CylinderDoseCalibrator.hh"

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderCalibrationTable.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderDigit.hh"

CylinderDoseCalibrator::CylinderDoseCalibrator(const CylinderCalibrationParameters& parameters)
    : fParameters(parameters) {
    MD1::ValidateBasicDosimeterCalibrationParameters(fParameters,
                                                     "CylinderDoseCalibrator::CylinderDoseCalibrator",
                                                     "CylinderInvalidCalibrationFactor",
                                                     "CylinderInvalidCalibrationFactorError",
                                                     "Cylinder");
}

CylinderCalibrationParameters CylinderDoseCalibrator::BuildParameters(
    const G4String& materialName,
    G4double cylinderRadius,
    G4double cylinderHeight,
    const G4String& envelopeMaterialName,
    G4double envelopeThickness,
    G4double calibrationFactor,
    G4double calibrationFactorError) {
    CylinderCalibrationParameters parameters;
    if (calibrationFactor > 0.) {
        parameters.calibrationFactor = calibrationFactor;
        parameters.calibrationFactorError = calibrationFactorError;
    } else {
        const auto calibrationData = CylinderCalibrationTable::GetCalibrationData(
            materialName, cylinderRadius, cylinderHeight, envelopeMaterialName, envelopeThickness);
        parameters.calibrationFactor = calibrationData.factor;
        parameters.calibrationFactorError = calibrationData.factorError;
    }

    if (cylinderRadius <= 0.) {
        G4Exception("CylinderDoseCalibrator::BuildParameters",
                    "CylinderInvalidRadius",
                    FatalException,
                    "Cylinder radius must be positive.");
    }
    if (cylinderHeight <= 0.) {
        G4Exception("CylinderDoseCalibrator::BuildParameters",
                    "CylinderInvalidHeight",
                    FatalException,
                    "Cylinder height must be positive.");
    }
    if (parameters.calibrationFactor <= 0.) {
        G4Exception("CylinderDoseCalibrator::BuildParameters",
                    "CylinderInvalidCalibrationFactor",
                    FatalException,
                    "Cylinder calibration factor must be defined by override or local calibration table and be positive.");
    }
    if (parameters.calibrationFactorError < 0.) {
        G4Exception("CylinderDoseCalibrator::BuildParameters",
                    "CylinderInvalidCalibrationFactorError",
                    FatalException,
                    "Cylinder calibration factor uncertainty must be non-negative.");
    }

    return parameters;
}

G4bool CylinderDoseCalibrator::HasCalibrationFactor(const G4String& materialName,
                                                    G4double cylinderRadius,
                                                    G4double cylinderHeight,
                                                    const G4String& envelopeMaterialName,
                                                    G4double envelopeThickness,
                                                    G4double calibrationFactor) {
    if (calibrationFactor > 0.) {
        return true;
    }

    return CylinderCalibrationTable::HasCalibrationFactor(
        materialName, cylinderRadius, cylinderHeight, envelopeMaterialName, envelopeThickness);
}

CylinderDoseCalibrationResult CylinderDoseCalibrator::Calibrate(const CylinderDigit& digit) const {
    return MD1::CalibrateBasicDosimeterCollectedCharge(digit.GetCollectedCharge(), fParameters);
}
