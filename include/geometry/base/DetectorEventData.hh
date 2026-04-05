#ifndef MD1_DETECTOR_EVENT_DATA_H
#define MD1_DETECTOR_EVENT_DATA_H

#include "globals.hh"

struct DetectorEventData {
    G4String detectorName;
    G4double totalEdep = 0.;
    G4double totalCollectedCharge = 0.;
    G4double totalDose = 0.;
    G4double totalEstimatedDoseToWater = 0.;
    G4bool hasSignal = false;
};

#endif // MD1_DETECTOR_EVENT_DATA_H
