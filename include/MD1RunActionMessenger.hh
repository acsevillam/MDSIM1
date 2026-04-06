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

#ifndef MD1_RUN_ACTION_MESSENGER_H
#define MD1_RUN_ACTION_MESSENGER_H

// Geant4 Headers
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UImessenger.hh"
class G4UIcmdWithADouble ;
class G4UIcmdWithAnInteger ;
#include "globals.hh"

namespace MD1 {

class MD1RunAction;

class MD1RunActionMessenger : public G4UImessenger {
public:
    explicit MD1RunActionMessenger(MD1RunAction* runAction);
    ~MD1RunActionMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String value) override;

private:
    MD1RunAction* fRunAction;
    G4UIdirectory* fRunDirectory = nullptr;
    G4UIcmdWithAnInteger* fSetMUCmd = nullptr;
    G4UIcmdWithADouble* fSetScaleFactorMUCmd = nullptr;
    G4UIcmdWithADouble* fSetScaleFactorMUErrorCmd = nullptr;
};

} // namespace MD1

#endif // MD1_RUN_ACTION_MESSENGER_H
