#include "geometry/base/DetectorRegistryMessenger.hh"

#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"

#include "geometry/base/DetectorRegistry.hh"

DetectorRegistryMessenger::DetectorRegistryMessenger(DetectorRegistry* registry)
    : fRegistry(registry) {
    fEnableDetectorCmd = new G4UIcmdWithAString("/MultiDetector1/detectors/enable", this);
    fEnableDetectorCmd->SetGuidance("Enable a detector module by name.");
    fEnableDetectorCmd->SetParameterName("detectorName", false);
    fEnableDetectorCmd->AvailableForStates(G4State_PreInit);

    fDisableDetectorCmd = new G4UIcmdWithAString("/MultiDetector1/detectors/disable", this);
    fDisableDetectorCmd->SetGuidance("Disable a detector module by name.");
    fDisableDetectorCmd->SetParameterName("detectorName", false);
    fDisableDetectorCmd->AvailableForStates(G4State_PreInit);

    fListDetectorsCmd = new G4UIcmdWithoutParameter("/MultiDetector1/detectors/list", this);
    fListDetectorsCmd->SetGuidance("List registered detector modules and their status.");
    fListDetectorsCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorRegistryMessenger::~DetectorRegistryMessenger() {
    delete fEnableDetectorCmd;
    delete fDisableDetectorCmd;
    delete fListDetectorsCmd;
}

void DetectorRegistryMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fEnableDetectorCmd) {
        fRegistry->EnableDetector(newValue);
    } else if (command == fDisableDetectorCmd) {
        fRegistry->DisableDetector(newValue);
    } else if (command == fListDetectorsCmd) {
        fRegistry->ListDetectors();
    }
}
