#ifndef MD1_DETECTOR_MODULE_H
#define MD1_DETECTOR_MODULE_H

#include <memory>

#include "G4AnalysisManager.hh"
#include "globals.hh"
#include "geometry/base/DetectorEventData.hh"

class G4DigiManager;
class G4Event;
class G4LogicalVolume;
class G4SDManager;

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
    virtual void CreateAnalysis(G4AnalysisManager* analysisManager) = 0;
    virtual void BeginOfEvent(const G4Event* event);
    virtual DetectorEventData ProcessEvent(const G4Event* event,
                                           G4AnalysisManager* analysisManager,
                                           G4DigiManager* digiManager) = 0;
};

using DetectorModulePtr = std::unique_ptr<DetectorModule>;

#endif // MD1_DETECTOR_MODULE_H
