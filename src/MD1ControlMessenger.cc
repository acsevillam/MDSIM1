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

#include "MD1ControlMessenger.hh"

#include "MD1Control.hh"
namespace MD1 {

MD1ControlMessenger::MD1ControlMessenger(MD1Control* aMD1Control) {

	fMD1Control = aMD1Control ;

    fControlDirectory = new G4UIdirectory("/MultiDetector1/control/");
    fControlDirectory->SetGuidance("Global MDSIM1 control and view commands.");

    fSetPrimaryGeneratorTypeCmd = new G4UIcmdWithAnInteger("/MultiDetector1/control/SetPrimaryGeneratorType", this);
    fSetPrimaryGeneratorTypeCmd->SetGuidance(
        "Set primary generator type: 1 for IAEA phase-space, 2 for GPS.");
    fSetPrimaryGeneratorTypeCmd->SetParameterName("PrimaryGeneratorType", false);
    fSetPrimaryGeneratorTypeCmd->AvailableForStates(G4State_PreInit);

    fCenterViewOnZAxisCmd = new G4UIcmdWithoutParameter("/MultiDetector1/control/CenterViewOnZAxis", this);
    fCenterViewOnZAxisCmd->SetGuidance(
        "Center the current target point on the global Z axis, keeping the current Z depth.");
    fCenterViewOnZAxisCmd->AvailableForStates(G4State_Idle);

    fToggleFocusAxesCmd = new G4UIcmdWithoutParameter("/MultiDetector1/control/ToggleFocusAxes", this);
    fToggleFocusAxesCmd->SetGuidance(
        "Show or hide world axes centered on the current viewer focus point.");
    fToggleFocusAxesCmd->AvailableForStates(G4State_Idle);

}

MD1ControlMessenger::~MD1ControlMessenger() {
    delete fToggleFocusAxesCmd;
    delete fCenterViewOnZAxisCmd;
    delete fSetPrimaryGeneratorTypeCmd;
    delete fControlDirectory;
}

void MD1ControlMessenger::SetNewValue(G4UIcommand* command,G4String newValue ){
    if (command == fSetPrimaryGeneratorTypeCmd) {
        fMD1Control->SetPrimaryGeneratorType(fSetPrimaryGeneratorTypeCmd->GetNewIntValue(newValue));
    } else if (command == fCenterViewOnZAxisCmd) {
        fMD1Control->CenterViewOnZAxis();
    } else if (command == fToggleFocusAxesCmd) {
        fMD1Control->ToggleFocusAxes();
    }
}

}
