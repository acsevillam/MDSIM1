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

// MultiDetector Headers
#include "MD1RunActionMessenger.hh"

using namespace std;

namespace MD1 {

MD1RunActionMessenger::MD1RunActionMessenger(MD1RunAction* aRunAction ){

	fRunAction = aRunAction ;

	fUIDirectoryCollection["MD1Run"] = new G4UIdirectory("/MultiDetector1/run/") ;
	fUIDirectoryCollection["MD1Run"] -> SetGuidance("Run parameters commands") ;

	fUIcmdWithAIntCollection["SetMU"] = new G4UIcmdWithAnInteger("/MultiDetector1/run/SetMU",this) ;
	fUIcmdWithAIntCollection["SetMU"] -> SetGuidance("Set monitor units");
	fUIcmdWithAIntCollection["SetMU"] -> SetParameterName("Radius",false);
	fUIcmdWithAIntCollection["SetMU"] -> AvailableForStates(G4State_PreInit, G4State_Idle);

}

void MD1RunActionMessenger::SetNewValue(G4UIcommand* command,G4String aValue ){

	if( command == fUIcmdWithAIntCollection["SetMU"] ) { fRunAction->SetMU(fUIcmdWithAIntCollection["SetMU"]->GetNewIntValue(aValue)); }
}


} // namespace MD1
