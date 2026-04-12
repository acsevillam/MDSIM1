#ifndef MD1_BB7_DETECTOR_MODULE_H
#define MD1_BB7_DETECTOR_MODULE_H

#include <memory>

#include "geometry/base/DetectorModule.hh"

class DetectorDualBB7;

class BB7DetectorModule : public DetectorModule {
public:
    BB7DetectorModule();
    ~BB7DetectorModule() override;

    G4String GetName() const override { return "BB7"; }
    G4String GetUiPath() const override { return "/MultiDetector1/detectors/BB7/"; }

    G4bool IsEnabled() const override { return fEnabled; }
    void SetEnabled(G4bool enabled) override { fEnabled = enabled; }
    G4bool HasPlacedGeometry() const override;

    void ConstructGeometry(G4LogicalVolume* motherVolume) override;
    void RegisterSensitiveDetectors(G4SDManager* sdManager) override;
    void RegisterDigitizers(G4DigiManager* digiManager) override;
    std::unique_ptr<DetectorRuntimeState> CreateRuntimeState() const override;
    void PrepareForRun(DetectorRuntimeState& runtimeState, G4bool isMaster) override;
    void MergeRunResults(DetectorRuntimeState& runtimeState, G4bool isMaster) override;
    void CreateAnalysis(G4AnalysisManager* analysisManager,
                        DetectorRuntimeState& runtimeState) override;
    std::vector<G4String> GetSummaryLabels() const override;
    G4String GetSummaryLabel(G4int detectorID) const override;
    void ProcessEvent(const G4Event* event,
                      G4AnalysisManager* analysisManager,
                      G4DigiManager* digiManager,
                      DetectorRuntimeState& runtimeState) override;
    void PrintResults(const G4Run* run,
                      const DetectorRuntimeState& runtimeState,
                      const MD1::DetectorPrintContext& context) const override;

private:
    G4bool fEnabled;
    std::unique_ptr<DetectorDualBB7> fGeometry;
};

#endif // MD1_BB7_DETECTOR_MODULE_H
