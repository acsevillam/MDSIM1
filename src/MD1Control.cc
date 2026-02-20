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

// Geant4 Headers
#include "G4UImanager.hh"

// MD1 Headers
#include "MD1Control.hh"
#include "G4SystemOfUnits.hh"

using namespace std ;

namespace MD1 {

// Define Static Variables
MD1Control* MD1Control::instance = NULL;

MD1Control::MD1Control() {
	fMD1ControlMessenger = new MD1ControlMessenger(this) ;
	fTrackingAction = NULL ;
	fSteppingAction = NULL ;
	fSensitiveDetectorAction = NULL ;
	fPhspFileName = "beam/Varian_TrueBeam6MV_01";
}

MD1Control::~MD1Control()
{ }

MD1Control* MD1Control::GetInstance() {

	if (instance == NULL) instance =  new MD1Control() ;
	return instance ;

}

void MD1Control::Kill() {

	if(instance!=NULL){
		delete instance ;
		instance = NULL ;
	}
}

void MD1Control::Setup(int argc,char** argv){

	if (argc>2) {  // batch mode
		// command line contains name of the macro to execute
		G4String command = "/control/execute " ;
		G4String fileName = argv[1] ;
		G4UImanager::GetUIpointer()->ApplyCommand(command+fileName) ;
	}

}

}