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

#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"

#include "geometry/phantoms/PhantomWaterTube.hh"
#include "geometry/phantoms/PhantomWaterTubeMessenger.hh"

namespace {

void ValidatePositiveLength(const char* methodName, G4double value, const char* parameterName) {
    if (value <= 0.) {
        G4Exception(methodName,
                    "InvalidPhantomDimension",
                    FatalException,
                    (G4String(parameterName) + " must be > 0.").c_str());
    }
}

} // namespace

PhantomWaterTube::PhantomWaterTube(G4double waterTubeRadius, G4double waterTubeHeight)
    : fStepLimit(nullptr),
      fWaterTubeRadius(waterTubeRadius),
      fWaterTubeHeight(waterTubeHeight),
      fPhantomWaterTubeMessenger(nullptr) {
    geometryName = "PhantomWaterTube";
    fPhantomWaterTubeMessenger = new PhantomWaterTubeMessenger(this);
    det_origin = G4ThreeVector(0., 0., -10. * cm);
}

PhantomWaterTube::~PhantomWaterTube() {
    delete fPhantomWaterTubeMessenger;
    fPhantomWaterTubeMessenger = nullptr;
}

void PhantomWaterTube::SetWaterTubeRadius(G4double waterTubeRadius) {
    ValidatePositiveLength("PhantomWaterTube::SetWaterTubeRadius",
                           waterTubeRadius,
                           "waterTube radius");
    fWaterTubeRadius = waterTubeRadius;
    InvalidateGeometryDefinition();
}

void PhantomWaterTube::SetWaterTubeHeight(G4double waterTubeHeight) {
    ValidatePositiveLength("PhantomWaterTube::SetWaterTubeHeight",
                           waterTubeHeight,
                           "waterTube height");
    fWaterTubeHeight = waterTubeHeight;
    InvalidateGeometryDefinition();
}

void PhantomWaterTube::InvalidateGeometryDefinition() {
    detGeo.clear();
    detLog.clear();
    fAreVolumensDefined = false;
}

void PhantomWaterTube::DefineMaterials() {
    auto* nist = G4NistManager::Instance();
    auto* materialWater = nist->FindOrBuildMaterial("G4_WATER");
    detMat["Water"] = materialWater;
}

void PhantomWaterTube::DefineVolumes() {
    if (fAreVolumensDefined) {
        return;
    }

    DefineMaterials();

    auto* geoWaterTube =
        new G4Tubs("WaterTube", 0., fWaterTubeRadius, fWaterTubeHeight / 2., 0., 360. * deg);
    detGeo["WaterTube"] = geoWaterTube;

    auto* logWaterTube = new G4LogicalVolume(geoWaterTube, detMat["Water"], "WaterTube");

    auto* visWaterTube = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.2));
    visWaterTube->SetVisibility(true);
    visWaterTube->SetForceLineSegmentsPerCircle(100);
    visWaterTube->SetForceSolid(true);
    logWaterTube->SetVisAttributes(visWaterTube);

    detLog["WaterTube"] = logWaterTube;
    fAreVolumensDefined = true;
}

void PhantomWaterTube::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4ThreeVector position(0., 0., 0.);
    auto* rotation = new G4RotationMatrix;
    AddGeometry(motherVolume, position, rotation, copyNo);
}

void PhantomWaterTube::AddGeometry(G4LogicalVolume* motherVolume,
                                   const G4ThreeVector& position,
                                   G4RotationMatrix* rotation,
                                   G4int copyNo) {
    if (rotation == nullptr) {
        rotation = new G4RotationMatrix;
    }
    G4Transform3D transform(*rotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void PhantomWaterTube::AddGeometry(G4LogicalVolume* motherVolume,
                                   G4Transform3D* transformation,
                                   G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }
    fAreVolumensAssembled = true;

    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    detFrame[copyNo] = new G4PVPlacement(finalTransform,
                                         detLog["WaterTube"],
                                         "WaterTube",
                                         motherVolume,
                                         false,
                                         copyNo,
                                         true);
}
