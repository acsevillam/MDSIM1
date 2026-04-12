#include "geometry/detectors/scintCube/messenger/DetectorScintCubeMessenger.hh"

#include <sstream>

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIparameter.hh"

#include "geometry/base/DetectorRegistry.hh"
#include "geometry/detectors/scintCube/geometry/DetectorScintCube.hh"

DetectorScintCubeMessenger::DetectorScintCubeMessenger(DetectorScintCube* detectorScintCube)
    : G4UImessenger(),
      fDetectorScintCube(detectorScintCube),
      fCurrentDetectorID(0) {
    fSetCubeSideCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setSide", this);
    fSetCubeSideCmd->SetGuidance("Set the side length of the scintCube detector.");
    fSetCubeSideCmd->SetParameterName("CubeSide", false);
    fSetCubeSideCmd->SetUnitCategory("Length");
    fSetCubeSideCmd->SetRange("CubeSide>0.");
    fSetCubeSideCmd->AvailableForStates(G4State_PreInit);

    fSetCubeMaterialCmd = new G4UIcmdWithAString("/MultiDetector1/detectors/scintCube/setMaterial", this);
    fSetCubeMaterialCmd->SetGuidance("Set the NIST material name of the scintCube detector.");
    fSetCubeMaterialCmd->SetParameterName("CubeMaterial", false);
    fSetCubeMaterialCmd->AvailableForStates(G4State_PreInit);

    fSetEnvelopeThicknessCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setEnvelopeThickness", this);
    fSetEnvelopeThicknessCmd->SetGuidance(
        "Set the envelope thickness around the scintCube detector. Zero disables the envelope.");
    fSetEnvelopeThicknessCmd->SetParameterName("EnvelopeThickness", false);
    fSetEnvelopeThicknessCmd->SetUnitCategory("Length");
    fSetEnvelopeThicknessCmd->SetRange("EnvelopeThickness>=0.");
    fSetEnvelopeThicknessCmd->AvailableForStates(G4State_PreInit);

    fSetEnvelopeMaterialCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/scintCube/setEnvelopeMaterial", this);
    fSetEnvelopeMaterialCmd->SetGuidance("Set the NIST material name of the scintCube envelope.");
    fSetEnvelopeMaterialCmd->SetParameterName("EnvelopeMaterial", false);
    fSetEnvelopeMaterialCmd->AvailableForStates(G4State_PreInit);

    fSetSplitAtInterfaceCmd =
        new G4UIcmdWithABool("/MultiDetector1/detectors/scintCube/setSplitAtInterface", this);
    fSetSplitAtInterfaceCmd->SetGuidance(
        "Enable automatic split at the WaterBox top interface when the scintCube protrudes into air.");
    fSetSplitAtInterfaceCmd->SetParameterName("SplitAtInterface", false);
    fSetSplitAtInterfaceCmd->AvailableForStates(G4State_PreInit);

    fSetScintillationYieldCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setScintillationYield", this);
    fSetScintillationYieldCmd->SetGuidance("Set the scintillation yield in photons/MeV.");
    fSetScintillationYieldCmd->SetParameterName("ScintillationYield", false);
    fSetScintillationYieldCmd->SetRange("ScintillationYield>0.");
    fSetScintillationYieldCmd->AvailableForStates(G4State_PreInit);

    fSetBirksConstantCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setBirksConstant", this);
    fSetBirksConstantCmd->SetGuidance("Set Birks constant in mm/MeV.");
    fSetBirksConstantCmd->SetParameterName("BirksConstant", false);
    fSetBirksConstantCmd->SetRange("BirksConstant>=0.");
    fSetBirksConstantCmd->AvailableForStates(G4State_PreInit);

    fSetLightCollectionEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setLightCollectionEfficiency", this);
    fSetLightCollectionEfficiencyCmd->SetGuidance("Set the light collection efficiency in [0,1].");
    fSetLightCollectionEfficiencyCmd->SetParameterName("LightCollectionEfficiency", false);
    fSetLightCollectionEfficiencyCmd->SetRange("LightCollectionEfficiency>=0. && LightCollectionEfficiency<=1.");
    fSetLightCollectionEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetDecayTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setDecayTime", this);
    fSetDecayTimeCmd->SetGuidance("Set scintillation decay time.");
    fSetDecayTimeCmd->SetParameterName("DecayTime", false);
    fSetDecayTimeCmd->SetUnitCategory("Time");
    fSetDecayTimeCmd->SetRange("DecayTime>=0.");
    fSetDecayTimeCmd->AvailableForStates(G4State_PreInit);

    fSetTransportDelayCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setTransportDelay", this);
    fSetTransportDelayCmd->SetGuidance("Set optical transport delay.");
    fSetTransportDelayCmd->SetParameterName("TransportDelay", false);
    fSetTransportDelayCmd->SetUnitCategory("Time");
    fSetTransportDelayCmd->SetRange("TransportDelay>=0.");
    fSetTransportDelayCmd->AvailableForStates(G4State_PreInit);

    fSetTimeJitterCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setTimeJitter", this);
    fSetTimeJitterCmd->SetGuidance("Set additional digit-level timing jitter.");
    fSetTimeJitterCmd->SetParameterName("TimeJitter", false);
    fSetTimeJitterCmd->SetUnitCategory("Time");
    fSetTimeJitterCmd->SetRange("TimeJitter>=0.");
    fSetTimeJitterCmd->AvailableForStates(G4State_PreInit);

    fSetResolutionScaleCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setResolutionScale", this);
    fSetResolutionScaleCmd->SetGuidance("Set the counting-resolution scale used in stochastic digitization.");
    fSetResolutionScaleCmd->SetParameterName("ResolutionScale", false);
    fSetResolutionScaleCmd->SetRange("ResolutionScale>=0.");
    fSetResolutionScaleCmd->AvailableForStates(G4State_PreInit);

    fSetPhotosensorTypeCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/scintCube/setPhotosensorType", this);
    fSetPhotosensorTypeCmd->SetGuidance("Set photosensor type: PMT or SiPM.");
    fSetPhotosensorTypeCmd->SetParameterName("PhotosensorType", false);
    fSetPhotosensorTypeCmd->AvailableForStates(G4State_PreInit);

    fSetPMTQuantumEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setPMTQuantumEfficiency", this);
    fSetPMTQuantumEfficiencyCmd->SetGuidance("Set PMT quantum efficiency in [0,1].");
    fSetPMTQuantumEfficiencyCmd->SetParameterName("QuantumEfficiency", false);
    fSetPMTQuantumEfficiencyCmd->SetRange("QuantumEfficiency>=0. && QuantumEfficiency<=1.");
    fSetPMTQuantumEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetPMTDynodeCollectionEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setPMTDynodeCollectionEfficiency", this);
    fSetPMTDynodeCollectionEfficiencyCmd->SetGuidance("Set PMT dynode collection efficiency in [0,1].");
    fSetPMTDynodeCollectionEfficiencyCmd->SetParameterName("DynodeCollectionEfficiency", false);
    fSetPMTDynodeCollectionEfficiencyCmd->SetRange(
        "DynodeCollectionEfficiency>=0. && DynodeCollectionEfficiency<=1.");
    fSetPMTDynodeCollectionEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetPMTTransitTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setPMTTransitTime", this);
    fSetPMTTransitTimeCmd->SetGuidance("Set PMT transit time.");
    fSetPMTTransitTimeCmd->SetParameterName("TransitTime", false);
    fSetPMTTransitTimeCmd->SetUnitCategory("Time");
    fSetPMTTransitTimeCmd->SetRange("TransitTime>=0.");
    fSetPMTTransitTimeCmd->AvailableForStates(G4State_PreInit);

    fSetPMTTransitTimeSpreadCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setPMTTransitTimeSpread", this);
    fSetPMTTransitTimeSpreadCmd->SetGuidance("Set PMT transit time spread.");
    fSetPMTTransitTimeSpreadCmd->SetParameterName("TransitTimeSpread", false);
    fSetPMTTransitTimeSpreadCmd->SetUnitCategory("Time");
    fSetPMTTransitTimeSpreadCmd->SetRange("TransitTimeSpread>=0.");
    fSetPMTTransitTimeSpreadCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMPDECmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setSiPMPDE", this);
    fSetSiPMPDECmd->SetGuidance("Set SiPM photo-detection efficiency in [0,1].");
    fSetSiPMPDECmd->SetParameterName("PhotoDetectionEfficiency", false);
    fSetSiPMPDECmd->SetRange("PhotoDetectionEfficiency>=0. && PhotoDetectionEfficiency<=1.");
    fSetSiPMPDECmd->AvailableForStates(G4State_PreInit);

    fSetSiPMMicrocellCountCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setSiPMMicrocellCount", this);
    fSetSiPMMicrocellCountCmd->SetGuidance("Set SiPM microcell count.");
    fSetSiPMMicrocellCountCmd->SetParameterName("MicrocellCount", false);
    fSetSiPMMicrocellCountCmd->SetRange("MicrocellCount>0.");
    fSetSiPMMicrocellCountCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMExcessNoiseFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setSiPMExcessNoiseFactor", this);
    fSetSiPMExcessNoiseFactorCmd->SetGuidance("Set SiPM excess noise factor (>=1).");
    fSetSiPMExcessNoiseFactorCmd->SetParameterName("ExcessNoiseFactor", false);
    fSetSiPMExcessNoiseFactorCmd->SetRange("ExcessNoiseFactor>=1.");
    fSetSiPMExcessNoiseFactorCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMAvalancheTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setSiPMAvalancheTime", this);
    fSetSiPMAvalancheTimeCmd->SetGuidance("Set SiPM avalanche time.");
    fSetSiPMAvalancheTimeCmd->SetParameterName("AvalancheTime", false);
    fSetSiPMAvalancheTimeCmd->SetUnitCategory("Time");
    fSetSiPMAvalancheTimeCmd->SetRange("AvalancheTime>=0.");
    fSetSiPMAvalancheTimeCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMAvalancheTimeSpreadCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/setSiPMAvalancheTimeSpread", this);
    fSetSiPMAvalancheTimeSpreadCmd->SetGuidance("Set SiPM avalanche time spread.");
    fSetSiPMAvalancheTimeSpreadCmd->SetParameterName("AvalancheTimeSpread", false);
    fSetSiPMAvalancheTimeSpreadCmd->SetUnitCategory("Time");
    fSetSiPMAvalancheTimeSpreadCmd->SetRange("AvalancheTimeSpread>=0.");
    fSetSiPMAvalancheTimeSpreadCmd->AvailableForStates(G4State_PreInit);

    fSetDoseCalibrationFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setDoseCalibrationFactor", this);
    fSetDoseCalibrationFactorCmd->SetGuidance("Set dose calibration factor in Gy/photoelectron.");
    fSetDoseCalibrationFactorCmd->SetParameterName("DoseCalibrationFactor", false);
    fSetDoseCalibrationFactorCmd->SetRange("DoseCalibrationFactor>0.");
    fSetDoseCalibrationFactorCmd->AvailableForStates(G4State_PreInit);

    fSetDoseCalibrationFactorErrorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/scintCube/setDoseCalibrationFactorError", this);
    fSetDoseCalibrationFactorErrorCmd->SetGuidance(
        "Set dose calibration factor uncertainty in Gy/photoelectron.");
    fSetDoseCalibrationFactorErrorCmd->SetParameterName("DoseCalibrationFactorError", false);
    fSetDoseCalibrationFactorErrorCmd->SetRange("DoseCalibrationFactorError>=0.");
    fSetDoseCalibrationFactorErrorCmd->AvailableForStates(G4State_PreInit);

    fDetectorIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/scintCube/detectorID", this);
    fDetectorIDCmd->SetGuidance("Select detector ID.");
    fDetectorIDCmd->SetParameterName("detectorID", false);
    fDetectorIDCmd->SetRange("detectorID>=0");
    fDetectorIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/scintCube/translate", this);
    fTranslateCmd->SetGuidance("Translate the detector by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/scintCube/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the detector to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->SetUnitCategory("Length");
    fTranslateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the detector around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the detector around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/scintCube/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the detector around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/detectors/scintCube/rotateTo", this);
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

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/detectors/scintCube/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the detector.");
    auto* volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto* copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/scintCube/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by detector ID.");
    fRemoveGeometryCmd->SetParameterName("detectorID", false);
    fRemoveGeometryCmd->SetRange("detectorID>=0");
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorScintCubeMessenger::~DetectorScintCubeMessenger() {
    delete fSetCubeSideCmd;
    delete fSetCubeMaterialCmd;
    delete fSetEnvelopeThicknessCmd;
    delete fSetEnvelopeMaterialCmd;
    delete fSetSplitAtInterfaceCmd;
    delete fSetScintillationYieldCmd;
    delete fSetBirksConstantCmd;
    delete fSetLightCollectionEfficiencyCmd;
    delete fSetDecayTimeCmd;
    delete fSetTransportDelayCmd;
    delete fSetTimeJitterCmd;
    delete fSetResolutionScaleCmd;
    delete fSetPhotosensorTypeCmd;
    delete fSetPMTQuantumEfficiencyCmd;
    delete fSetPMTDynodeCollectionEfficiencyCmd;
    delete fSetPMTTransitTimeCmd;
    delete fSetPMTTransitTimeSpreadCmd;
    delete fSetSiPMPDECmd;
    delete fSetSiPMMicrocellCountCmd;
    delete fSetSiPMExcessNoiseFactorCmd;
    delete fSetSiPMAvalancheTimeCmd;
    delete fSetSiPMAvalancheTimeSpreadCmd;
    delete fSetDoseCalibrationFactorCmd;
    delete fSetDoseCalibrationFactorErrorCmd;
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

void DetectorScintCubeMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetCubeSideCmd) {
        fDetectorScintCube->SetCubeSide(fCurrentDetectorID, fSetCubeSideCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetCubeMaterialCmd) {
        fDetectorScintCube->SetCubeMaterial(fCurrentDetectorID, newValue);
    } else if (command == fSetEnvelopeThicknessCmd) {
        fDetectorScintCube->SetEnvelopeThickness(
            fCurrentDetectorID, fSetEnvelopeThicknessCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetEnvelopeMaterialCmd) {
        fDetectorScintCube->SetEnvelopeMaterial(fCurrentDetectorID, newValue);
    } else if (command == fSetSplitAtInterfaceCmd) {
        fDetectorScintCube->SetSplitAtInterface(
            fCurrentDetectorID, fSetSplitAtInterfaceCmd->GetNewBoolValue(newValue));
    } else if (command == fSetScintillationYieldCmd) {
        fDetectorScintCube->SetScintillationYield(
            fCurrentDetectorID, fSetScintillationYieldCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetBirksConstantCmd) {
        fDetectorScintCube->SetBirksConstant(
            fCurrentDetectorID, fSetBirksConstantCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetLightCollectionEfficiencyCmd) {
        fDetectorScintCube->SetLightCollectionEfficiency(
            fCurrentDetectorID, fSetLightCollectionEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetDecayTimeCmd) {
        fDetectorScintCube->SetDecayTime(
            fCurrentDetectorID, fSetDecayTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetTransportDelayCmd) {
        fDetectorScintCube->SetTransportDelay(
            fCurrentDetectorID, fSetTransportDelayCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetTimeJitterCmd) {
        fDetectorScintCube->SetTimeJitter(
            fCurrentDetectorID, fSetTimeJitterCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetResolutionScaleCmd) {
        fDetectorScintCube->SetResolutionScale(
            fCurrentDetectorID, fSetResolutionScaleCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPhotosensorTypeCmd) {
        if (newValue == "PMT" || newValue == "pmt") {
            fDetectorScintCube->SetPhotosensorType(fCurrentDetectorID, ScintCubePhotosensorType::PMT);
        } else if (newValue == "SiPM" || newValue == "sipm" || newValue == "SIPM") {
            fDetectorScintCube->SetPhotosensorType(fCurrentDetectorID, ScintCubePhotosensorType::SiPM);
        } else {
            G4Exception("DetectorScintCubeMessenger::SetNewValue",
                        "ScintCubeInvalidPhotosensorType",
                        FatalException,
                        ("Unsupported photosensor type: " + newValue + ". Use PMT or SiPM.").c_str());
        }
    } else if (command == fSetPMTQuantumEfficiencyCmd) {
        fDetectorScintCube->SetPMTQuantumEfficiency(
            fCurrentDetectorID, fSetPMTQuantumEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTDynodeCollectionEfficiencyCmd) {
        fDetectorScintCube->SetPMTDynodeCollectionEfficiency(
            fCurrentDetectorID, fSetPMTDynodeCollectionEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTTransitTimeCmd) {
        fDetectorScintCube->SetPMTTransitTime(
            fCurrentDetectorID, fSetPMTTransitTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTTransitTimeSpreadCmd) {
        fDetectorScintCube->SetPMTTransitTimeSpread(
            fCurrentDetectorID, fSetPMTTransitTimeSpreadCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMPDECmd) {
        fDetectorScintCube->SetSiPMPDE(
            fCurrentDetectorID, fSetSiPMPDECmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMMicrocellCountCmd) {
        fDetectorScintCube->SetSiPMMicrocellCount(
            fCurrentDetectorID, fSetSiPMMicrocellCountCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMExcessNoiseFactorCmd) {
        fDetectorScintCube->SetSiPMExcessNoiseFactor(
            fCurrentDetectorID, fSetSiPMExcessNoiseFactorCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMAvalancheTimeCmd) {
        fDetectorScintCube->SetSiPMAvalancheTime(
            fCurrentDetectorID, fSetSiPMAvalancheTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMAvalancheTimeSpreadCmd) {
        fDetectorScintCube->SetSiPMAvalancheTimeSpread(
            fCurrentDetectorID, fSetSiPMAvalancheTimeSpreadCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetDoseCalibrationFactorCmd) {
        fDetectorScintCube->SetDoseCalibrationFactor(
            fCurrentDetectorID,
            fSetDoseCalibrationFactorCmd->GetNewDoubleValue(newValue) * gray);
    } else if (command == fSetDoseCalibrationFactorErrorCmd) {
        fDetectorScintCube->SetDoseCalibrationFactorError(
            fCurrentDetectorID,
            fSetDoseCalibrationFactorErrorCmd->GetNewDoubleValue(newValue) * gray);
    } else if (command == fTranslateCmd) {
        fDetectorScintCube->Translate(fCurrentDetectorID, fTranslateCmd->GetNew3VectorValue(newValue));
    } else if (command == fTranslateToCmd) {
        fDetectorScintCube->TranslateTo(fCurrentDetectorID, fTranslateToCmd->GetNew3VectorValue(newValue));
    } else if (command == fRotateXCmd) {
        fDetectorScintCube->RotateX(fCurrentDetectorID, fRotateXCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateYCmd) {
        fDetectorScintCube->RotateY(fCurrentDetectorID, fRotateYCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateZCmd) {
        fDetectorScintCube->RotateZ(fCurrentDetectorID, fRotateZCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fDetectorScintCube->RotateTo(fCurrentDetectorID, theta, phi, psi);
    } else if (command == fDetectorIDCmd) {
        fCurrentDetectorID = fDetectorIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        DetectorRegistry::GetInstance()->EnableDetector("scintCube");
        fDetectorScintCube->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fDetectorScintCube->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
        if (!fDetectorScintCube->HasPlacementRequests()) {
            DetectorRegistry::GetInstance()->DisableDetector("scintCube");
        }
    }
}

void DetectorScintCubeMessenger::ConvertToDoubleTrio(const G4String& paramString,
                                                     G4double& xval,
                                                     G4double& yval,
                                                     G4double& zval) {
    G4double x, y, z;
    char units[30];
    std::istringstream is(paramString);
    is >> x >> y >> z >> units;
    const G4String unit = units;
    xval = x * G4UIcommand::ValueOf(unit);
    yval = y * G4UIcommand::ValueOf(unit);
    zval = z * G4UIcommand::ValueOf(unit);
}
