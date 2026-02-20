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

namespace MD1 {

MD1ControlMessenger::MD1ControlMessenger(MD1Control* aMD1Control) {

	fMD1Control = aMD1Control ;

    fSetPrimaryGeneratorTypeCmd = new G4UIcmdWithAnInteger("/MultiDetector1/control/SetPrimaryGeneratorType", this);
    fSetPrimaryGeneratorTypeCmd->SetGuidance("Set primary generator type");
    fSetPrimaryGeneratorTypeCmd->SetParameterName("PrimaryGeneratorType", false);
    fSetPrimaryGeneratorTypeCmd->AvailableForStates(G4State_PreInit);

    fSetPhspFileNameCmd = new G4UIcmdWithAString("/MultiDetector1/control/SetPhspFileName", this);
    fSetPhspFileNameCmd->SetGuidance("Set the phase space filename");
    fSetPhspFileNameCmd->SetParameterName("PhspFileName", false);
    fSetPhspFileNameCmd->AvailableForStates(G4State_PreInit);
}

MD1ControlMessenger::~MD1ControlMessenger() {
    delete fSetPhspFileNameCmd;  
}

void MD1ControlMessenger::SetNewValue(G4UIcommand* command,G4String newValue ){
    if (command == fSetPhspFileNameCmd) {
        fMD1Control->SetPhspFileName(newValue);
    } 
    if (command == fSetPrimaryGeneratorTypeCmd) {
        fMD1Control->SetPrimaryGeneratorType(fSetPrimaryGeneratorTypeCmd->GetNewIntValue(newValue));
    } 
}

}
