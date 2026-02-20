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
#include "GenericGeometry.hh"
#include "G4GeometryManager.hh"
#include "G4VisManager.hh"
#include "G4RunManager.hh"
#include "G4Exception.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolumeStore.hh"

GenericGeometry::GenericGeometry()
    : geometryName("GenericGeometry"),
      det_origin(G4ThreeVector()),
      fAreVolumensDefined(false),
      fAreVolumensAssembled(false) {}

void GenericGeometry::RotateTo(const G4int& copyNo, const G4double& theta, const G4double& phi, const G4double& psi) {
    detRotMat[copyNo] = new G4RotationMatrix();
    detRotMat[copyNo]->setTheta(theta);
    detRotMat[copyNo]->setPhi(phi);
    detRotMat[copyNo]->setPsi(psi);
    detFrame[copyNo]->SetRotation(detRotMat[copyNo]);
    UpdateGeometry();
}

void GenericGeometry::RotateX(const G4int& copyNo, const G4double& delta) {
    if(!detRotMat[copyNo]) detRotMat[copyNo] = new G4RotationMatrix();
    detRotMat[copyNo]->rotateX(delta);
    detFrame[copyNo]->SetRotation(detRotMat[copyNo]);
    UpdateGeometry();
}

void GenericGeometry::RotateY(const G4int& copyNo, const G4double& delta) {
    if(!detRotMat[copyNo]) detRotMat[copyNo] = new G4RotationMatrix();
    detRotMat[copyNo]->rotateY(delta);
    detFrame[copyNo]->SetRotation(detRotMat[copyNo]);
    UpdateGeometry();
}

void GenericGeometry::RotateZ(const G4int& copyNo, const G4double& delta) {
    if(!detRotMat[copyNo]) detRotMat[copyNo] = new G4RotationMatrix();
    detRotMat[copyNo]->rotateZ(delta);
    detFrame[copyNo]->SetRotation(detRotMat[copyNo]);
    UpdateGeometry();
}

void GenericGeometry::AddGeometryTo(const G4String& volumeName, const G4int& copyNo) {
    G4LogicalVolume* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(volumeName, false);
    if (logicalVolume) {
        AddGeometry(logicalVolume, copyNo);
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::AddGeometryTo",
                    "AddGeometryToError",
                    FatalException,
                    ("Logical volume " + volumeName + " in geometry " + geometryName + " was not found!").c_str());
    }
}

void GenericGeometry::RemoveGeometry(const G4int& copyNo) {
    auto it = detFrame.find(copyNo);
    if (it != detFrame.end()) {
        G4GeometryManager* geoman = G4GeometryManager::GetInstance();
        geoman->OpenGeometry(it->second);
        delete it->second;
        detFrame.erase(it);
        detPosition.erase(copyNo);
        detRotMat.erase(copyNo);
        geoman->CloseGeometry(it->second);
        UpdateGeometry();
    } else {
        G4Exception("GenericGeometry::RemoveGeometry",
                    "RemoveGeometryError",
                    FatalException,
                    ("Physical volume with copy number " + std::to_string(copyNo) + " in geometry " + geometryName + " was not found!").c_str());
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
        it->second->SetRotation(NewPtrRotMatrix(rotation));
        it->second->SetTranslation(translation);
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
    G4VisManager::GetInstance()->NotifyHandlers();
}
