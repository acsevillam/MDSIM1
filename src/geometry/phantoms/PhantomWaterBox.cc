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
#include "G4Exception.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VisManager.hh"
#include "G4GenericMessenger.hh"

// MultiDetector Headers
#include "geometry/phantoms/PhantomWaterBox.hh"

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

PhantomWaterBox::PhantomWaterBox(G4double fWaterBoxDx)
    : fStepLimit(nullptr),
      fWaterBoxDx(fWaterBoxDx),
      fWaterBoxDy(fWaterBoxDx),
      fWaterBoxDz(fWaterBoxDx),
      fPhantomWaterBoxMessenger(nullptr) {
    geometryName = "PhantomWaterBox";
    fPhantomWaterBoxMessenger = new PhantomWaterBoxMessenger(this) ;
    det_origin = G4ThreeVector(0., 0., -10*cm);
}

PhantomWaterBox::~PhantomWaterBox() {
    delete fPhantomWaterBoxMessenger;
    fPhantomWaterBoxMessenger = nullptr;
}

void PhantomWaterBox::SetWaterBoxSide(G4double waterBoxSide) {
    ValidatePositiveLength("PhantomWaterBox::SetWaterBoxSide", waterBoxSide, "waterBox side");
    fWaterBoxDx = waterBoxSide;
    fWaterBoxDy = waterBoxSide;
    fWaterBoxDz = waterBoxSide;
    InvalidateGeometryDefinition();
}

void PhantomWaterBox::SetWaterBoxSize(const G4ThreeVector& waterBoxSize) {
    ValidatePositiveLength("PhantomWaterBox::SetWaterBoxSize", waterBoxSize.x(), "waterBox sizeX");
    ValidatePositiveLength("PhantomWaterBox::SetWaterBoxSize", waterBoxSize.y(), "waterBox sizeY");
    ValidatePositiveLength("PhantomWaterBox::SetWaterBoxSize", waterBoxSize.z(), "waterBox sizeZ");
    fWaterBoxDx = waterBoxSize.x();
    fWaterBoxDy = waterBoxSize.y();
    fWaterBoxDz = waterBoxSize.z();
    InvalidateGeometryDefinition();
}

void PhantomWaterBox::InvalidateGeometryDefinition() {
    detGeo.clear();
    detLog.clear();
    fAreVolumensDefined = false;
}

// Method to define the materials used in the phantom water box
void PhantomWaterBox::DefineMaterials() {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* materialWater = nist->FindOrBuildMaterial("G4_WATER");

    // Store materials in arrays
    detMat["Water"] = materialWater;
}

// Method to define the geometrical volumes of the phantom water box
void PhantomWaterBox::DefineVolumes() {
    if (fAreVolumensDefined) return;
    DefineMaterials();

    // Geometrical volumes
    G4Box* geoWaterBox =
        new G4Box("WaterBox", fWaterBoxDx / 2, fWaterBoxDy / 2, fWaterBoxDz / 2);

    // Store geometrical volumes in arrays
    detGeo["WaterBox"] = geoWaterBox;

    // Logical volumes
    G4LogicalVolume* logWaterBox = new G4LogicalVolume(geoWaterBox, detMat["Water"], "WaterBox");

    // Visualization attributes
    G4VisAttributes* visWaterBox = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.2));
    visWaterBox->SetVisibility(true);
    logWaterBox->SetVisAttributes(visWaterBox);

    // Store logical volumes in arrays
    detLog["WaterBox"] = logWaterBox;

    fAreVolumensDefined=true;
}

// Method to add geometry to the mother volume with a copy number
void PhantomWaterBox::AddGeometry(G4LogicalVolume* motherVolume, G4int CopyNo) {
    G4ThreeVector pos(0, 0, 0);
    G4RotationMatrix* rot = new G4RotationMatrix;
    AddGeometry(motherVolume, pos, rot, CopyNo);
}

// Method to add geometry to the mother volume with a translation and rotation
void PhantomWaterBox::AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& vec, G4RotationMatrix* pRot, G4int CopyNo) {
    if(pRot==nullptr) pRot = new G4RotationMatrix;
    G4Transform3D transform(*pRot, vec);
    AddGeometry(motherVolume, &transform, CopyNo);
}

// Method to add geometry to the mother volume with a transformation
void PhantomWaterBox::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    // Ensure materials and volumes are defined
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }
    fAreVolumensAssembled=true;

    G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    // Place the volume
    detFrame[copyNo] = new G4PVPlacement(
        finalTransform,                               // Rotation and translation
        detLog["WaterBox"],                           // Logical volume
        "WaterBox",                                   // Name
        motherVolume,                                 // Mother logical volume
        false,                                        // No boolean operations
        copyNo,                                       // Copy number
        true);                                        // Check overlaps
}
