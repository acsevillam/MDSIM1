#ifndef MDSIM1_CUBE_DOSE_CALIBRATOR_HH
#define MDSIM1_CUBE_DOSE_CALIBRATOR_HH

#include "geometry/detectors/basic/core/BasicDosimeterCalibration.hh"

class CubeDigit;

using CubeCalibrationParameters = MD1::BasicDosimeterCalibrationParameters;
using CubeDoseCalibrationResult = MD1::BasicDosimeterCalibrationResult;

class CubeDoseCalibrator {
public:
    explicit CubeDoseCalibrator(const CubeCalibrationParameters& parameters);

    static CubeCalibrationParameters BuildParameters(const G4String& materialName,
                                                     G4double cubeSide,
                                                     const G4String& envelopeMaterialName,
                                                     G4double envelopeThickness,
                                                     G4double calibrationFactor,
                                                     G4double calibrationFactorError = 0.);
    static G4bool HasCalibrationFactor(const G4String& materialName,
                                       G4double cubeSide,
                                       const G4String& envelopeMaterialName,
                                       G4double envelopeThickness,
                                       G4double calibrationFactor);

    CubeDoseCalibrationResult Calibrate(const CubeDigit& digit) const;

private:
    CubeCalibrationParameters fParameters;
};

#endif // MDSIM1_CUBE_DOSE_CALIBRATOR_HH
