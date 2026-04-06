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

#include "MD1RunActionMessenger.hh"
#include "MD1RunAction.hh"

namespace MD1 {

MD1RunActionMessenger::MD1RunActionMessenger(MD1RunAction* runAction)
    : fRunAction(runAction) {
    fRunDirectory = new G4UIdirectory("/MultiDetector1/run/");
    fRunDirectory->SetGuidance("Run parameters commands");

    fSetMUCmd = new G4UIcmdWithAnInteger("/MultiDetector1/run/SetMU", this);
    fSetMUCmd->SetGuidance("Set monitor units.");
    fSetMUCmd->SetParameterName("MU", false);
    fSetMUCmd->SetRange("MU>0");
    fSetMUCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetScaleFactorMUCmd = new G4UIcmdWithADouble("/MultiDetector1/run/SetScaleFactorMU", this);
    fSetScaleFactorMUCmd->SetGuidance("Set the conversion factor from per-event results to per-MU results.");
    fSetScaleFactorMUCmd->SetParameterName("ScaleFactorMU", false);
    fSetScaleFactorMUCmd->SetRange("ScaleFactorMU>0.");
    fSetScaleFactorMUCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetScaleFactorMUErrorCmd = new G4UIcmdWithADouble("/MultiDetector1/run/SetScaleFactorMUError", this);
    fSetScaleFactorMUErrorCmd->SetGuidance("Set the absolute uncertainty of the per-MU scaling factor.");
    fSetScaleFactorMUErrorCmd->SetParameterName("ScaleFactorMUError", false);
    fSetScaleFactorMUErrorCmd->SetRange("ScaleFactorMUError>=0.");
    fSetScaleFactorMUErrorCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MD1RunActionMessenger::~MD1RunActionMessenger() {
    delete fSetScaleFactorMUErrorCmd;
    delete fSetScaleFactorMUCmd;
    delete fSetMUCmd;
    delete fRunDirectory;
}

void MD1RunActionMessenger::SetNewValue(G4UIcommand* command, G4String value) {
    if (command == fSetMUCmd) {
        fRunAction->SetMU(fSetMUCmd->GetNewIntValue(value));
    } else if (command == fSetScaleFactorMUCmd) {
        fRunAction->SetScaleFactorMU(fSetScaleFactorMUCmd->GetNewDoubleValue(value));
    } else if (command == fSetScaleFactorMUErrorCmd) {
        fRunAction->SetScaleFactorMUError(fSetScaleFactorMUErrorCmd->GetNewDoubleValue(value));
    }
}

} // namespace MD1
