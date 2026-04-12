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

#include "MD1Control.hh"
#include "MD1ControlMessenger.hh"

#include <string>

#include "G4Exception.hh"

namespace MD1 {

// Define Static Variables
MD1Control* MD1Control::instance = nullptr;

MD1Control::MD1Control() {
	fMD1ControlMessenger = new MD1ControlMessenger(this);
}

MD1Control::~MD1Control()
{
	delete fMD1ControlMessenger;
	fMD1ControlMessenger = nullptr;
}

MD1Control* MD1Control::GetInstance() {

	if (instance == nullptr) instance =  new MD1Control();
	return instance ;

}

void MD1Control::Kill() {

	if(instance!=nullptr){
		delete instance ;
		instance = nullptr ;
	}
}

void MD1Control::SetPrimaryGeneratorType(G4int aPrimaryGeneratorType) {
	if (aPrimaryGeneratorType != 1 && aPrimaryGeneratorType != 2) {
		G4Exception("MD1Control::SetPrimaryGeneratorType",
		            "InvalidPrimaryGeneratorType",
		            FatalException,
		            ("Unsupported primary generator type " +
		             std::to_string(aPrimaryGeneratorType) +
		             ". Use 1 for IAEA phase-space or 2 for GPS.")
		                .c_str());
	}

	fPrimaryGeneratorType = aPrimaryGeneratorType;
}

}
