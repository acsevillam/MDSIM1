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
#define MD1_DETECTOR_CONSTRUCTION_MESSENGER_H

// Standard Headers
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
#include "G4UIcmdWithoutParameter.hh"

// MultiDetector Headers
#include "MD1RunAction.hh"

typedef std::map <G4String,G4UIdirectory*>         			UIDirectoryCollection ;
typedef std::map <G4String,G4UIcmdWithAString*>    			UIcmdWithAStringCollection ;
typedef std::map <G4String,G4UIcmdWithABool*>    			UIcmdWithABoolCollection ;
typedef std::map <G4String,G4UIcmdWithADoubleAndUnit*>    	UIcmdWithADoubleAndUnitCollection ;
typedef std::map <G4String,G4UIcmdWithAnInteger*>    		UIcmdWithAIntCollection ;
typedef std::map <G4String,G4UIcmdWithoutParameter*>    	UIcmdWithoutParameter ;

class G4UIdirectory ;
class G4UIcmdWithAString ;
class G4UIcmdWithABool ;
class G4UIcmdWithADoubleAndUnit ;
class G4UIcmdWithAnInteger ;
class G4UIcmdWithoutParameter ;

/// Detector construction class to define materials and geometry.
namespace MD1 {

class MD1RunAction;

class MD1RunActionMessenger : public G4UImessenger {
public:
    MD1RunActionMessenger(MD1RunAction* aRunAction );
    ~MD1RunActionMessenger() override = default;

	void SetNewValue(G4UIcommand* command, G4String aValue) override;

private:

	MD1RunAction*				            fRunAction ;

	UIDirectoryCollection					fUIDirectoryCollection ;
	UIcmdWithAStringCollection				fUIcmdWithAStringCollection ;
	UIcmdWithABoolCollection				fUIcmdWithABoolCollection ;
	UIcmdWithADoubleAndUnitCollection		fUIcmdWithADoubleAndUnitCollection;
	UIcmdWithAIntCollection					fUIcmdWithAIntCollection;
	UIcmdWithoutParameter					fUIcmdWithoutParameter;
};

} // namespace MD1

#endif // MD1_DETECTOR_CONSTRUCTION_MESSENGER_H
