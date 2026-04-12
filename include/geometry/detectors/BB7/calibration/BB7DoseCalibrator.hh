#ifndef MDSIM1_BB7_DOSE_CALIBRATOR_HH
#define MDSIM1_BB7_DOSE_CALIBRATOR_HH

#include "G4SystemOfUnits.hh"
#include "globals.hh"

class BB7Digit;

struct BB7CalibrationParameters {
    G4double calibrationFactor = 1.0 * gray / coulomb;
    G4double calibrationFactorError = 0.01 * gray / coulomb;
};

struct BB7DoseCalibrationResult {
    G4double estimatedDoseToWater = 0.;
    G4double calibrationError = 0.;
};

class BB7DoseCalibrator {
public:
    explicit BB7DoseCalibrator(const BB7CalibrationParameters& parameters);

    BB7DoseCalibrationResult Calibrate(const BB7Digit& digit) const;

private:
    BB7CalibrationParameters fParameters;
};

#endif // MDSIM1_BB7_DOSE_CALIBRATOR_HH
