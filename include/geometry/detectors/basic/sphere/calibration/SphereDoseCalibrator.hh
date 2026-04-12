#pragma once

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"

class SphereDigit;

using SphereCalibrationParameters = MD1::BasicDosimeterCalibrationParameters;
using SphereDoseCalibrationResult = MD1::BasicDosimeterCalibrationResult;

class SphereDoseCalibrator {
public:
    explicit SphereDoseCalibrator(const SphereCalibrationParameters& parameters);

    static SphereCalibrationParameters BuildParameters(const G4String& materialName,
                                                       G4double sphereRadius,
                                                       const G4String& envelopeMaterialName,
                                                       G4double envelopeThickness,
                                                       G4double calibrationFactor,
                                                       G4double calibrationFactorError = 0.);
    static G4bool HasCalibrationFactor(const G4String& materialName,
                                       G4double sphereRadius,
                                       const G4String& envelopeMaterialName,
                                       G4double envelopeThickness,
                                       G4double calibrationFactor);

    SphereDoseCalibrationResult Calibrate(const SphereDigit& digit) const;

private:
    SphereCalibrationParameters fParameters;
};
