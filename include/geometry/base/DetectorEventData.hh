#ifndef MD1_DETECTOR_EVENT_DATA_H
#define MD1_DETECTOR_EVENT_DATA_H

#include <cmath>
#include <vector>

#include "globals.hh"

struct CalibratedDoseToWaterData {
    G4double value = 0.;
    G4double calibrationError = 0.;

    void Add(G4double nominalValue, G4double absoluteCalibrationError) {
        value += nominalValue;
        calibrationError += std::abs(absoluteCalibrationError);
    }
};

struct DetectorInstanceEventData {
    G4String summaryLabel;
    G4double totalEdep = 0.;
    G4double totalCollectedCharge = 0.;
    G4double totalDose = 0.;
    CalibratedDoseToWaterData estimatedDoseToWater;
};

struct DetectorEventData {
    G4double totalEdep = 0.;
    G4double totalCollectedCharge = 0.;
    G4double totalDose = 0.;
    G4double totalEstimatedDoseToWater = 0.;
    G4bool hasSignal = false;
    std::vector<DetectorInstanceEventData> instanceData;
};

#endif // MD1_DETECTOR_EVENT_DATA_H
