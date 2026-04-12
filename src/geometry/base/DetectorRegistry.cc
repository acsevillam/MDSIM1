#include "geometry/base/DetectorRegistry.hh"

#include "G4Exception.hh"

#include "geometry/base/DetectorRegistryMessenger.hh"
#include "geometry/detectors/BB7/BB7DetectorModule.hh"
#include "geometry/detectors/basic/cylinder/CylinderDetectorModule.hh"
#include "geometry/detectors/basic/cube/CubeDetectorModule.hh"
#include "geometry/detectors/model11/Model11DetectorModule.hh"
#include "geometry/detectors/scintCube/ScintCubeDetectorModule.hh"
#include "geometry/detectors/basic/sphere/SphereDetectorModule.hh"

DetectorRegistry* DetectorRegistry::fInstance = nullptr;

DetectorRegistry* DetectorRegistry::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new DetectorRegistry();
    }
    return fInstance;
}

void DetectorRegistry::Kill() {
    delete fInstance;
    fInstance = nullptr;
}

DetectorRegistry::DetectorRegistry()
    : fMessenger(nullptr) {
    fMessenger = new DetectorRegistryMessenger(this);
    RegisterDefaults();
}

DetectorRegistry::~DetectorRegistry() {
    delete fMessenger;
    fMessenger = nullptr;
}

void DetectorRegistry::RegisterDefaults() {
    fDetectors.push_back(std::make_unique<CubeDetectorModule>());
    fDetectors.push_back(std::make_unique<CylinderDetectorModule>());
    fDetectors.push_back(std::make_unique<SphereDetectorModule>());
    fDetectors.push_back(std::make_unique<ScintCubeDetectorModule>());
    fDetectors.push_back(std::make_unique<Model11DetectorModule>());
    fDetectors.push_back(std::make_unique<BB7DetectorModule>());
}

std::vector<DetectorModule*> DetectorRegistry::GetEnabledDetectors() const {
    std::vector<DetectorModule*> enabledDetectors;
    for (const auto& detector : fDetectors) {
        if (detector->IsEnabled()) {
            enabledDetectors.push_back(detector.get());
        }
    }
    return enabledDetectors;
}

std::vector<DetectorModule*> DetectorRegistry::GetActiveDetectors() const {
    std::vector<DetectorModule*> activeDetectors;
    for (const auto& detector : fDetectors) {
        if (detector->IsEnabled() && detector->HasPlacedGeometry()) {
            activeDetectors.push_back(detector.get());
        }
    }
    return activeDetectors;
}

void DetectorRegistry::EnableDetector(const G4String& detectorName) {
    auto* detector = FindDetector(detectorName);
    if (detector == nullptr) {
        G4Exception("DetectorRegistry::EnableDetector",
                    "DetectorNotFound",
                    FatalException,
                    ("Unknown detector module: " + detectorName).c_str());
        return;
    }
    detector->SetEnabled(true);
}

void DetectorRegistry::DisableDetector(const G4String& detectorName) {
    auto* detector = FindDetector(detectorName);
    if (detector == nullptr) {
        G4Exception("DetectorRegistry::DisableDetector",
                    "DetectorNotFound",
                    FatalException,
                    ("Unknown detector module: " + detectorName).c_str());
        return;
    }
    detector->SetEnabled(false);
}

DetectorModule* DetectorRegistry::FindDetector(const G4String& detectorName) const {
    for (const auto& detector : fDetectors) {
        if (detector->GetName() == detectorName) {
            return detector.get();
        }
    }
    return nullptr;
}

void DetectorRegistry::ListDetectors() const {
    G4cout << "Registered detector modules:" << G4endl;
    for (const auto& detector : fDetectors) {
        G4cout << " - " << detector->GetName()
               << " [module=" << (detector->IsEnabled() ? "enabled" : "disabled") << "]"
               << " [geometry=" << (detector->HasPlacedGeometry() ? "placed" : "not placed") << "]"
               << " ui=" << detector->GetUiPath()
               << G4endl;
    }
}
