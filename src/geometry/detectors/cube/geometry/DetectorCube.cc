/*
 *
 * Geant4 MultiDetector Simulation
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
#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

// MultiDetector Headers
#include "geometry/detectors/cube/geometry/DetectorCube.hh"

DetectorCube::DetectorCube(G4double cubeSide,
                           const G4String& materialName,
                           G4double calibrationFactor,
                           G4double calibrationFactorError)
    : fCubeSide(cubeSide),
      fMaterialName(materialName),
      fCalibrationFactor(calibrationFactor),
      fCalibrationFactorError(calibrationFactorError),
      fDetectorCubeMessenger(nullptr) {
    geometryName = "DetectorCube";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorCubeMessenger = new DetectorCubeMessenger(this);
}

DetectorCube::~DetectorCube() {
    delete fDetectorCubeMessenger;
    fDetectorCubeMessenger = nullptr;
}

void DetectorCube::DefineMaterials() {
    if (detMat.find(fMaterialName) != detMat.end()) {
        return;
    }

    auto* nistManager = G4NistManager::Instance();
    auto* material = nistManager->FindOrBuildMaterial(fMaterialName, false);
    if (material == nullptr) {
        G4Exception("DetectorCube::DefineMaterials",
                    "DetectorCubeInvalidMaterial",
                    FatalException,
                    ("Material " + fMaterialName + " was not found in the NIST database.").c_str());
        return;
    }

    detMat[fMaterialName] = material;
}

void DetectorCube::DefineVolumes() {
    if (fAreVolumensDefined) {
        return;
    }

    DefineMaterials();

    auto* geoDetectorCube = new G4Box("DetectorCube", fCubeSide / 2., fCubeSide / 2., fCubeSide / 2.);
    detGeo["DetectorCube"] = geoDetectorCube;

    auto* logDetectorCube = new G4LogicalVolume(geoDetectorCube, detMat[fMaterialName], "DetectorCube");
    auto* visDetectorCube = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
    visDetectorCube->SetVisibility(true);
    visDetectorCube->SetForceSolid(true);
    logDetectorCube->SetVisAttributes(visDetectorCube);
    detLog["DetectorCube"] = logDetectorCube;

    fAreVolumensDefined = true;
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    auto* rotation = new G4RotationMatrix();
    AddGeometry(motherVolume, G4ThreeVector(), rotation, copyNo);
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume,
                               const G4ThreeVector& position,
                               G4RotationMatrix* rotation,
                               G4int copyNo) {
    if (rotation == nullptr) {
        rotation = new G4RotationMatrix();
    }
    G4Transform3D transform(*rotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    fAreVolumensAssembled = true;
    G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    detFrame[copyNo] = new G4PVPlacement(
        finalTransform,
        detLog["DetectorCube"],
        "DetectorCube_phys",
        motherVolume,
        false,
        copyNo,
        true);
}
