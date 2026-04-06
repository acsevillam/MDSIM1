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

// C++ Headers
#include <sstream>

// Geant4 Headers
#include "G4UIparameter.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "geometry/base/DetectorRegistry.hh"
#include "geometry/detectors/cube/messenger/DetectorCubeMessenger.hh"
#include "geometry/detectors/cube/geometry/DetectorCube.hh"

DetectorCubeMessenger::DetectorCubeMessenger(DetectorCube* detectorCube)
    : G4UImessenger(), fDetectorCube(detectorCube), fCurrentDetectorID(0) {
    fSetCubeSideCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/cube/setSide", this);
    fSetCubeSideCmd->SetGuidance("Set the side length of the cube detector.");
    fSetCubeSideCmd->SetParameterName("CubeSide", false);
    fSetCubeSideCmd->SetUnitCategory("Length");
    fSetCubeSideCmd->SetRange("CubeSide>0.");
    fSetCubeSideCmd->AvailableForStates(G4State_PreInit);

    fSetCubeMaterialCmd = new G4UIcmdWithAString("/MultiDetector1/detectors/cube/setMaterial", this);
    fSetCubeMaterialCmd->SetGuidance("Set the NIST material name of the cube detector.");
    fSetCubeMaterialCmd->SetParameterName("CubeMaterial", false);
    fSetCubeMaterialCmd->AvailableForStates(G4State_PreInit);

    fSetCalibrationFactorCmd = new G4UIcmdWithADouble("/MultiDetector1/detectors/cube/setCalibrationFactor", this);
    fSetCalibrationFactorCmd->SetGuidance("Set the experimental cube calibration factor in cGy/nC.");
    fSetCalibrationFactorCmd->SetParameterName("CalibrationFactor", false);
    fSetCalibrationFactorCmd->SetRange("CalibrationFactor>0.");
    fSetCalibrationFactorCmd->AvailableForStates(G4State_PreInit);

    fSetCalibrationFactorErrorCmd = new G4UIcmdWithADouble("/MultiDetector1/detectors/cube/setCalibrationFactorError", this);
    fSetCalibrationFactorErrorCmd->SetGuidance("Set the experimental cube calibration factor uncertainty in cGy/nC.");
    fSetCalibrationFactorErrorCmd->SetParameterName("CalibrationFactorError", false);
    fSetCalibrationFactorErrorCmd->SetRange("CalibrationFactorError>=0.");
    fSetCalibrationFactorErrorCmd->AvailableForStates(G4State_PreInit);

    fDetectorIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/cube/detectorID", this);
    fDetectorIDCmd->SetGuidance("Select detector ID.");
    fDetectorIDCmd->SetParameterName("detectorID", false);
    fDetectorIDCmd->SetRange("detectorID>=0");
    fDetectorIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/cube/translate", this);
    fTranslateCmd->SetGuidance("Translate the detector by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/cube/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the detector to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->SetUnitCategory("Length");
    fTranslateToCmd->AvailableForStates(G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/cube/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the detector around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/cube/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the detector around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/cube/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the detector around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/detectors/cube/rotateTo", this);
    fRotateToCmd->SetGuidance("Set the rotation angles of the detector.");
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
    fRotateToCmd->AvailableForStates(G4State_Idle);

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/detectors/cube/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the detector.");
    auto* volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto* copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/cube/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by detector ID.");
    fRemoveGeometryCmd->SetParameterName("detectorID", false);
    fRemoveGeometryCmd->SetRange("detectorID>=0");
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorCubeMessenger::~DetectorCubeMessenger() {
    delete fSetCubeSideCmd;
    delete fSetCubeMaterialCmd;
    delete fSetCalibrationFactorCmd;
    delete fSetCalibrationFactorErrorCmd;
    delete fDetectorIDCmd;
    delete fTranslateCmd;
    delete fTranslateToCmd;
    delete fRotateXCmd;
    delete fRotateYCmd;
    delete fRotateZCmd;
    delete fRotateToCmd;
    delete fAddGeometryToCmd;
    delete fRemoveGeometryCmd;
}

void DetectorCubeMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetCubeSideCmd) {
        fDetectorCube->SetCubeSide(fSetCubeSideCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetCubeMaterialCmd) {
        fDetectorCube->SetCubeMaterial(newValue);
    } else if (command == fSetCalibrationFactorCmd) {
        fDetectorCube->SetCalibrationFactor(
            fSetCalibrationFactorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) / (1e-9 * coulomb));
    } else if (command == fSetCalibrationFactorErrorCmd) {
        fDetectorCube->SetCalibrationFactorError(
            fSetCalibrationFactorErrorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) / (1e-9 * coulomb));
    } else if (command == fTranslateCmd) {
        fDetectorCube->Translate(fCurrentDetectorID, fTranslateCmd->GetNew3VectorValue(newValue));
    } else if (command == fTranslateToCmd) {
        fDetectorCube->TranslateTo(fCurrentDetectorID, fTranslateToCmd->GetNew3VectorValue(newValue));
    } else if (command == fRotateXCmd) {
        fDetectorCube->RotateX(fCurrentDetectorID, fRotateXCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateYCmd) {
        fDetectorCube->RotateY(fCurrentDetectorID, fRotateYCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateZCmd) {
        fDetectorCube->RotateZ(fCurrentDetectorID, fRotateZCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fDetectorCube->RotateTo(fCurrentDetectorID, theta, phi, psi);
    } else if (command == fDetectorIDCmd) {
        fCurrentDetectorID = fDetectorIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        DetectorRegistry::GetInstance()->EnableDetector("cube");
        fDetectorCube->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fDetectorCube->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
        if (!fDetectorCube->HasPlacementRequests()) {
            DetectorRegistry::GetInstance()->DisableDetector("cube");
        }
    }
}

void DetectorCubeMessenger::ConvertToDoubleTrio(const G4String& paramString,
                                                G4double& xval,
                                                G4double& yval,
                                                G4double& zval) {
    G4double x, y, z;
    char units[30];
    std::istringstream is(paramString);
    is >> x >> y >> z >> units;
    G4String unit = units;
    xval = x * G4UIcommand::ValueOf(unit);
    yval = y * G4UIcommand::ValueOf(unit);
    zval = z * G4UIcommand::ValueOf(unit);
}
