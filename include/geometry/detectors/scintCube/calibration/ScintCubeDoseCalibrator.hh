#ifndef MDSIM1_SCINT_CUBE_DOSE_CALIBRATOR_HH
#define MDSIM1_SCINT_CUBE_DOSE_CALIBRATOR_HH

#include "G4SystemOfUnits.hh"
#include "globals.hh"

class ScintCubeDigit;

struct ScintCubeCalibrationParameters {
    G4double doseCalibrationFactor = 1.0e-12 * gray;
    G4double doseCalibrationFactorError = 0.;
};

struct ScintCubeDoseCalibrationResult {
    G4double estimatedDoseToWater = 0.;
    G4double calibrationError = 0.;
};

class ScintCubeDoseCalibrator {
public:
    explicit ScintCubeDoseCalibrator(const ScintCubeCalibrationParameters& parameters);

    ScintCubeDoseCalibrationResult Calibrate(const ScintCubeDigit& digit) const;

private:
    ScintCubeCalibrationParameters fParameters;
};

#endif // MDSIM1_SCINT_CUBE_DOSE_CALIBRATOR_HH
