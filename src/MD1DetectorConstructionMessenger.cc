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

#include "MD1DetectorConstructionMessenger.hh"

#include "MD1DetectorConstruction.hh"

namespace MD1 {

MD1DetectorConstructionMessenger::MD1DetectorConstructionMessenger(
    MD1DetectorConstruction* detectorConstruction)
    : fDetectorConstruction(detectorConstruction), fSetDefaultWaterPhantomCmd(nullptr) {
    fSetDefaultWaterPhantomCmd =
        new G4UIcmdWithAString("/MultiDetector1/phantoms/setDefault", this);
    fSetDefaultWaterPhantomCmd->SetGuidance(
        "Select the default water phantom shape used during detector construction.");
    fSetDefaultWaterPhantomCmd->SetGuidance("Available options: waterBox, waterTube.");
    fSetDefaultWaterPhantomCmd->SetParameterName("phantomType", false);
    fSetDefaultWaterPhantomCmd->SetCandidates("waterBox waterTube");
    fSetDefaultWaterPhantomCmd->AvailableForStates(G4State_PreInit);
}

MD1DetectorConstructionMessenger::~MD1DetectorConstructionMessenger() {
    delete fSetDefaultWaterPhantomCmd;
}

void MD1DetectorConstructionMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetDefaultWaterPhantomCmd) {
        fDetectorConstruction->SetDefaultWaterPhantomType(newValue);
    }
}

} // namespace MD1
