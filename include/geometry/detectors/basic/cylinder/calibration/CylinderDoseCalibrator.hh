#pragma once

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"

class CylinderDigit;

using CylinderCalibrationParameters = MD1::BasicDosimeterCalibrationParameters;
using CylinderDoseCalibrationResult = MD1::BasicDosimeterCalibrationResult;

class CylinderDoseCalibrator {
public:
    explicit CylinderDoseCalibrator(const CylinderCalibrationParameters& parameters);

    static CylinderCalibrationParameters BuildParameters(const G4String& materialName,
                                                         G4double cylinderRadius,
                                                         G4double cylinderHeight,
                                                         const G4String& envelopeMaterialName,
                                                         G4double envelopeThickness,
                                                         G4double calibrationFactor,
                                                         G4double calibrationFactorError = 0.);
    static G4bool HasCalibrationFactor(const G4String& materialName,
                                       G4double cylinderRadius,
                                       G4double cylinderHeight,
                                       const G4String& envelopeMaterialName,
                                       G4double envelopeThickness,
                                       G4double calibrationFactor);

    CylinderDoseCalibrationResult Calibrate(const CylinderDigit& digit) const;

private:
    CylinderCalibrationParameters fParameters;
};
