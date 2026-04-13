/*
 *
 * Geant4 MultiDetector Simulation v1
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

// Geant4 Headers
#include "geometry/base/GenericGeometry.hh"
#include "G4GeometryManager.hh"
#include "G4VVisManager.hh"
#include "G4VisManager.hh"
#include "G4RunManager.hh"
#include "G4Exception.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Threading.hh"

#include <algorithm>

namespace {

G4bool ShouldSkipWorkerGeometryMutation() {
    return G4Threading::IsWorkerThread();
}

G4bool IsWaterPhantomAlias(const G4String& volumeName) {
    return volumeName == "WaterBox" || volumeName == "WaterTube";
}

G4LogicalVolume* ResolvePlacementLogicalVolume(const G4String& volumeName) {
    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName, false);
    if (logicalVolume != nullptr || !IsWaterPhantomAlias(volumeName)) {
        return logicalVolume;
    }

    if (volumeName != "WaterBox") {
        logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
        if (logicalVolume != nullptr) {
            return logicalVolume;
        }
    }

    if (volumeName != "WaterTube") {
        logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("WaterTube", false);
        if (logicalVolume != nullptr) {
            return logicalVolume;
        }
    }

    return nullptr;
}

} // namespace

GenericGeometry::GenericGeometry()
    : geometryName("GenericGeometry"),
      det_origin(G4ThreeVector()),
      fAreVolumensDefined(false),
      fAreVolumensAssembled(false) {}

GenericGeometry::~GenericGeometry() {
    for (auto& [copyNo, rotation] : detRotMat) {
        delete rotation;
        rotation = nullptr;
    }
}

G4RotationMatrix* GenericGeometry::EnsureRotationMatrix(const G4int& copyNo) {
    auto& rotation = detRotMat[copyNo];
    if (rotation == nullptr) {
        rotation = new G4RotationMatrix();
    }
    return rotation;
}

void GenericGeometry::SetPrimaryFrameVolume(const G4int& copyNo, G4VPhysicalVolume* frameVolume) {
    detFrame[copyNo] = frameVolume;
}

void GenericGeometry::AddAuxiliaryFrameVolume(const G4int& copyNo, G4VPhysicalVolume* frameVolume) {
    detAuxFrames[copyNo].push_back(frameVolume);
}

std::vector<G4VPhysicalVolume*> GenericGeometry::GetPlacementFrames(const G4int& copyNo) const {
    std::vector<G4VPhysicalVolume*> frames;
    auto primaryIt = detFrame.find(copyNo);
    if (primaryIt != detFrame.end() && primaryIt->second != nullptr) {
        frames.push_back(primaryIt->second);
    }

    auto auxIt = detAuxFrames.find(copyNo);
    if (auxIt != detAuxFrames.end()) {
        for (auto* frame : auxIt->second) {
            if (frame != nullptr) {
                frames.push_back(frame);
            }
        }
    }
    return frames;
}

G4Transform3D GenericGeometry::BuildStoredTransform(const G4int& copyNo) const {
    G4RotationMatrix rotation;
    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end() && rotationIt->second != nullptr) {
        rotation = *(rotationIt->second);
    }

    G4ThreeVector position;
    auto positionIt = detPosition.find(copyNo);
    if (positionIt != detPosition.end()) {
        position = positionIt->second - det_origin;
    }

    return G4Transform3D(rotation, position);
}

void GenericGeometry::RebuildPlacement(const G4int& copyNo) {
    auto motherIt = detMotherVolumeNames.find(copyNo);
    if (motherIt == detMotherVolumeNames.end()) {
        G4Exception("GenericGeometry::RebuildPlacement",
                    "PlacementMotherNotFound",
                    FatalException,
                    ("Requested mother volume for copy number " + std::to_string(copyNo) +
                     " in geometry " + geometryName + " was not found.").c_str());
        return;
    }

    const G4String motherVolumeName = motherIt->second;
    G4RotationMatrix rotation;
    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end() && rotationIt->second != nullptr) {
        rotation = *(rotationIt->second);
    }
    G4LogicalVolume* logicalVolume = ResolvePlacementLogicalVolume(motherVolumeName);
    if (logicalVolume == nullptr) {
        G4Exception("GenericGeometry::RebuildPlacement",
                    "PlacementMotherLogicalVolumeNotFound",
                    FatalException,
                    ("Logical volume " + motherIt->second + " in geometry " + geometryName +
                     " was not found while rebuilding the placement.").c_str());
        return;
    }

    const auto transform = BuildStoredTransform(copyNo);
    RemoveGeometry(copyNo);
    detMotherVolumeNames[copyNo] = motherVolumeName;
    auto transformCopy = transform;
    AddGeometry(logicalVolume, &transformCopy, copyNo);
    auto storedRotationIt = detRotMat.find(copyNo);
    if (storedRotationIt != detRotMat.end()) {
        delete storedRotationIt->second;
        storedRotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);
}

G4bool GenericGeometry::RequiresPlacementRebuild(const G4int& /*copyNo*/) const {
    return false;
}

