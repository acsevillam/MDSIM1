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

#include <sstream>

#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/phantoms/PhantomWaterTube.hh"
#include "geometry/phantoms/PhantomWaterTubeMessenger.hh"

PhantomWaterTubeMessenger::PhantomWaterTubeMessenger(PhantomWaterTube* phantom)
    : G4UImessenger(), fPhantom(phantom), fCurrentPhantomID(0) {
    fSetRadiusCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/watertube/setRadius", this);
    fSetRadiusCmd->SetGuidance("Set the radius of the water tube phantom.");
    fSetRadiusCmd->SetParameterName("radius", false);
    fSetRadiusCmd->SetUnitCategory("Length");
    fSetRadiusCmd->SetRange("radius>0.");
    fSetRadiusCmd->AvailableForStates(G4State_PreInit);

    fSetHeightCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/watertube/setHeight", this);
    fSetHeightCmd->SetGuidance("Set the full height of the water tube phantom.");
    fSetHeightCmd->SetParameterName("height", false);
    fSetHeightCmd->SetUnitCategory("Length");
    fSetHeightCmd->SetRange("height>0.");
    fSetHeightCmd->AvailableForStates(G4State_PreInit);

    fPhantomIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/phantoms/watertube/phantomID", this);
    fPhantomIDCmd->SetGuidance("Select phantom ID.");
    fPhantomIDCmd->SetParameterName("phantomID", false);
    fPhantomIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/phantoms/watertube/translate", this);
    fTranslateCmd->SetGuidance("Translate the phantom by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/phantoms/watertube/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the phantom to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/watertube/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the phantom around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/watertube/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the phantom around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/watertube/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the phantom around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/phantoms/watertube/rotateTo", this);
    fRotateToCmd->SetGuidance("Set the rotation angles of the phantom.");
    auto* thetaParam = new G4UIparameter("theta", 'd', true);
    thetaParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(thetaParam);
    auto* phiParam = new G4UIparameter("phi", 'd', true);
    phiParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(phiParam);
    auto* psiParam = new G4UIparameter("psi", 'd', true);
    psiParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(psiParam);
    auto* unitParam = new G4UIparameter("unit", 's', true);
    unitParam->SetDefaultValue("deg");
    fRotateToCmd->SetParameter(unitParam);
    fRotateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/phantoms/watertube/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the phantom.");
    auto* volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto* copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/phantoms/watertube/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by phantom ID.");
    fRemoveGeometryCmd->SetParameterName("phantomID", false);
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

PhantomWaterTubeMessenger::~PhantomWaterTubeMessenger() {
    delete fSetRadiusCmd;
    delete fSetHeightCmd;
    delete fPhantomIDCmd;
    delete fTranslateCmd;
    delete fTranslateToCmd;
    delete fRotateXCmd;
    delete fRotateYCmd;
    delete fRotateZCmd;
    delete fRotateToCmd;
    delete fAddGeometryToCmd;
    delete fRemoveGeometryCmd;
}

void PhantomWaterTubeMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetRadiusCmd) {
        fPhantom->SetWaterTubeRadius(fSetRadiusCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetHeightCmd) {
        fPhantom->SetWaterTubeHeight(fSetHeightCmd->GetNewDoubleValue(newValue));
    } else if (command == fTranslateCmd) {
        const G4ThreeVector position = fTranslateCmd->GetNew3VectorValue(newValue);
        fPhantom->Translate(fCurrentPhantomID, position);
    } else if (command == fTranslateToCmd) {
        const G4ThreeVector position = fTranslateToCmd->GetNew3VectorValue(newValue);
        fPhantom->TranslateTo(fCurrentPhantomID, position);
    } else if (command == fRotateXCmd) {
        const G4double angle = fRotateXCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateX(fCurrentPhantomID, angle);
    } else if (command == fRotateYCmd) {
        const G4double angle = fRotateYCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateY(fCurrentPhantomID, angle);
    } else if (command == fRotateZCmd) {
        const G4double angle = fRotateZCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateZ(fCurrentPhantomID, angle);
    } else if (command == fRotateToCmd) {
        G4double theta = 0.;
        G4double phi = 0.;
        G4double psi = 0.;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fPhantom->RotateTo(fCurrentPhantomID, theta, phi, psi);
    } else if (command == fPhantomIDCmd) {
        fCurrentPhantomID = fPhantomIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream stream(newValue);
        G4String logicalVolumeName;
        G4int copyNo = 0;
        stream >> logicalVolumeName >> copyNo;
        fPhantom->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fPhantom->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
    }
}

void PhantomWaterTubeMessenger::ConvertToDoubleTrio(const G4String& paramString,
                                                    G4double& xval,
                                                    G4double& yval,
                                                    G4double& zval) {
    G4double x = 0.;
    G4double y = 0.;
    G4double z = 0.;
    char units[30];
    std::istringstream stream(paramString);
    stream >> x >> y >> z >> units;
    const G4String unit = units;
    xval = x * G4UIcommand::ValueOf(unit);
    yval = y * G4UIcommand::ValueOf(unit);
    zval = z * G4UIcommand::ValueOf(unit);
}
