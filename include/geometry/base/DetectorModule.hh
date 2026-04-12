#ifndef MD1_DETECTOR_MODULE_H
#define MD1_DETECTOR_MODULE_H

#include <memory>
#include <vector>

#include "G4AnalysisManager.hh"
#include "globals.hh"

#include "geometry/base/DetectorPrintContext.hh"

class G4DigiManager;
class G4Event;
class G4LogicalVolume;
class G4Run;
class G4SDManager;

class DetectorRuntimeState {
public:
    virtual ~DetectorRuntimeState() = default;
};

class DetectorModule {
public:
    virtual ~DetectorModule() = default;

    virtual G4String GetName() const = 0;
    virtual G4String GetUiPath() const = 0;

    virtual G4bool IsEnabled() const = 0;
    virtual void SetEnabled(G4bool enabled) = 0;
    virtual G4bool HasPlacedGeometry() const = 0;

    virtual void ConstructGeometry(G4LogicalVolume* motherVolume) = 0;
    virtual void RegisterSensitiveDetectors(G4SDManager* sdManager) = 0;
    virtual void RegisterDigitizers(G4DigiManager* digiManager) = 0;
    virtual std::unique_ptr<DetectorRuntimeState> CreateRuntimeState() const;
    virtual void PrepareForRun(DetectorRuntimeState& runtimeState, G4bool isMaster);
    virtual void MergeRunResults(DetectorRuntimeState& runtimeState, G4bool isMaster);
    virtual void CreateAnalysis(G4AnalysisManager* analysisManager,
                                DetectorRuntimeState& runtimeState) = 0;
    virtual void BeginOfEvent(const G4Event* event, DetectorRuntimeState& runtimeState);
    virtual std::vector<G4String> GetSummaryLabels() const;
    virtual G4String GetSummaryLabel(G4int detectorID) const;
    virtual void ProcessEvent(const G4Event* event,
                              G4AnalysisManager* analysisManager,
                              G4DigiManager* digiManager,
                              DetectorRuntimeState& runtimeState) = 0;
    virtual void PrintResults(const G4Run* run,
                              const DetectorRuntimeState& runtimeState,
                              const MD1::DetectorPrintContext& context) const;
};

using DetectorModulePtr = std::unique_ptr<DetectorModule>;

#endif // MD1_DETECTOR_MODULE_H
