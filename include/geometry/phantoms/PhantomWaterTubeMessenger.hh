/*
 *
 * Geant4 MultiDetector Simulation
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

#ifndef PHANTOM_WATER_TUBE_MESSENGER_H
#define PHANTOM_WATER_TUBE_MESSENGER_H

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"

#include "geometry/phantoms/PhantomWaterTube.hh"

class PhantomWaterTube;

/**
 * @class PhantomWaterTubeMessenger
 * @brief Messenger class to handle UI commands for PhantomWaterTube.
 */
class PhantomWaterTubeMessenger : public G4UImessenger {
public:
    explicit PhantomWaterTubeMessenger(PhantomWaterTube* phantom);

    ~PhantomWaterTubeMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    void ConvertToDoubleTrio(const G4String& paramString,
                             G4double& xval,
                             G4double& yval,
                             G4double& zval);

    PhantomWaterTube* fPhantom;
    G4int fCurrentPhantomID;

    G4UIcmdWithADoubleAndUnit* fSetRadiusCmd;
    G4UIcmdWithADoubleAndUnit* fSetHeightCmd;
    G4UIcmdWithAnInteger* fPhantomIDCmd;
    G4UIcmdWith3VectorAndUnit* fTranslateCmd;
    G4UIcmdWith3VectorAndUnit* fTranslateToCmd;
    G4UIcmdWithADoubleAndUnit* fRotateXCmd;
    G4UIcmdWithADoubleAndUnit* fRotateYCmd;
    G4UIcmdWithADoubleAndUnit* fRotateZCmd;
    G4UIcommand* fRotateToCmd;
    G4UIcommand* fAddGeometryToCmd;
    G4UIcmdWithAnInteger* fRemoveGeometryCmd;
};

#endif // PHANTOM_WATER_TUBE_MESSENGER_H
