#ifndef MD1_CUBE_DETECTOR_MODULE_H
#define MD1_CUBE_DETECTOR_MODULE_H

#include <memory>

#include "geometry/base/DetectorModule.hh"

class DetectorCube;
class CubeSensitiveDetector;
class CubeDigitizer;

class CubeDetectorModule : public DetectorModule {
public:
    CubeDetectorModule();
    ~CubeDetectorModule() override;

    G4String GetName() const override { return "cube"; }
    G4String GetUiPath() const override { return "/MultiDetector1/detectors/cube/"; }

    G4bool IsEnabled() const override { return fEnabled; }
    void SetEnabled(G4bool enabled) override { fEnabled = enabled; }
    G4bool HasPlacedGeometry() const override;

    void ConstructGeometry(G4LogicalVolume* motherVolume) override;
    void RegisterSensitiveDetectors(G4SDManager* sdManager) override;
    void RegisterDigitizers(G4DigiManager* digiManager) override;
    std::unique_ptr<DetectorRuntimeState> CreateRuntimeState() const override;
    void CreateAnalysis(G4AnalysisManager* analysisManager,
                        DetectorRuntimeState& runtimeState) override;
    std::vector<G4String> GetSummaryLabels() const override;
    G4String GetSummaryLabel(G4int detectorID) const override;
    DetectorEventData ProcessEvent(const G4Event* event,
                                   G4AnalysisManager* analysisManager,
                                   G4DigiManager* digiManager,
                                   DetectorRuntimeState& runtimeState) override;

private:
    G4bool fEnabled;
    std::unique_ptr<DetectorCube> fGeometry;
};

#endif // MD1_CUBE_DETECTOR_MODULE_H
