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

#pragma once

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"

class DetectorCylinder;

class DetectorCylinderMessenger : public G4UImessenger {
public:
    explicit DetectorCylinderMessenger(DetectorCylinder* detectorCylinder);
    ~DetectorCylinderMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    void ConvertToDoubleTrio(const G4String& paramString,
                             G4double& xval,
                             G4double& yval,
                             G4double& zval);

    DetectorCylinder* fDetectorCylinder;
    G4int fCurrentDetectorID;

    G4UIcmdWithADoubleAndUnit* fSetCylinderRadiusCmd;
    G4UIcmdWithADoubleAndUnit* fSetCylinderHeightCmd;
    G4UIcmdWithAString* fSetCylinderMaterialCmd;
    G4UIcmdWithADoubleAndUnit* fSetEnvelopeThicknessCmd;
    G4UIcmdWithAString* fSetEnvelopeMaterialCmd;
    G4UIcmdWithABool* fSetSplitAtInterfaceCmd;
    G4UIcmdWithADouble* fSetCalibrationFactorCmd;
    G4UIcmdWithADouble* fSetCalibrationFactorErrorCmd;
    G4UIcmdWithAnInteger* fDetectorIDCmd;
    G4UIcmdWith3VectorAndUnit* fTranslateCmd;
    G4UIcmdWith3VectorAndUnit* fTranslateToCmd;
    G4UIcmdWithADoubleAndUnit* fRotateXCmd;
    G4UIcmdWithADoubleAndUnit* fRotateYCmd;
    G4UIcmdWithADoubleAndUnit* fRotateZCmd;
    G4UIcommand* fRotateToCmd;
    G4UIcommand* fAddGeometryToCmd;
    G4UIcmdWithAnInteger* fRemoveGeometryCmd;
};
