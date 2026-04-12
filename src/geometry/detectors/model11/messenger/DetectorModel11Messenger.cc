#include "geometry/detectors/model11/messenger/DetectorModel11Messenger.hh"

#include <sstream>

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIparameter.hh"

#include "geometry/base/DetectorRegistry.hh"
#include "geometry/detectors/model11/geometry/DetectorModel11.hh"

DetectorModel11Messenger::DetectorModel11Messenger(DetectorModel11* detectorModel11)
    : G4UImessenger(),
      fDetectorModel11(detectorModel11),
      fCurrentDetectorID(0) {
    fSetSplitAtInterfaceCmd =
        new G4UIcmdWithABool("/MultiDetector1/detectors/model11/setSplitAtInterface", this);
    fSetSplitAtInterfaceCmd->SetGuidance(
        "Keep split-at-interface support disabled for GDML-driven model11 geometry.");
    fSetSplitAtInterfaceCmd->SetParameterName("SplitAtInterface", false);
    fSetSplitAtInterfaceCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometryGDMLCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/model11/setImportedGeometryGDML", this);
    fSetImportedGeometryGDMLCmd->SetGuidance(
        "Set the GDML file that defines the full model11 geometry.");
    fSetImportedGeometryGDMLCmd->SetParameterName("ImportedGeometryGDML", false);
    fSetImportedGeometryGDMLCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometryRootLogicalCmd = new G4UIcmdWithAString(
        "/MultiDetector1/detectors/model11/setImportedGeometryRootLogical", this);
    fSetImportedGeometryRootLogicalCmd->SetGuidance(
        "Select the imported GDML root by exact logical-volume name.");
    fSetImportedGeometryRootLogicalCmd->SetParameterName("ImportedGeometryRootLogical", false);
    fSetImportedGeometryRootLogicalCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometryRootPhysicalCmd = new G4UIcmdWithAString(
        "/MultiDetector1/detectors/model11/setImportedGeometryRootPhysical", this);
    fSetImportedGeometryRootPhysicalCmd->SetGuidance(
        "Select the imported GDML root by exact physical-volume name.");
    fSetImportedGeometryRootPhysicalCmd->SetParameterName("ImportedGeometryRootPhysical", false);
    fSetImportedGeometryRootPhysicalCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometryRootAssemblyCmd = new G4UIcmdWithAString(
        "/MultiDetector1/detectors/model11/setImportedGeometryRootAssembly", this);
    fSetImportedGeometryRootAssemblyCmd->SetGuidance(
        "Select the imported GDML root by exact assembly name.");
    fSetImportedGeometryRootAssemblyCmd->SetParameterName("ImportedGeometryRootAssembly", false);
    fSetImportedGeometryRootAssemblyCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometryValidateCmd = new G4UIcmdWithABool(
        "/MultiDetector1/detectors/model11/setImportedGeometryValidate", this);
    fSetImportedGeometryValidateCmd->SetGuidance(
        "Enable XML schema validation when reading the imported GDML.");
    fSetImportedGeometryValidateCmd->SetParameterName("ValidateImportedGeometry", false);
    fSetImportedGeometryValidateCmd->AvailableForStates(G4State_PreInit);

    fSetImportedGeometrySchemaCmd = new G4UIcmdWithAString(
        "/MultiDetector1/detectors/model11/setImportedGeometrySchema", this);
    fSetImportedGeometrySchemaCmd->SetGuidance(
        "Optionally override the XML schema used to validate the imported GDML.");
    fSetImportedGeometrySchemaCmd->SetParameterName("ImportedGeometrySchema", false);
    fSetImportedGeometrySchemaCmd->AvailableForStates(G4State_PreInit);

    fAddSensitiveVolumeCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/model11/addSensitiveVolume", this);
    fAddSensitiveVolumeCmd->SetGuidance(
        "Mark a GDML logical volume or placed volume name as sensitive for the selected model11 detectorID.");
    fAddSensitiveVolumeCmd->SetParameterName("SensitiveVolumeName", false);
    fAddSensitiveVolumeCmd->AvailableForStates(G4State_PreInit);

    fRemoveSensitiveVolumeCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/model11/removeSensitiveVolume", this);
    fRemoveSensitiveVolumeCmd->SetGuidance(
        "Remove a GDML logical volume or placed volume name from the sensitive-volume selection.");
    fRemoveSensitiveVolumeCmd->SetParameterName("SensitiveVolumeName", false);
    fRemoveSensitiveVolumeCmd->AvailableForStates(G4State_PreInit);

    fClearSensitiveVolumesCmd =
        new G4UIcommand("/MultiDetector1/detectors/model11/clearSensitiveVolumes", this);
    fClearSensitiveVolumesCmd->SetGuidance(
        "Clear all GDML volume names currently selected as sensitive for the chosen detectorID.");
    fClearSensitiveVolumesCmd->AvailableForStates(G4State_PreInit);

    fSetScintillationYieldCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setScintillationYield", this);
    fSetScintillationYieldCmd->SetGuidance("Set the scintillation yield in photons/MeV.");
    fSetScintillationYieldCmd->SetParameterName("ScintillationYield", false);
    fSetScintillationYieldCmd->SetRange("ScintillationYield>0.");
    fSetScintillationYieldCmd->AvailableForStates(G4State_PreInit);

    fSetBirksConstantCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setBirksConstant", this);
    fSetBirksConstantCmd->SetGuidance("Set Birks constant in mm/MeV.");
    fSetBirksConstantCmd->SetParameterName("BirksConstant", false);
    fSetBirksConstantCmd->SetRange("BirksConstant>=0.");
    fSetBirksConstantCmd->AvailableForStates(G4State_PreInit);

    fSetLightCollectionEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setLightCollectionEfficiency", this);
    fSetLightCollectionEfficiencyCmd->SetGuidance("Set the light collection efficiency in [0,1].");
    fSetLightCollectionEfficiencyCmd->SetParameterName("LightCollectionEfficiency", false);
    fSetLightCollectionEfficiencyCmd->SetRange("LightCollectionEfficiency>=0. && LightCollectionEfficiency<=1.");
    fSetLightCollectionEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetDecayTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setDecayTime", this);
    fSetDecayTimeCmd->SetGuidance("Set scintillation decay time.");
    fSetDecayTimeCmd->SetParameterName("DecayTime", false);
    fSetDecayTimeCmd->SetUnitCategory("Time");
    fSetDecayTimeCmd->SetRange("DecayTime>=0.");
    fSetDecayTimeCmd->AvailableForStates(G4State_PreInit);

    fSetTransportDelayCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setTransportDelay", this);
    fSetTransportDelayCmd->SetGuidance("Set optical transport delay.");
    fSetTransportDelayCmd->SetParameterName("TransportDelay", false);
    fSetTransportDelayCmd->SetUnitCategory("Time");
    fSetTransportDelayCmd->SetRange("TransportDelay>=0.");
    fSetTransportDelayCmd->AvailableForStates(G4State_PreInit);

    fSetTimeJitterCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setTimeJitter", this);
    fSetTimeJitterCmd->SetGuidance("Set additional digit-level timing jitter.");
    fSetTimeJitterCmd->SetParameterName("TimeJitter", false);
    fSetTimeJitterCmd->SetUnitCategory("Time");
    fSetTimeJitterCmd->SetRange("TimeJitter>=0.");
    fSetTimeJitterCmd->AvailableForStates(G4State_PreInit);

    fSetResolutionScaleCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setResolutionScale", this);
    fSetResolutionScaleCmd->SetGuidance("Set the counting-resolution scale used in stochastic digitization.");
    fSetResolutionScaleCmd->SetParameterName("ResolutionScale", false);
    fSetResolutionScaleCmd->SetRange("ResolutionScale>=0.");
    fSetResolutionScaleCmd->AvailableForStates(G4State_PreInit);

    fSetPhotosensorTypeCmd =
        new G4UIcmdWithAString("/MultiDetector1/detectors/model11/setPhotosensorType", this);
    fSetPhotosensorTypeCmd->SetGuidance("Set photosensor type: PMT or SiPM.");
    fSetPhotosensorTypeCmd->SetParameterName("PhotosensorType", false);
    fSetPhotosensorTypeCmd->AvailableForStates(G4State_PreInit);

    fSetPMTQuantumEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setPMTQuantumEfficiency", this);
    fSetPMTQuantumEfficiencyCmd->SetGuidance("Set PMT quantum efficiency in [0,1].");
    fSetPMTQuantumEfficiencyCmd->SetParameterName("QuantumEfficiency", false);
    fSetPMTQuantumEfficiencyCmd->SetRange("QuantumEfficiency>=0. && QuantumEfficiency<=1.");
    fSetPMTQuantumEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetPMTDynodeCollectionEfficiencyCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setPMTDynodeCollectionEfficiency", this);
    fSetPMTDynodeCollectionEfficiencyCmd->SetGuidance("Set PMT dynode collection efficiency in [0,1].");
    fSetPMTDynodeCollectionEfficiencyCmd->SetParameterName("DynodeCollectionEfficiency", false);
    fSetPMTDynodeCollectionEfficiencyCmd->SetRange(
        "DynodeCollectionEfficiency>=0. && DynodeCollectionEfficiency<=1.");
    fSetPMTDynodeCollectionEfficiencyCmd->AvailableForStates(G4State_PreInit);

    fSetPMTTransitTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setPMTTransitTime", this);
    fSetPMTTransitTimeCmd->SetGuidance("Set PMT transit time.");
    fSetPMTTransitTimeCmd->SetParameterName("TransitTime", false);
    fSetPMTTransitTimeCmd->SetUnitCategory("Time");
    fSetPMTTransitTimeCmd->SetRange("TransitTime>=0.");
    fSetPMTTransitTimeCmd->AvailableForStates(G4State_PreInit);

    fSetPMTTransitTimeSpreadCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setPMTTransitTimeSpread", this);
    fSetPMTTransitTimeSpreadCmd->SetGuidance("Set PMT transit time spread.");
    fSetPMTTransitTimeSpreadCmd->SetParameterName("TransitTimeSpread", false);
    fSetPMTTransitTimeSpreadCmd->SetUnitCategory("Time");
    fSetPMTTransitTimeSpreadCmd->SetRange("TransitTimeSpread>=0.");
    fSetPMTTransitTimeSpreadCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMPDECmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setSiPMPDE", this);
    fSetSiPMPDECmd->SetGuidance("Set SiPM photo-detection efficiency in [0,1].");
    fSetSiPMPDECmd->SetParameterName("PhotoDetectionEfficiency", false);
    fSetSiPMPDECmd->SetRange("PhotoDetectionEfficiency>=0. && PhotoDetectionEfficiency<=1.");
    fSetSiPMPDECmd->AvailableForStates(G4State_PreInit);

    fSetSiPMMicrocellCountCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setSiPMMicrocellCount", this);
    fSetSiPMMicrocellCountCmd->SetGuidance("Set SiPM microcell count.");
    fSetSiPMMicrocellCountCmd->SetParameterName("MicrocellCount", false);
    fSetSiPMMicrocellCountCmd->SetRange("MicrocellCount>0.");
    fSetSiPMMicrocellCountCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMExcessNoiseFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setSiPMExcessNoiseFactor", this);
    fSetSiPMExcessNoiseFactorCmd->SetGuidance("Set SiPM excess noise factor (>=1).");
    fSetSiPMExcessNoiseFactorCmd->SetParameterName("ExcessNoiseFactor", false);
    fSetSiPMExcessNoiseFactorCmd->SetRange("ExcessNoiseFactor>=1.");
    fSetSiPMExcessNoiseFactorCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMAvalancheTimeCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setSiPMAvalancheTime", this);
    fSetSiPMAvalancheTimeCmd->SetGuidance("Set SiPM avalanche time.");
    fSetSiPMAvalancheTimeCmd->SetParameterName("AvalancheTime", false);
    fSetSiPMAvalancheTimeCmd->SetUnitCategory("Time");
    fSetSiPMAvalancheTimeCmd->SetRange("AvalancheTime>=0.");
    fSetSiPMAvalancheTimeCmd->AvailableForStates(G4State_PreInit);

    fSetSiPMAvalancheTimeSpreadCmd =
        new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/setSiPMAvalancheTimeSpread", this);
    fSetSiPMAvalancheTimeSpreadCmd->SetGuidance("Set SiPM avalanche time spread.");
    fSetSiPMAvalancheTimeSpreadCmd->SetParameterName("AvalancheTimeSpread", false);
    fSetSiPMAvalancheTimeSpreadCmd->SetUnitCategory("Time");
    fSetSiPMAvalancheTimeSpreadCmd->SetRange("AvalancheTimeSpread>=0.");
    fSetSiPMAvalancheTimeSpreadCmd->AvailableForStates(G4State_PreInit);

    fSetDoseCalibrationFactorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setDoseCalibrationFactor", this);
    fSetDoseCalibrationFactorCmd->SetGuidance("Set dose calibration factor in Gy/photoelectron.");
    fSetDoseCalibrationFactorCmd->SetParameterName("DoseCalibrationFactor", false);
    fSetDoseCalibrationFactorCmd->SetRange("DoseCalibrationFactor>0.");
    fSetDoseCalibrationFactorCmd->AvailableForStates(G4State_PreInit);

    fSetDoseCalibrationFactorErrorCmd =
        new G4UIcmdWithADouble("/MultiDetector1/detectors/model11/setDoseCalibrationFactorError", this);
    fSetDoseCalibrationFactorErrorCmd->SetGuidance(
        "Set dose calibration factor uncertainty in Gy/photoelectron.");
    fSetDoseCalibrationFactorErrorCmd->SetParameterName("DoseCalibrationFactorError", false);
    fSetDoseCalibrationFactorErrorCmd->SetRange("DoseCalibrationFactorError>=0.");
    fSetDoseCalibrationFactorErrorCmd->AvailableForStates(G4State_PreInit);

    fDetectorIDCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/model11/detectorID", this);
    fDetectorIDCmd->SetGuidance("Select detector ID.");
    fDetectorIDCmd->SetParameterName("detectorID", false);
    fDetectorIDCmd->SetRange("detectorID>=0");
    fDetectorIDCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/model11/translate", this);
    fTranslateCmd->SetGuidance("Translate the detector by a vector.");
    fTranslateCmd->SetParameterName("dx", "dy", "dz", true, true);
    fTranslateCmd->SetDefaultUnit("cm");
    fTranslateCmd->SetUnitCategory("Length");
    fTranslateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTranslateToCmd = new G4UIcmdWith3VectorAndUnit("/MultiDetector1/detectors/model11/translateTo", this);
    fTranslateToCmd->SetGuidance("Translate the detector to a position.");
    fTranslateToCmd->SetParameterName("x", "y", "z", true, true);
    fTranslateToCmd->SetDefaultUnit("cm");
    fTranslateToCmd->SetUnitCategory("Length");
    fTranslateToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateXCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/rotateX", this);
    fRotateXCmd->SetGuidance("Rotate the detector around X-axis.");
    fRotateXCmd->SetParameterName("angle", false);
    fRotateXCmd->SetDefaultUnit("deg");
    fRotateXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateYCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/rotateY", this);
    fRotateYCmd->SetGuidance("Rotate the detector around Y-axis.");
    fRotateYCmd->SetParameterName("angle", false);
    fRotateYCmd->SetDefaultUnit("deg");
    fRotateYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateZCmd = new G4UIcmdWithADoubleAndUnit("/MultiDetector1/detectors/model11/rotateZ", this);
    fRotateZCmd->SetGuidance("Rotate the detector around Z-axis.");
    fRotateZCmd->SetParameterName("angle", false);
    fRotateZCmd->SetDefaultUnit("deg");
    fRotateZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRotateToCmd = new G4UIcommand("/MultiDetector1/detectors/model11/rotateTo", this);
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

    fAddGeometryToCmd = new G4UIcommand("/MultiDetector1/detectors/model11/addGeometryTo", this);
    fAddGeometryToCmd->SetGuidance("Add geometry to the detector.");
    auto* volumeNameParam = new G4UIparameter("logicalVolumeName", 's', false);
    fAddGeometryToCmd->SetParameter(volumeNameParam);
    auto* copyNoParam = new G4UIparameter("copyNo", 'i', false);
    fAddGeometryToCmd->SetParameter(copyNoParam);
    fAddGeometryToCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fRemoveGeometryCmd = new G4UIcmdWithAnInteger("/MultiDetector1/detectors/model11/removeGeometry", this);
    fRemoveGeometryCmd->SetGuidance("Remove geometry by detector ID.");
    fRemoveGeometryCmd->SetParameterName("detectorID", false);
    fRemoveGeometryCmd->SetRange("detectorID>=0");
    fRemoveGeometryCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorModel11Messenger::~DetectorModel11Messenger() {
    delete fSetSplitAtInterfaceCmd;
    delete fSetImportedGeometryGDMLCmd;
    delete fSetImportedGeometryRootLogicalCmd;
    delete fSetImportedGeometryRootPhysicalCmd;
    delete fSetImportedGeometryRootAssemblyCmd;
    delete fSetImportedGeometryValidateCmd;
    delete fSetImportedGeometrySchemaCmd;
    delete fAddSensitiveVolumeCmd;
    delete fRemoveSensitiveVolumeCmd;
    delete fClearSensitiveVolumesCmd;
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

void DetectorModel11Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fSetSplitAtInterfaceCmd) {
        fDetectorModel11->SetSplitAtInterface(
            fCurrentDetectorID, fSetSplitAtInterfaceCmd->GetNewBoolValue(newValue));
    } else if (command == fSetImportedGeometryGDMLCmd) {
        fDetectorModel11->SetImportedGeometryGDMLPath(fCurrentDetectorID, newValue);
    } else if (command == fSetImportedGeometryRootLogicalCmd) {
        fDetectorModel11->SetImportedGeometryRootLogicalName(fCurrentDetectorID, newValue);
    } else if (command == fSetImportedGeometryRootPhysicalCmd) {
        fDetectorModel11->SetImportedGeometryRootPhysicalName(fCurrentDetectorID, newValue);
    } else if (command == fSetImportedGeometryRootAssemblyCmd) {
        fDetectorModel11->SetImportedGeometryRootAssemblyName(fCurrentDetectorID, newValue);
    } else if (command == fSetImportedGeometryValidateCmd) {
        fDetectorModel11->SetImportedGeometryValidate(
            fCurrentDetectorID, fSetImportedGeometryValidateCmd->GetNewBoolValue(newValue));
    } else if (command == fSetImportedGeometrySchemaCmd) {
        fDetectorModel11->SetImportedGeometrySchema(fCurrentDetectorID, newValue);
    } else if (command == fAddSensitiveVolumeCmd) {
        fDetectorModel11->AddSensitiveVolume(fCurrentDetectorID, newValue);
    } else if (command == fRemoveSensitiveVolumeCmd) {
        fDetectorModel11->RemoveSensitiveVolume(fCurrentDetectorID, newValue);
    } else if (command == fClearSensitiveVolumesCmd) {
        fDetectorModel11->ClearSensitiveVolumes(fCurrentDetectorID);
    } else if (command == fSetScintillationYieldCmd) {
        fDetectorModel11->SetScintillationYield(
            fCurrentDetectorID, fSetScintillationYieldCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetBirksConstantCmd) {
        fDetectorModel11->SetBirksConstant(
            fCurrentDetectorID, fSetBirksConstantCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetLightCollectionEfficiencyCmd) {
        fDetectorModel11->SetLightCollectionEfficiency(
            fCurrentDetectorID, fSetLightCollectionEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetDecayTimeCmd) {
        fDetectorModel11->SetDecayTime(
            fCurrentDetectorID, fSetDecayTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetTransportDelayCmd) {
        fDetectorModel11->SetTransportDelay(
            fCurrentDetectorID, fSetTransportDelayCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetTimeJitterCmd) {
        fDetectorModel11->SetTimeJitter(
            fCurrentDetectorID, fSetTimeJitterCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetResolutionScaleCmd) {
        fDetectorModel11->SetResolutionScale(
            fCurrentDetectorID, fSetResolutionScaleCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPhotosensorTypeCmd) {
        if (newValue == "PMT" || newValue == "pmt") {
            fDetectorModel11->SetPhotosensorType(fCurrentDetectorID, Model11PhotosensorType::PMT);
        } else if (newValue == "SiPM" || newValue == "sipm" || newValue == "SIPM") {
            fDetectorModel11->SetPhotosensorType(fCurrentDetectorID, Model11PhotosensorType::SiPM);
        } else {
            G4Exception("DetectorModel11Messenger::SetNewValue",
                        "Model11InvalidPhotosensorType",
                        FatalException,
                        ("Unsupported photosensor type: " + newValue + ". Use PMT or SiPM.").c_str());
        }
    } else if (command == fSetPMTQuantumEfficiencyCmd) {
        fDetectorModel11->SetPMTQuantumEfficiency(
            fCurrentDetectorID, fSetPMTQuantumEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTDynodeCollectionEfficiencyCmd) {
        fDetectorModel11->SetPMTDynodeCollectionEfficiency(
            fCurrentDetectorID, fSetPMTDynodeCollectionEfficiencyCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTTransitTimeCmd) {
        fDetectorModel11->SetPMTTransitTime(
            fCurrentDetectorID, fSetPMTTransitTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetPMTTransitTimeSpreadCmd) {
        fDetectorModel11->SetPMTTransitTimeSpread(
            fCurrentDetectorID, fSetPMTTransitTimeSpreadCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMPDECmd) {
        fDetectorModel11->SetSiPMPDE(
            fCurrentDetectorID, fSetSiPMPDECmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMMicrocellCountCmd) {
        fDetectorModel11->SetSiPMMicrocellCount(
            fCurrentDetectorID, fSetSiPMMicrocellCountCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMExcessNoiseFactorCmd) {
        fDetectorModel11->SetSiPMExcessNoiseFactor(
            fCurrentDetectorID, fSetSiPMExcessNoiseFactorCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMAvalancheTimeCmd) {
        fDetectorModel11->SetSiPMAvalancheTime(
            fCurrentDetectorID, fSetSiPMAvalancheTimeCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetSiPMAvalancheTimeSpreadCmd) {
        fDetectorModel11->SetSiPMAvalancheTimeSpread(
            fCurrentDetectorID, fSetSiPMAvalancheTimeSpreadCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetDoseCalibrationFactorCmd) {
        fDetectorModel11->SetDoseCalibrationFactor(
            fCurrentDetectorID,
            fSetDoseCalibrationFactorCmd->GetNewDoubleValue(newValue) * gray);
    } else if (command == fSetDoseCalibrationFactorErrorCmd) {
        fDetectorModel11->SetDoseCalibrationFactorError(
            fCurrentDetectorID,
            fSetDoseCalibrationFactorErrorCmd->GetNewDoubleValue(newValue) * gray);
    } else if (command == fTranslateCmd) {
        fDetectorModel11->Translate(fCurrentDetectorID, fTranslateCmd->GetNew3VectorValue(newValue));
    } else if (command == fTranslateToCmd) {
        fDetectorModel11->TranslateTo(fCurrentDetectorID, fTranslateToCmd->GetNew3VectorValue(newValue));
    } else if (command == fRotateXCmd) {
        fDetectorModel11->RotateX(fCurrentDetectorID, fRotateXCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateYCmd) {
        fDetectorModel11->RotateY(fCurrentDetectorID, fRotateYCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateZCmd) {
        fDetectorModel11->RotateZ(fCurrentDetectorID, fRotateZCmd->GetNewDoubleValue(newValue));
    } else if (command == fRotateToCmd) {
        G4double theta, phi, psi;
        ConvertToDoubleTrio(newValue, theta, phi, psi);
        fDetectorModel11->RotateTo(fCurrentDetectorID, theta, phi, psi);
    } else if (command == fDetectorIDCmd) {
        fCurrentDetectorID = fDetectorIDCmd->GetNewIntValue(newValue);
    } else if (command == fAddGeometryToCmd) {
        std::istringstream is(newValue);
        G4String logicalVolumeName;
        G4int copyNo;
        is >> logicalVolumeName >> copyNo;
        DetectorRegistry::GetInstance()->EnableDetector("model11");
        fDetectorModel11->AddGeometryTo(logicalVolumeName, copyNo);
    } else if (command == fRemoveGeometryCmd) {
        fDetectorModel11->RemoveGeometry(fRemoveGeometryCmd->GetNewIntValue(newValue));
        if (!fDetectorModel11->HasPlacementRequests()) {
            DetectorRegistry::GetInstance()->DisableDetector("model11");
        }
    }
}

void DetectorModel11Messenger::ConvertToDoubleTrio(const G4String& paramString,
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
