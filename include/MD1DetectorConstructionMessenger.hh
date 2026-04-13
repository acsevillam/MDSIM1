/*
 *
 * Geant4 MultiDetector Simulation v1
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

#ifndef MD1_DETECTOR_CONSTRUCTION_MESSENGER_H
#define MD1_DETECTOR_CONSTRUCTION_MESSENGER_H 1

#include "G4UIcmdWithAString.hh"
#include "G4UImessenger.hh"

namespace MD1 {

class MD1DetectorConstruction;

class MD1DetectorConstructionMessenger : public G4UImessenger {
public:
    explicit MD1DetectorConstructionMessenger(MD1DetectorConstruction* detectorConstruction);
    ~MD1DetectorConstructionMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    MD1DetectorConstruction* fDetectorConstruction;
    G4UIcmdWithAString* fSetDefaultWaterPhantomCmd;
};

} // namespace MD1

#endif // MD1_DETECTOR_CONSTRUCTION_MESSENGER_H
