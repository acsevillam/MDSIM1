#ifndef MDSIM1_MODEL11_DOSE_CALIBRATOR_HH
#define MDSIM1_MODEL11_DOSE_CALIBRATOR_HH

#include "G4SystemOfUnits.hh"
#include "globals.hh"

class Model11Digit;

struct Model11CalibrationParameters {
    G4double doseCalibrationFactor = 1.0e-12 * gray;
    G4double doseCalibrationFactorError = 0.;
};

struct Model11DoseCalibrationResult {
    G4double estimatedDoseToWater = 0.;
    G4double calibrationError = 0.;
};

class Model11DoseCalibrator {
public:
    explicit Model11DoseCalibrator(const Model11CalibrationParameters& parameters);

    Model11DoseCalibrationResult Calibrate(const Model11Digit& digit) const;

private:
    Model11CalibrationParameters fParameters;
};

#endif // MDSIM1_MODEL11_DOSE_CALIBRATOR_HH
