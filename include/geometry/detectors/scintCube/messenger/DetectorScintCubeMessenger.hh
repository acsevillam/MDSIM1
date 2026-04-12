#ifndef MDSIM1_DETECTOR_SCINT_CUBE_MESSENGER_HH
#define MDSIM1_DETECTOR_SCINT_CUBE_MESSENGER_HH

#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcommand.hh"
#include "G4UImessenger.hh"

class DetectorScintCube;

class DetectorScintCubeMessenger : public G4UImessenger {
public:
    explicit DetectorScintCubeMessenger(DetectorScintCube* detectorScintCube);
    ~DetectorScintCubeMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    void ConvertToDoubleTrio(const G4String& paramString, G4double& xval, G4double& yval, G4double& zval);

    DetectorScintCube* fDetectorScintCube;
    G4int fCurrentDetectorID;

    G4UIcmdWithADoubleAndUnit* fSetCubeSideCmd;
    G4UIcmdWithAString* fSetCubeMaterialCmd;
    G4UIcmdWithADoubleAndUnit* fSetEnvelopeThicknessCmd;
    G4UIcmdWithAString* fSetEnvelopeMaterialCmd;
    G4UIcmdWithABool* fSetSplitAtInterfaceCmd;
    G4UIcmdWithADouble* fSetScintillationYieldCmd;
    G4UIcmdWithADouble* fSetBirksConstantCmd;
    G4UIcmdWithADouble* fSetLightCollectionEfficiencyCmd;
    G4UIcmdWithADoubleAndUnit* fSetDecayTimeCmd;
    G4UIcmdWithADoubleAndUnit* fSetTransportDelayCmd;
    G4UIcmdWithADoubleAndUnit* fSetTimeJitterCmd;
    G4UIcmdWithADouble* fSetResolutionScaleCmd;
    G4UIcmdWithAString* fSetPhotosensorTypeCmd;
    G4UIcmdWithADouble* fSetPMTQuantumEfficiencyCmd;
    G4UIcmdWithADouble* fSetPMTDynodeCollectionEfficiencyCmd;
    G4UIcmdWithADoubleAndUnit* fSetPMTTransitTimeCmd;
    G4UIcmdWithADoubleAndUnit* fSetPMTTransitTimeSpreadCmd;
    G4UIcmdWithADouble* fSetSiPMPDECmd;
    G4UIcmdWithADouble* fSetSiPMMicrocellCountCmd;
    G4UIcmdWithADouble* fSetSiPMExcessNoiseFactorCmd;
    G4UIcmdWithADoubleAndUnit* fSetSiPMAvalancheTimeCmd;
    G4UIcmdWithADoubleAndUnit* fSetSiPMAvalancheTimeSpreadCmd;
    G4UIcmdWithADouble* fSetDoseCalibrationFactorCmd;
    G4UIcmdWithADouble* fSetDoseCalibrationFactorErrorCmd;
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

#endif // MDSIM1_DETECTOR_SCINT_CUBE_MESSENGER_HH
