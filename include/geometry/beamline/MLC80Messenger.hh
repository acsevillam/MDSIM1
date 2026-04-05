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

#ifndef MLC80_MESSENGER_H
#define MLC80_MESSENGER_H 1

// Geant4 Headers
#include "G4UImessenger.hh"
#include "globals.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcommand.hh"

// MultiDetector Headers
#include "geometry/beamline/MLC80.hh"

typedef std::map <G4String,G4UIdirectory*>         			UIDirectoryCollection ;
typedef std::map <G4String,G4UIcmdWithAString*>    			UIcmdWithAStringCollection ;
typedef std::map <G4String,G4UIcmdWithABool*>    			  UIcmdWithABoolCollection ;
typedef std::map <G4String,G4UIcmdWithADoubleAndUnit*>  UIcmdWithADoubleAndUnitCollection ;
typedef std::map <G4String,G4UIcmdWithAnInteger*>    		UIcmdWithAIntCollection ;
typedef std::map <G4String,G4UIcmdWithoutParameter*>    UIcmdWithoutParameterCollection ;
typedef std::map <G4String,G4UIcmdWith3Vector*>    			UIcmdWith3VectorCollection ;
typedef std::map <G4String,G4UIcmdWith3VectorAndUnit*>  UIcmdWith3VectorAndUnitCollection ;
typedef std::map <G4String,G4UIcommand*>    				    UIcommandCollection ;

class G4UIdirectory ;
class G4UIcmdWithAString ;
class G4UIcmdWithABool ;
class G4UIcmdWithADoubleAndUnit ;
class G4UIcmdWithAnInteger ;
class G4UIcmdWithoutParameter ;
class G4UIcmdWith3Vector ;
class G4UIcmdWith3VectorAndUnit ;
class G4UIcommand ;

class MLC80;

class MLC80Messenger: public G4UImessenger
{
  public:
    MLC80Messenger(MLC80* );
   ~MLC80Messenger() override = default;
    
	void SetNewValue(G4UIcommand* command, G4String aValue) override;

  private:
    MLC80* fMLC80;
    UIDirectoryCollection					fUIDirectoryCollection ;
    UIcmdWithAStringCollection				fUIcmdWithAStringCollection ;
    UIcmdWithABoolCollection				fUIcmdWithABoolCollection ;
    UIcmdWithADoubleAndUnitCollection		fUIcmdWithADoubleAndUnitCollection;
    UIcmdWithAIntCollection					fUIcmdWithAIntCollection;
    UIcmdWithoutParameterCollection			fUIcmdWithoutParameterCollection;
    UIcmdWith3VectorCollection				fUIcmdWith3VectorCollection;
    UIcmdWith3VectorAndUnitCollection		fUIcmdWith3VectorAndUnitCollection;
    UIcommandCollection						fUIcommandCollection;
 };

#endif // MLC80_MESSENGER_H

