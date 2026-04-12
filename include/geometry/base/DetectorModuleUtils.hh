#ifndef MD1_DETECTOR_MODULE_UTILS_H
#define MD1_DETECTOR_MODULE_UTILS_H

#include "G4DigiManager.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"

#include "geometry/base/DetectorModule.hh"

namespace DetectorModuleUtils {

inline G4LogicalVolume* GetLogicalVolumeOrThrow(const G4String& logicalVolumeName,
                                                const G4String& detectorName,
                                                const char* scope) {
    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(logicalVolumeName, false);
    if (logicalVolume == nullptr) {
        G4Exception(scope,
                    "DetectorLogicalVolumeNotFound",
                    FatalException,
                    ("Detector module " + detectorName +
                     " could not find logical volume " + logicalVolumeName + ".").c_str());
    }
    return logicalVolume;
}

template <typename TDigitizer>
TDigitizer* GetDigitizerOrThrow(G4DigiManager* digiManager,
                                const G4String& moduleName,
                                const G4String& detectorName,
                                const char* scope) {
    auto* digitizer = static_cast<TDigitizer*>(digiManager->FindDigitizerModule(moduleName));
    if (digitizer == nullptr) {
        G4Exception(scope,
                    "DetectorDigitizerNotFound",
                    FatalException,
                    ("Detector module " + detectorName +
                     " is active but digitizer " + moduleName + " was not registered.").c_str());
    }
    return digitizer;
}

inline G4int GetDigiCollectionIdOrThrow(G4DigiManager* digiManager,
                                        const G4String& collectionName,
                                        const G4String& detectorName,
                                        const char* scope) {
    const G4int collectionId = digiManager->GetDigiCollectionID(collectionName);
    if (collectionId < 0) {
        G4Exception(scope,
                    "DetectorDigiCollectionNotFound",
                    FatalException,
                    ("Detector module " + detectorName +
                     " could not resolve digi collection " + collectionName + ".").c_str());
    }
    return collectionId;
}

template <typename TRuntimeState>
TRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                      const G4String& detectorName,
                                      const char* scope) {
    auto* typedState = dynamic_cast<TRuntimeState*>(&runtimeState);
    if (typedState == nullptr) {
        G4Exception(scope,
                    "DetectorRuntimeStateTypeMismatch",
                    FatalException,
                    ("Detector module " + detectorName +
                     " received an incompatible runtime state.").c_str());
    }
    return *typedState;
}

template <typename TRuntimeState>
const TRuntimeState& GetRuntimeStateOrThrow(const DetectorRuntimeState& runtimeState,
                                            const G4String& detectorName,
                                            const char* scope) {
    auto* typedState = dynamic_cast<const TRuntimeState*>(&runtimeState);
    if (typedState == nullptr) {
        G4Exception(scope,
                    "DetectorRuntimeStateTypeMismatch",
                    FatalException,
                    ("Detector module " + detectorName +
                     " received an incompatible runtime state.").c_str());
    }
    return *typedState;
}

template <typename TSensitiveDetector>
TSensitiveDetector* GetOrCreateSensitiveDetector(G4SDManager* sdManager,
                                                 const G4String& sensitiveDetectorName) {
    auto* existing =
        dynamic_cast<TSensitiveDetector*>(sdManager->FindSensitiveDetector(sensitiveDetectorName, false));
    if (existing != nullptr) {
        return existing;
    }

    auto* sensitiveDetector = new TSensitiveDetector(sensitiveDetectorName);
    sdManager->AddNewDetector(sensitiveDetector);
    return sensitiveDetector;
}

} // namespace DetectorModuleUtils

#endif // MD1_DETECTOR_MODULE_UTILS_H
