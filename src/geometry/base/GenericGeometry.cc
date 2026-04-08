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
#include "G4RunManager.hh"
#include "G4Exception.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolumeStore.hh"

#include <algorithm>

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
    G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(motherVolumeName, false);
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

void GenericGeometry::RotateTo(const G4int& copyNo, const G4double& theta, const G4double& phi, const G4double& psi) {
    auto frames = GetPlacementFrames(copyNo);
    if (frames.empty()) {
        G4Exception("GenericGeometry::RotateTo",
                    "RotateToError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->setTheta(theta);
    rotation->setPhi(phi);
    rotation->setPsi(psi);
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetRotation(rotation);
        }
    }
    UpdateGeometry();
}

void GenericGeometry::RotateX(const G4int& copyNo, const G4double& delta) {
    auto frames = GetPlacementFrames(copyNo);
    if (frames.empty()) {
        G4Exception("GenericGeometry::RotateX",
                    "RotateXError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateX(delta);
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetRotation(rotation);
        }
    }
    UpdateGeometry();
}

void GenericGeometry::RotateY(const G4int& copyNo, const G4double& delta) {
    auto frames = GetPlacementFrames(copyNo);
    if (frames.empty()) {
        G4Exception("GenericGeometry::RotateY",
                    "RotateYError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateY(delta);
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetRotation(rotation);
        }
    }
    UpdateGeometry();
}

void GenericGeometry::RotateZ(const G4int& copyNo, const G4double& delta) {
    auto frames = GetPlacementFrames(copyNo);
    if (frames.empty()) {
        G4Exception("GenericGeometry::RotateZ",
                    "RotateZError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateZ(delta);
    if (RequiresPlacementRebuild(copyNo)) {
        RebuildPlacement(copyNo);
    } else {
        for (auto* frame : frames) {
            frame->SetRotation(rotation);
        }
    }
    UpdateGeometry();
}

void GenericGeometry::AddGeometryTo(const G4String& volumeName, const G4int& copyNo) {
    detMotherVolumeNames[copyNo] = volumeName;

    auto frameIt = detFrame.find(copyNo);
    if (frameIt != detFrame.end()) {
        RemoveGeometry(copyNo);
        detMotherVolumeNames[copyNo] = volumeName;
    }

    G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName, false);
    if (logicalVolume) {
        AddGeometry(logicalVolume, copyNo);
        fAreVolumensAssembled = !detFrame.empty();
        UpdateGeometry();
    }
}

void GenericGeometry::RemoveGeometry(const G4int& copyNo) {
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
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        detPosition[copyNo] = position + det_origin;
        if (RequiresPlacementRebuild(copyNo)) {
            RebuildPlacement(copyNo);
        } else {
            for (auto* frame : frames) {
                frame->SetTranslation(detPosition[copyNo]);
            }
        }
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::TranslateTo",
                    "TranslateToError",
                    FatalException,
                    ("Physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
    }
}

void GenericGeometry::Translate(const G4int& copyNo, const G4ThreeVector& delta) {
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        G4ThreeVector newPosition = frames.front()->GetTranslation() + delta;
        detPosition[copyNo] = newPosition;
        if (RequiresPlacementRebuild(copyNo)) {
            RebuildPlacement(copyNo);
        } else {
            for (auto* frame : frames) {
                frame->SetTranslation(newPosition);
            }
        }
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::Translate",
                    "TranslateError",
                    FatalException,
                    ("Physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
    }
}

void GenericGeometry::ApplyTransformation(const G4int& copyNo, const G4Transform3D& transform) {
    G4RotationMatrix rotation = transform.getRotation().inverse();
    G4ThreeVector translation = transform.getTranslation();
    auto frames = GetPlacementFrames(copyNo);
    if (!frames.empty()) {
        auto rotationIt = detRotMat.find(copyNo);
        if (rotationIt != detRotMat.end()) {
            delete rotationIt->second;
            rotationIt->second = nullptr;
        }
        detRotMat[copyNo] = NewPtrRotMatrix(rotation);
        detPosition[copyNo] = translation;
        if (RequiresPlacementRebuild(copyNo)) {
            RebuildPlacement(copyNo);
        } else {
            for (auto* frame : frames) {
                frame->SetRotation(detRotMat[copyNo]);
                frame->SetTranslation(translation);
            }
        }
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::ApplyTransformation",
                    "TransformationError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
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

 G4RotationMatrix* GenericGeometry::NewPtrRotMatrix(const G4RotationMatrix &RotMat)
{
    G4RotationMatrix *pRotMatrix; 
    if ( RotMat.isIdentity() ){
        pRotMatrix = 0; }
        else
        {
            pRotMatrix = new G4RotationMatrix(RotMat);
            }
            // fallocatedRotM= ! (RotMat.isIdentity());
        return pRotMatrix;
}

void GenericGeometry::UpdateGeometry() {
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    if (auto* visManager = G4VVisManager::GetConcreteInstance()) {
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

        G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName, false);
        if (logicalVolume == nullptr) {
            G4Exception("GenericGeometry::AssembleRequestedGeometries",
                        "AddGeometryToError",
                        FatalException,
                        ("Logical volume " + volumeName + " in geometry " + geometryName + " was not found!").c_str());
            return;
        }

        AddGeometry(logicalVolume, copyNo);
        geometryAdded = true;
    }

    fAreVolumensAssembled = !detFrame.empty();
    if (geometryAdded) {
        UpdateGeometry();
    }
}
