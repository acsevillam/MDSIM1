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
#include "geometry/base/DetectorRegistry.hh"
#include "geometry/detectors/BB7/messenger/DetectorDualBB7Messenger.hh"
#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"

DetectorDualBB7Messenger::DetectorDualBB7Messenger(DetectorDualBB7* detector)
    : G4UImessenger(), fDetector(detector), fCurrentDetectorID(0) {
    
    fDetectorIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/BB7/detectorID", this);
    fDetectorIDCmd->SetGuidance("Select detector ID.");
    fDetectorIDCmd->SetParameterName("detectorID", false);
    fDetectorIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/BB7/translate", this);
    fTranslateCmd->SetGuidance("Translate the detector by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/BB7/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the detector to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->AvailableForStates(G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/BB7/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the detector around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/BB7/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the detector around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/BB7/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the detector around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/detectors/BB7/rotateTo", this);
    fRotateToCmd->SetGuidance("Set the rotation angles of the detector.");
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
    fRotateToCmd->AvailableForStates(G4State_Idle);

    fSetCalibrationFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/BB7/setCalibrationFactor", this);
    fSetCalibrationFactorCmd->SetGuidance("Set the experimental BB7 calibration factor in cGy/nC.");
    fSetCalibrationFactorCmd->SetParameterName("CalibrationFactor", false);
    fSetCalibrationFactorCmd->SetRange("CalibrationFactor>0.");
    fSetCalibrationFactorCmd->AvailableForStates(G4State_PreInit);

    fSetCalibrationFactorErrorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/BB7/setCalibrationFactorError", this);
    fSetCalibrationFactorErrorCmd->SetGuidance(
        "Set the experimental BB7 calibration factor uncertainty in cGy/nC.");
    fSetCalibrationFactorErrorCmd->SetParameterName("CalibrationFactorError", false);
    fSetCalibrationFactorErrorCmd->SetRange("CalibrationFactorError>=0.");
    fSetCalibrationFactorErrorCmd->AvailableForStates(G4State_PreInit);

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/detectors/BB7/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the detector.");
    auto volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/BB7/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by detector ID.");
    fRemoveGeometryCmd->SetParameterName("detectorID", false);
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorDualBB7Messenger::~DetectorDualBB7Messenger() {
    delete fDetectorIDCmd;
    delete fTranslateCmd;
    delete fTranslateToCmd;
    delete fRotateXCmd;
    delete fRotateYCmd;
    delete fRotateZCmd;
    delete fRotateToCmd;
    delete fSetCalibrationFactorCmd;
    delete fSetCalibrationFactorErrorCmd;
    delete fAddGeometryToCmd;
    delete fRemoveGeometryCmd;
}

void DetectorDualBB7Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fTranslateCmd) {
        G4ThreeVector position = fTranslateCmd->GetNew3VectorValue(newValue);
        fDetector->Translate(fCurrentDetectorID, position);
    } else if (command == fTranslateToCmd) {
        G4ThreeVector position = fTranslateToCmd->GetNew3VectorValue(newValue);
        fDetector->TranslateTo(fCurrentDetectorID, position);
    } else if (command == fRotateXCmd) {
        G4double angle = fRotateXCmd->GetNewDoubleValue(newValue);
        fDetector->RotateX(fCurrentDetectorID, angle);
    } else if (command == fRotateYCmd) {
        G4double angle = fRotateYCmd->GetNewDoubleValue(newValue);
        fDetector->RotateY(fCurrentDetectorID, angle);
    } else if (command == fRotateZCmd) {
        G4double angle = fRotateZCmd->GetNewDoubleValue(newValue);
        fDetector->RotateZ(fCurrentDetectorID, angle);
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fDetector->RotateTo(fCurrentDetectorID, theta, phi, psi);
    } else if (command == fSetCalibrationFactorCmd) {
        fDetector->SetCalibrationFactor(
            fCurrentDetectorID,
            fSetCalibrationFactorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) /
                (1e-9 * coulomb));
    } else if (command == fSetCalibrationFactorErrorCmd) {
        fDetector->SetCalibrationFactorError(
            fCurrentDetectorID,
            fSetCalibrationFactorErrorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) /
                (1e-9 * coulomb));
    } else if (command == fDetectorIDCmd) {
        fCurrentDetectorID = fDetectorIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        DetectorRegistry::GetInstance()->EnableDetector("BB7");
        fDetector->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fDetector->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
        if (!fDetector->HasPlacementRequests()) {
            DetectorRegistry::GetInstance()->DisableDetector("BB7");
        }
    }
}

void DetectorDualBB7Messenger::ConvertToDoubleTrio(const G4String& paramString, G4double& xval, G4double& yval, G4double& zval) {
    G4double x, y, z;
    char unts[30];
    std::istringstream is(paramString);
    is >> x >> y >> z >> unts;
    G4String unt = unts;
    xval = x * G4UIcommand::ValueOf(unt);
    yval = y * G4UIcommand::ValueOf(unt);
    zval = z * G4UIcommand::ValueOf(unt);
}