void GenericGeometry::OnAfterPlacementRemoval(const G4int& /*copyNo*/) {}

G4bool GenericGeometry::HasPlacementRequest(const G4int& copyNo) const {
    return detMotherVolumeNames.find(copyNo) != detMotherVolumeNames.end();
}

void GenericGeometry::ThrowMissingPlacementException(const char* methodName,
                                                     const char* errorCode,
                                                     const G4String& volumeDescription,
                                                     const G4int& copyNo) const {
    G4Exception(methodName,
                errorCode,
                FatalException,
                (volumeDescription + " with copy number " + std::to_string(copyNo) +
                 " in geometry " + geometryName + " was not found!").c_str());
}

void GenericGeometry::StoreRotation(const G4int& copyNo, const G4RotationMatrix& rotation) {
    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end()) {
        delete rotationIt->second;
        rotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);
}

void GenericGeometry::ApplyRotationMutation(const G4int& copyNo,
                                            const std::function<void(G4RotationMatrix&)>& mutation,
                                            const char* methodName,
                                            const char* errorCode) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }

    auto frames = GetPlacementFrames(copyNo);
    if (frames.empty()) {
        if (HasPlacementRequest(copyNo)) {
            auto* rotation = EnsureRotationMatrix(copyNo);
            mutation(*rotation);
            return;
        }
        ThrowMissingPlacementException(methodName, errorCode, "Frame physical volume", copyNo);
        return;
    }

    auto* rotation = EnsureRotationMatrix(copyNo);
    mutation(*rotation);
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetRotation(rotation);
        }
    }
    UpdateGeometry();
}

void GenericGeometry::ApplyTranslationToFramesOrRebuild(const G4int& copyNo,
                                                        const std::vector<G4VPhysicalVolume*>& frames,
                                                        const G4ThreeVector& translation) {
    detPosition[copyNo] = translation;
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetTranslation(translation);
        }
    }
}

void GenericGeometry::RotateTo(const G4int& copyNo, const G4double& theta, const G4double& phi, const G4double& psi) {
    ApplyRotationMutation(
        copyNo,
        [theta, phi, psi](G4RotationMatrix& rotation) {
            rotation.setTheta(theta);
            rotation.setPhi(phi);
            rotation.setPsi(psi);
        },
        "GenericGeometry::RotateTo",
        "RotateToError");
}

void GenericGeometry::RotateX(const G4int& copyNo, const G4double& delta) {
    ApplyRotationMutation(
        copyNo,
        [delta](G4RotationMatrix& rotation) { rotation.rotateX(delta); },
        "GenericGeometry::RotateX",
        "RotateXError");
}

void GenericGeometry::RotateY(const G4int& copyNo, const G4double& delta) {
    ApplyRotationMutation(
        copyNo,
        [delta](G4RotationMatrix& rotation) { rotation.rotateY(delta); },
        "GenericGeometry::RotateY",
        "RotateYError");
}

void GenericGeometry::RotateZ(const G4int& copyNo, const G4double& delta) {
    ApplyRotationMutation(
        copyNo,
        [delta](G4RotationMatrix& rotation) { rotation.rotateZ(delta); },
        "GenericGeometry::RotateZ",
        "RotateZError");
}

