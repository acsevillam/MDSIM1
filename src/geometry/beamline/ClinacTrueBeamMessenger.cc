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
#include "geometry/beamline/ClinacTrueBeamMessenger.hh"
#include "geometry/beamline/ClinacTrueBeam.hh"

ClinacTrueBeamMessenger::ClinacTrueBeamMessenger(ClinacTrueBeam* clinac)
    : G4UImessenger(), fClinac(clinac), fCurrentClinacID(0) {

    fSetJaw1XCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/setJaw1X", this);
    fSetJaw1XCmd->SetGuidance("Set Jaw1X apperture to the specified value.");
    fSetJaw1XCmd->SetParameterName("length", false);
    fSetJaw1XCmd->SetDefaultUnit("cm");
    fSetJaw1XCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetJaw2XCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/setJaw2X", this);
    fSetJaw2XCmd->SetGuidance("Set Jaw2X apperture to the specified value.");
    fSetJaw2XCmd->SetParameterName("length", false);
    fSetJaw2XCmd->SetDefaultUnit("cm");
    fSetJaw2XCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetJaw1YCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/setJaw1Y", this);
    fSetJaw1YCmd->SetGuidance("Set Jaw1Y apperture to the specified value.");
    fSetJaw1YCmd->SetParameterName("length", false);
    fSetJaw1YCmd->SetDefaultUnit("cm");
    fSetJaw1YCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetJaw2YCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/setJaw2Y", this);
    fSetJaw2YCmd->SetGuidance("Set Jaw2Y apperture to the specified value.");
    fSetJaw2YCmd->SetParameterName("length", false);
    fSetJaw2YCmd->SetDefaultUnit("cm");
    fSetJaw2YCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateGantryToCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/rotateGantryTo", this);
    fRotateGantryToCmd->SetGuidance("Rotate the clinac gantry around X-axis to the specified angle.");
    fRotateGantryToCmd->SetParameterName("angle", false);
    fRotateGantryToCmd->SetDefaultUnit("deg");
    fRotateGantryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateGantryCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/rotateGantry", this);
    fRotateGantryCmd->SetGuidance("Rotate the clinac gantry around X-axis.");
    fRotateGantryCmd->SetParameterName("angle", false);
    fRotateGantryCmd->SetDefaultUnit("deg");
    fRotateGantryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateCollimatorToCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/rotateCollimatorTo", this);
    fRotateCollimatorToCmd->SetGuidance("Rotate the clinac collimator around Z'-axis to the specified angle.");
    fRotateCollimatorToCmd->SetParameterName("angle", false);
    fRotateCollimatorToCmd->SetDefaultUnit("deg");
    fRotateCollimatorToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateCollimatorCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/beamline/clinac/rotateCollimator", this);
    fRotateCollimatorCmd->SetGuidance("Rotate the clinac collimator around Z'-axis.");
    fRotateCollimatorCmd->SetParameterName("angle", false);
    fRotateCollimatorCmd->SetDefaultUnit("deg");
    fRotateCollimatorCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

ClinacTrueBeamMessenger::~ClinacTrueBeamMessenger() {
    delete fSetJaw1XCmd;
    delete fSetJaw2XCmd;
    delete fSetJaw1YCmd;
    delete fSetJaw2YCmd;
    delete fRotateGantryToCmd;
    delete fRotateGantryCmd;
    delete fRotateCollimatorToCmd;
    delete fRotateCollimatorCmd;
}

void ClinacTrueBeamMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetJaw1XCmd) {
        G4double value = fSetJaw1XCmd->GetNewDoubleValue(newValue);
        fClinac->SetJaw1X(fCurrentClinacID, value);
    } else if (command == fSetJaw2XCmd) {
        G4double value = fSetJaw2XCmd->GetNewDoubleValue(newValue);
        fClinac->SetJaw2X(fCurrentClinacID, value);
    } else if (command == fSetJaw1YCmd) {
        G4double value = fSetJaw1YCmd->GetNewDoubleValue(newValue);
        fClinac->SetJaw1Y(fCurrentClinacID, value);
    } else if (command == fSetJaw2YCmd) {
        G4double value = fSetJaw2YCmd->GetNewDoubleValue(newValue);
        fClinac->SetJaw2Y(fCurrentClinacID, value);
    } else if (command == fRotateGantryToCmd) {
        G4double angle = fRotateGantryCmd->GetNewDoubleValue(newValue);
        fClinac->RotateGantryTo(fCurrentClinacID, angle);
    } else if (command == fRotateGantryCmd) {
        G4double angle = fRotateGantryCmd->GetNewDoubleValue(newValue);
        fClinac->RotateGantry(fCurrentClinacID, angle);
    } else if (command == fRotateCollimatorToCmd) {
        G4double angle = fRotateCollimatorToCmd->GetNewDoubleValue(newValue);
        fClinac->RotateCollimatorTo(fCurrentClinacID, angle);
    } else if (command == fRotateCollimatorCmd) {
        G4double angle = fRotateCollimatorCmd->GetNewDoubleValue(newValue);
        fClinac->RotateCollimator(fCurrentClinacID, angle);
    }
}

void ClinacTrueBeamMessenger::ConvertToDoubleTrio(const G4String& paramString, G4double& xval, G4double& yval, G4double& zval) {
    G4double x, y, z;
    char unts[30];
    std::istringstream is(paramString);
    is >> x >> y >> z >> unts;
    G4String unt = unts;
    xval = x * G4UIcommand::ValueOf(unt);
    yval = y * G4UIcommand::ValueOf(unt);
    zval = z * G4UIcommand::ValueOf(unt);
}
