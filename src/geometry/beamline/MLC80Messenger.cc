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

#include "geometry/beamline/MLC80Messenger.hh"

#include "geometry/beamline/MLC80.hh"
#include "G4VVisCommand.hh"

using namespace std;

MLC80Messenger::MLC80Messenger(MLC80* aMLC80)
{ 

  fMLC80 = aMLC80;

}
//****************************************************************************
void MLC80Messenger::SetNewValue(G4UIcommand* command,G4String newValue)
{ 

}