void GenericGeometry::AddGeometryTo(const G4String& volumeName, const G4int& copyNo) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }
    detMotherVolumeNames[copyNo] = volumeName;

    auto frameIt = detFrame.find(copyNo);
    if (frameIt != detFrame.end()) {
        RemoveGeometry(copyNo);
        detMotherVolumeNames[copyNo] = volumeName;
    }

    G4LogicalVolume* logicalVolume = ResolvePlacementLogicalVolume(volumeName);
    if (logicalVolume) {
        auto storedTransform = BuildStoredTransform(copyNo);
        AddGeometry(logicalVolume, &storedTransform, copyNo);
        fAreVolumensAssembled = !detFrame.empty();
        UpdateGeometry();
    }
}

void GenericGeometry::RemoveGeometry(const G4int& copyNo) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }
    detMotherVolumeNames.erase(copyNo);
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        G4GeometryManager* geoman = G4GeometryManager::GetInstance();
        geoman->OpenGeometry(frames.front());
        for (auto* frameVolume : frames) {
            delete frameVolume;
        }
        detFrame.erase(copyNo);
        detAuxFrames.erase(copyNo);
        OnAfterPlacementRemoval(copyNo);
        detPosition.erase(copyNo);
        auto rotationIt = detRotMat.find(copyNo);
        if (rotationIt != detRotMat.end()) {
            delete rotationIt->second;
            detRotMat.erase(rotationIt);
        }
        fAreVolumensAssembled = !detFrame.empty();
        geoman->CloseGeometry();
        UpdateGeometry();
    }
}

void GenericGeometry::TranslateTo(const G4int& copyNo, const G4ThreeVector& position) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        ApplyTranslationToFramesOrRebuild(copyNo, frames, position + det_origin);
        UpdateGeometry();
    } else {
        if (HasPlacementRequest(copyNo)) {
            detPosition[copyNo] = position + det_origin;
            return;
        }
        ThrowMissingPlacementException("GenericGeometry::TranslateTo",
                                       "TranslateToError",
                                       "Physical volume",
                                       copyNo);
    }
}

void GenericGeometry::Translate(const G4int& copyNo, const G4ThreeVector& delta) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        G4ThreeVector newPosition = frames.front()->GetTranslation() + delta;
        ApplyTranslationToFramesOrRebuild(copyNo, frames, newPosition);
        UpdateGeometry();
    } else {
        if (HasPlacementRequest(copyNo)) {
            const auto positionIt = detPosition.find(copyNo);
            const G4ThreeVector currentPosition =
                (positionIt != detPosition.end()) ? positionIt->second : det_origin;
            detPosition[copyNo] = currentPosition + delta;
            return;
        }
        ThrowMissingPlacementException("GenericGeometry::Translate",
                                       "TranslateError",
                                       "Physical volume",
                                       copyNo);
    }
}

void GenericGeometry::ApplyTransformation(const G4int& copyNo, const G4Transform3D& transform) {
    if (ShouldSkipWorkerGeometryMutation()) {
        return;
    }
    G4RotationMatrix rotation = transform.getRotation().inverse();
    G4ThreeVector translation = transform.getTranslation();
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        StoreRotation(copyNo, rotation);
        ApplyTranslationToFramesOrRebuild(copyNo, frames, translation);
        if (RequiresPlacementRebuild(copyNo)) {
            // ApplyTranslationToFramesOrRebuild has already rebuilt using the stored transform.
        } else {
            for (auto* frame : frames) {
                frame->SetRotation(detRotMat[copyNo]);
            }
        }
        UpdateGeometry();
    } else {
        if (HasPlacementRequest(copyNo)) {
            StoreRotation(copyNo, rotation);
            detPosition[copyNo] = translation;
            return;
        }
        ThrowMissingPlacementException("GenericGeometry::ApplyTransformation",
                                       "TransformationError",
                                       "Frame physical volume",
                                       copyNo);
    }
}

G4Material* GenericGeometry::GetMaterial(const G4String& matName) {
    auto it = detMat.find(matName);
    if (it != detMat.end()) {
        return it->second;
    } else {
        G4Exception("GenericGeometry::GetMaterial",
                    "MaterialNotFound",
                    FatalException,
                    ("Material " + matName + " in geometry " + geometryName + " was not found!").c_str());
        return nullptr; // This line will not be reached due to the exception
    }
}

