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

void GenericGeometry::RotateTo(const G4int& copyNo, const G4double& theta, const G4double& phi, const G4double& psi) {
    auto it = detFrame.find(copyNo);
    if (it == detFrame.end()) {
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
    it->second->SetRotation(rotation);
    UpdateGeometry();
}

void GenericGeometry::RotateX(const G4int& copyNo, const G4double& delta) {
    auto it = detFrame.find(copyNo);
    if (it == detFrame.end()) {
        G4Exception("GenericGeometry::RotateX",
                    "RotateXError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateX(delta);
    it->second->SetRotation(rotation);
    UpdateGeometry();
}

void GenericGeometry::RotateY(const G4int& copyNo, const G4double& delta) {
    auto it = detFrame.find(copyNo);
    if (it == detFrame.end()) {
        G4Exception("GenericGeometry::RotateY",
                    "RotateYError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateY(delta);
    it->second->SetRotation(rotation);
    UpdateGeometry();
}

void GenericGeometry::RotateZ(const G4int& copyNo, const G4double& delta) {
    auto it = detFrame.find(copyNo);
    if (it == detFrame.end()) {
        G4Exception("GenericGeometry::RotateZ",
                    "RotateZError",
                    FatalException,
                    ("Frame physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
        return;
    }

    G4RotationMatrix* rotation = EnsureRotationMatrix(copyNo);
    rotation->rotateZ(delta);
    it->second->SetRotation(rotation);
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
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        G4GeometryManager* geoman = G4GeometryManager::GetInstance();
        G4VPhysicalVolume* frameVolume = it->second;
        geoman->OpenGeometry(frameVolume);
        delete frameVolume;
        detFrame.erase(it);
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
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        detPosition[copyNo] = position + det_origin;
        it->second->SetTranslation(detPosition[copyNo]);
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::TranslateTo",
                    "TranslateToError",
                    FatalException,
                    ("Physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
    }
}

void GenericGeometry::Translate(const G4int& copyNo, const G4ThreeVector& delta) {
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        G4ThreeVector newPosition = it->second->GetTranslation() + delta;
        it->second->SetTranslation(newPosition);
        detPosition[copyNo] = newPosition;
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
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        auto rotationIt = detRotMat.find(copyNo);
        if (rotationIt != detRotMat.end()) {
            delete rotationIt->second;
            rotationIt->second = nullptr;
        }
        detRotMat[copyNo] = NewPtrRotMatrix(rotation);
        it->second->SetRotation(detRotMat[copyNo]);
        it->second->SetTranslation(translation);
        detPosition[copyNo] = translation;
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
