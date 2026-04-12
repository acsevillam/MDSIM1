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

#include "geometry/detectors/basic/sphere/messenger/DetectorSphereMessenger.hh"

#include <sstream>

#include "G4SystemOfUnits.hh"
#include "G4UIparameter.hh"

#include "geometry/base/DetectorRegistry.hh"
#include "geometry/detectors/basic/sphere/geometry/DetectorSphere.hh"

DetectorSphereMessenger::DetectorSphereMessenger(DetectorSphere* detectorSphere)
    : G4UImessenger(), fDetectorSphere(detectorSphere), fCurrentDetectorID(0) {
    fSetSphereRadiusCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/sphere/setRadius", this);
    fSetSphereRadiusCmd->SetGuidance("Set the radius of the sphere detector.");
    fSetSphereRadiusCmd->SetParameterName("SphereRadius", false);
    fSetSphereRadiusCmd->SetUnitCategory("Length");
    fSetSphereRadiusCmd->SetRange("SphereRadius>0.");
    fSetSphereRadiusCmd->AvailableForStates(G4State_PreInit);

    fSetSphereMaterialCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/sphere/setMaterial", this);
    fSetSphereMaterialCmd->SetGuidance("Set the NIST material name of the sphere detector.");
    fSetSphereMaterialCmd->SetParameterName("SphereMaterial", false);
    fSetSphereMaterialCmd->AvailableForStates(G4State_PreInit);

    fSetEnvelopeThicknessCmd = new G4UIcmdWithADoubleAndUnit(
        "/MultiDetector1/detectors/sphere/setEnvelopeThickness", this);
    fSetEnvelopeThicknessCmd->SetGuidance(
        "Set the envelope thickness around the sphere detector. Zero disables the envelope.");
    fSetEnvelopeThicknessCmd->SetParameterName("EnvelopeThickness", false);
    fSetEnvelopeThicknessCmd->SetUnitCategory("Length");
    fSetEnvelopeThicknessCmd->SetRange("EnvelopeThickness>=0.");
    fSetEnvelopeThicknessCmd->AvailableForStates(G4State_PreInit);

    fSetEnvelopeMaterialCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/sphere/setEnvelopeMaterial", this);
    fSetEnvelopeMaterialCmd->SetGuidance("Set the NIST material name of the sphere envelope.");
    fSetEnvelopeMaterialCmd->SetParameterName("EnvelopeMaterial", false);
    fSetEnvelopeMaterialCmd->AvailableForStates(G4State_PreInit);

    fSetSplitAtInterfaceCmd =
        new G4UIcmdWithABool("/MultiDetector1/detectors/sphere/setSplitAtInterface", this);
    fSetSplitAtInterfaceCmd->SetGuidance(
        "Enable automatic split at the WaterBox top interface when the sphere protrudes into air.");
    fSetSplitAtInterfaceCmd->SetParameterName("SplitAtInterface", false);
    fSetSplitAtInterfaceCmd->AvailableForStates(G4State_PreInit);

    fSetCalibrationFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/sphere/setCalibrationFactor", this);
    fSetCalibrationFactorCmd->SetGuidance("Set the experimental sphere calibration factor in cGy/nC.");
    fSetCalibrationFactorCmd->SetParameterName("CalibrationFactor", false);
    fSetCalibrationFactorCmd->SetRange("CalibrationFactor>0.");
    fSetCalibrationFactorCmd->AvailableForStates(G4State_PreInit);

    fSetCalibrationFactorErrorCmd = new G4UIcmdWithADouble(
        "/MultiDetector1/detectors/sphere/setCalibrationFactorError", this);
    fSetCalibrationFactorErrorCmd->SetGuidance(
        "Set the experimental sphere calibration factor uncertainty in cGy/nC.");
    fSetCalibrationFactorErrorCmd->SetParameterName("CalibrationFactorError", false);
    fSetCalibrationFactorErrorCmd->SetRange("CalibrationFactorError>=0.");
    fSetCalibrationFactorErrorCmd->AvailableForStates(G4State_PreInit);

    fDetectorIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/sphere/detectorID", this);
    fDetectorIDCmd->SetGuidance("Select detector ID.");
    fDetectorIDCmd->SetParameterName("detectorID", false);
    fDetectorIDCmd->SetRange("detectorID>=0");
    fDetectorIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/sphere/translate", this);
    fTranslateCmd->SetGuidance("Translate the detector by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateToCmd =
        new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/sphere/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the detector to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->SetUnitCategory("Length");
    fTranslateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/sphere/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the detector around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/sphere/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the detector around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/sphere/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the detector around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/detectors/sphere/rotateTo", this);
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
    fRotateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/detectors/sphere/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the detector.");
    auto* volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto* copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd =
        new G4UIcmdWithAnInteger("/MultiDetector1/detectors/sphere/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by detector ID.");
    fRemoveGeometryCmd->SetParameterName("detectorID", false);
    fRemoveGeometryCmd->SetRange("detectorID>=0");
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorSphereMessenger::~DetectorSphereMessenger() {
    delete fSetSphereRadiusCmd;
    delete fSetSphereMaterialCmd;
    delete fSetEnvelopeThicknessCmd;
    delete fSetEnvelopeMaterialCmd;
    delete fSetSplitAtInterfaceCmd;
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

void DetectorSphereMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetSphereRadiusCmd) {
        fDetectorSphere->SetSphereRadius(fSetSphereRadiusCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSphereMaterialCmd) {
        fDetectorSphere->SetSphereMaterial(newValue);
    } else if (command == fSetEnvelopeThicknessCmd) {
        fDetectorSphere->SetEnvelopeThickness(fSetEnvelopeThicknessCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetEnvelopeMaterialCmd) {
        fDetectorSphere->SetEnvelopeMaterial(newValue);
    } else if (command == fSetSplitAtInterfaceCmd) {
        fDetectorSphere->SetSplitAtInterface(fSetSplitAtInterfaceCmd->GetNewBoolValue(newValue));
    } else if (command == fSetCalibrationFactorCmd) {
        fDetectorSphere->SetCalibrationFactor(
            fSetCalibrationFactorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) / (1e-9 * coulomb));
    } else if (command == fSetCalibrationFactorErrorCmd) {
        fDetectorSphere->SetCalibrationFactorError(
            fSetCalibrationFactorErrorCmd->GetNewDoubleValue(newValue) * (1e-2 * gray) / (1e-9 * coulomb));
    } else if (command == fTranslateCmd) {
        fDetectorSphere->Translate(fCurrentDetectorID, fTranslateCmd->GetNew3VectorValue(newValue));
    } else if (command == fTranslateToCmd) {
        fDetectorSphere->TranslateTo(fCurrentDetectorID, fTranslateToCmd->GetNew3VectorValue(newValue));
    } else if (command == fRotateXCmd) {
        fDetectorSphere->RotateX(fCurrentDetectorID, fRotateXCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateYCmd) {
        fDetectorSphere->RotateY(fCurrentDetectorID, fRotateYCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateZCmd) {
        fDetectorSphere->RotateZ(fCurrentDetectorID, fRotateZCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fDetectorSphere->RotateTo(fCurrentDetectorID, theta, phi, psi);
    } else if (command == fDetectorIDCmd) {
        fCurrentDetectorID = fDetectorIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        DetectorRegistry::GetInstance()->EnableDetector("sphere");
        fDetectorSphere->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fDetectorSphere->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
        if (!fDetectorSphere->HasPlacementRequests()) {
            DetectorRegistry::GetInstance()->DisableDetector("sphere");
        }
    }
}

void DetectorSphereMessenger::ConvertToDoubleTrio(const G4String& paramString,
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
