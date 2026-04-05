#ifndef MD1_DETECTOR_REGISTRY_H
#define MD1_DETECTOR_REGISTRY_H

#include <memory>
#include <vector>

#include "globals.hh"
#include "geometry/base/DetectorModule.hh"

class DetectorRegistryMessenger;

class DetectorRegistry {
public:
    static DetectorRegistry* GetInstance();
    static void Kill();

    const std::vector<DetectorModulePtr>& GetAllDetectors() const { return fDetectors; }
    std::vector<DetectorModule*> GetEnabledDetectors() const;
    std::vector<DetectorModule*> GetActiveDetectors() const;

    void EnableDetector(const G4String& detectorName);
    void DisableDetector(const G4String& detectorName);
    DetectorModule* FindDetector(const G4String& detectorName) const;
    void ListDetectors() const;

private:
    DetectorRegistry();
    ~DetectorRegistry();

    void RegisterDefaults();

    static DetectorRegistry* fInstance;
    std::vector<DetectorModulePtr> fDetectors;
    DetectorRegistryMessenger* fMessenger;
};

#endif // MD1_DETECTOR_REGISTRY_H