G4VSolid* GenericGeometry::GetGeoVolume(const G4String& geoVolumeName) {
    auto it = detGeo.find(geoVolumeName);
    if (it != detGeo.end()) {
        return it->second;
    } else {
        G4Exception("GenericGeometry::GetGeoVolume",
                    "GeoVolumeNotFound",
                    FatalException,
                    ("Geometrical volume " + geoVolumeName + " in geometry " + geometryName + " was not found!").c_str());
        return nullptr; // This line will not be reached due to the exception
    }
}

G4LogicalVolume* GenericGeometry::GetLogVolume(const G4String& logVolumeName) {
    auto it = detLog.find(logVolumeName);
    if (it != detLog.end()) {
        return it->second;
    } else {
        G4Exception("GenericGeometry::GetLogVolume",
                    "LogVolumeNotFound",
                    FatalException,
                    ("Logical volume " + logVolumeName + " in geometry " + geometryName + " was not found!").c_str());
        return nullptr; // This line will not be reached due to the exception
    }
}

G4VPhysicalVolume* GenericGeometry::GetPhysVolume(const G4String& physVolumeName) {
    auto it = detPhys.find(physVolumeName);
    if (it != detPhys.end()) {
        return it->second;
    } else {
        G4Exception("GenericGeometry::GetPhysVolume",
                    "PhysVolumeNotFound",
                    FatalException,
                    ("Physical volume with copy number " + physVolumeName + " in geometry " + geometryName + " was not found!").c_str());
        return nullptr; // This line will not be reached due to the exception
    }
}

G4VPhysicalVolume* GenericGeometry::GetFrameVolume(const G4int& copyNo) {
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        return it->second;
    } else {
        G4Exception("GenericGeometry::GetFrameVolume",
                    "PhysVolumeNotFound",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return nullptr; // This line will not be reached due to the exception
    }
}

G4RotationMatrix* GenericGeometry::NewPtrRotMatrix(const G4RotationMatrix& rotationMatrix) {
    if (rotationMatrix.isIdentity()) {
        return nullptr;
    }
    return new G4RotationMatrix(rotationMatrix);
}

void GenericGeometry::UpdateGeometry() {
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    if (auto* visManager = dynamic_cast<G4VisManager*>(G4VVisManager::GetConcreteInstance());
        visManager != nullptr && visManager->GetCurrentViewer() != nullptr) {
        visManager->NotifyHandlers();
    }
}

G4bool GenericGeometry::HasPlacementRequests() const {
    return !detMotherVolumeNames.empty();
}

G4bool GenericGeometry::HasAssembledGeometry() const {
    return !detFrame.empty();
}

std::vector<G4int> GenericGeometry::GetPlacementCopyNumbers() const {
    std::vector<G4int> copyNumbers;
    copyNumbers.reserve(detMotherVolumeNames.size());
    for (const auto& [copyNo, volumeName] : detMotherVolumeNames) {
        (void)volumeName;
        copyNumbers.push_back(copyNo);
    }
    return copyNumbers;
}

void GenericGeometry::AssembleRequestedGeometries() {
    G4bool geometryAdded = false;
    for (const auto& [copyNo, volumeName] : detMotherVolumeNames) {
        if (detFrame.find(copyNo) != detFrame.end()) {
            continue;
        }

        G4LogicalVolume* logicalVolume = ResolvePlacementLogicalVolume(volumeName);
        if (logicalVolume == nullptr) {
            G4Exception("GenericGeometry::AssembleRequestedGeometries",
                        "AddGeometryToError",
                        FatalException,
                        ("Logical volume " + volumeName + " in geometry " + geometryName + " was not found!").c_str());
            return;
        }

        auto storedTransform = BuildStoredTransform(copyNo);
        AddGeometry(logicalVolume, &storedTransform, copyNo);
        geometryAdded = true;
    }

    fAreVolumensAssembled = !detFrame.empty();
    if (geometryAdded) {
        UpdateGeometry();
    }
}
