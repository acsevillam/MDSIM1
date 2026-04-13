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

// Geant4 Headers
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "geometry/phantoms/PhantomWaterBoxMessenger.hh"
#include "geometry/phantoms/PhantomWaterBox.hh"

PhantomWaterBoxMessenger::PhantomWaterBoxMessenger(PhantomWaterBox* phantom)
    : G4UImessenger(), fPhantom(phantom), fCurrentPhantomID(0) {

    fSetSideCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/waterbox/setSide", this);
    fSetSideCmd->SetGuidance("Set the full cubic side of the water box phantom.");
    fSetSideCmd->SetParameterName("side", false);
    fSetSideCmd->SetUnitCategory("Length");
    fSetSideCmd->SetRange("side>0.");
    fSetSideCmd->AvailableForStates(G4State_PreInit);

    fSetSizeCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/phantoms/waterbox/setSize", this);
    fSetSizeCmd->SetGuidance("Set the full water box size along X, Y and Z.");
    fSetSizeCmd->SetParameterName("sizeX", "sizeY", "sizeZ", false, false);
    fSetSizeCmd->SetDefaultUnit("cm");
    fSetSizeCmd->SetUnitCategory("Length");
    fSetSizeCmd->AvailableForStates(G4State_PreInit);
    
    fPhantomIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/phantoms/waterbox/phantomID", this);
    fPhantomIDCmd->SetGuidance("Select phantom ID.");
    fPhantomIDCmd->SetParameterName("phantomID", false);
    fPhantomIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/phantoms/waterbox/translate", this);
    fTranslateCmd->SetGuidance("Translate the phantom by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/phantoms/waterbox/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the phantom to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/waterbox/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the phantom around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/waterbox/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the phantom around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/phantoms/waterbox/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the phantom around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/phantoms/waterbox/rotateTo", this);
    fRotateToCmd->SetGuidance("Set the rotation angles of the phantom.");
    auto thetaParam = new G4UIparameter("theta", 'd', true);
    thetaParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(thetaParam);
    auto phiParam = new G4UIparameter("phi", 'd', true);
    phiParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(phiParam);
    auto psiParam = new G4UIparameter("psi", 'd', true);
    psiParam->SetDefaultValue(0.);
    fRotateToCmd->SetParameter(psiParam);
    auto unitParam = new G4UIparameter("unit", 's', true);
    unitParam->SetDefaultValue("deg");
    fRotateToCmd->SetParameter(unitParam);
    fRotateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/phantoms/waterbox/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the phantom.");
    auto volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/phantoms/waterbox/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by phantom ID.");
    fRemoveGeometryCmd->SetParameterName("phantomID", false);
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

PhantomWaterBoxMessenger::~PhantomWaterBoxMessenger() {
    delete fSetSideCmd;
    delete fSetSizeCmd;
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

void PhantomWaterBoxMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetSideCmd) {
        fPhantom->SetWaterBoxSide(fSetSideCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSizeCmd) {
        fPhantom->SetWaterBoxSize(fSetSizeCmd->GetNew3VectorValue(newValue));
    } else if (command == fTranslateCmd) {
        G4ThreeVector position = fTranslateCmd->GetNew3VectorValue(newValue);
        fPhantom->Translate(fCurrentPhantomID, position);
    } else if (command == fTranslateToCmd) {
        G4ThreeVector position = fTranslateToCmd->GetNew3VectorValue(newValue);
        fPhantom->TranslateTo(fCurrentPhantomID, position);
    } else if (command == fRotateXCmd) {
        G4double angle = fRotateXCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateX(fCurrentPhantomID, angle);
    } else if (command == fRotateYCmd) {
        G4double angle = fRotateYCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateY(fCurrentPhantomID, angle);
    } else if (command == fRotateZCmd) {
        G4double angle = fRotateZCmd->GetNewDoubleValue(newValue);
        fPhantom->RotateZ(fCurrentPhantomID, angle);
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fPhantom->RotateTo(fCurrentPhantomID, theta, phi, psi);
    } else if (command == fPhantomIDCmd) {
        fCurrentPhantomID = fPhantomIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        fPhantom->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fPhantom->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
    }
}

void PhantomWaterBoxMessenger::ConvertToDoubleTrio(const G4String& paramString, G4double& xval, G4double& yval, G4double& zval) {
    G4double x, y, z;
    char unts[30];
    std::istringstream is(paramString);
    is >> x >> y >> z >> unts;
    G4String unt = unts;
    xval = x * G4UIcommand::ValueOf(unt);
    yval = y * G4UIcommand::ValueOf(unt);
    zval = z * G4UIcommand::ValueOf(unt);
}
