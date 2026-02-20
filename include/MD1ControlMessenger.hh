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

#ifndef MD1ControlMessenger_h
#define MD1ControlMessenger_h 1

// C++ Headers
#include <vector>
#include <map>

// Geant4 Headers
#include "G4UImessenger.hh"
#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

// MD1 Headers
#include "MD1Control.hh"

using namespace std;

class G4UIdirectory ;
class G4UIcmdWithAString ;
class G4UIcmdWithABool ;
class G4UIcmdWithADoubleAndUnit ;

namespace MD1 {

class MD1Control ;

class MD1ControlMessenger: public G4UImessenger {

public:

	MD1ControlMessenger(MD1Control* aMD1Control );
	virtual ~MD1ControlMessenger();

	void SetNewValue(G4UIcommand* command, G4String aValue) ;

private:

	MD1Control*								fMD1Control ;
	G4UIcmdWithAString* fSetPhspFileNameCmd; ///< Command to set the phase space filename
	G4UIcmdWithAnInteger* fSetPrimaryGeneratorTypeCmd; ///< Command to set the phase space filename


};

}

#endif // MD1ControlMessenger_h