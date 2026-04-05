#ifndef MD1_BB7_DETECTOR_MODULE_H
#define MD1_BB7_DETECTOR_MODULE_H

#include <memory>

#include "G4SystemOfUnits.hh"
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
    void CreateAnalysis(G4AnalysisManager* analysisManager) override;
    DetectorEventData ProcessEvent(const G4Event* event,
                                   G4AnalysisManager* analysisManager,
                                   G4DigiManager* digiManager) override;

private:
    static const G4double kStaticCalibrationFactor;

    G4bool fEnabled;
    G4int fNtupleId;
    G4int fChargeMapId;
    std::unique_ptr<DetectorDualBB7> fGeometry;
};

#endif // MD1_BB7_DETECTOR_MODULE_H
