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

// Geant4 Headers
#include "G4UImessenger.hh"
#include "G4UIcmdWithAnInteger.hh"

namespace MD1 {

class MD1Control ;

class MD1ControlMessenger: public G4UImessenger {

public:

	MD1ControlMessenger(MD1Control* aMD1Control );
	virtual ~MD1ControlMessenger();

	void SetNewValue(G4UIcommand* command, G4String aValue) ;

private:

	MD1Control*								fMD1Control ;
	G4UIcmdWithAnInteger* fSetPrimaryGeneratorTypeCmd; ///< Command to select the primary generator backend.


};

}

#endif // MD1ControlMessenger_h
