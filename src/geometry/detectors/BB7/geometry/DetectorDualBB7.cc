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
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4PVReplica.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4GeometryManager.hh"
#include "G4RunManager.hh"
#include "G4VVisManager.hh"
#include "G4Transform3D.hh"
#include "G4PhysicalVolumeStore.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"

DetectorDualBB7::DetectorDualBB7() 
    : fStepLimit(nullptr) {
    geometryName = "DetectorDualBB7";
    fDetectorSingleBB7 = new DetectorSingleBB7();
    fDetectorSingleBB7->DefineVolumes();
    fDetectorDualBB7Messenger = new DetectorDualBB7Messenger(this) ;
    det_origin = G4ThreeVector(0., 0., 10*cm);
}

void DetectorDualBB7::SetCalibrationFactor(G4int detectorID, G4double calibrationFactor) {
    fCalibrationParametersByDetector[detectorID].calibrationFactor = calibrationFactor;
}

void DetectorDualBB7::SetCalibrationFactorError(G4int detectorID, G4double calibrationFactorError) {
    fCalibrationParametersByDetector[detectorID].calibrationFactorError = calibrationFactorError;
}

BB7CalibrationParameters DetectorDualBB7::GetCalibrationParameters(G4int detectorID) const {
    const auto it = fCalibrationParametersByDetector.find(detectorID);
    if (it != fCalibrationParametersByDetector.end()) {
        return it->second;
    }
    return {};
}

void DetectorDualBB7::DefineMaterials() {
    if (fAreVolumensDefined) return;
    // Get NIST material manager
    G4NistManager* nistManager = G4NistManager::Instance();
    G4Material* materialAir = nistManager->FindOrBuildMaterial("G4_AIR");
    // Store materials
    detMat["Air"] = materialAir;
}

void DetectorDualBB7::DefineVolumes() {
    if (fAreVolumensDefined) return;
    DefineMaterials();
    
    auto geoDaFrame = (G4Box*) fDetectorSingleBB7->GetGeoVolume("DaFrame");
    auto geoKaptonFrameBox1 = (G4Box*) fDetectorSingleBB7->GetGeoVolume("KaptonFrameBox1");
    auto geoSiO2FrameBox1 = (G4Box*) fDetectorSingleBB7->GetGeoVolume("SiO2FrameBox1");

    G4Box* geoDetectorFrameBox1 = new G4Box("DetectorFrameBox1", geoKaptonFrameBox1->GetXHalfLength(), geoKaptonFrameBox1->GetYHalfLength(), 1.45*mm);
    G4Box* geoDetectorFrameBox2 = new G4Box("DetectorFrameBox2", geoSiO2FrameBox1->GetXHalfLength(), geoSiO2FrameBox1->GetYHalfLength(), 1.45*mm*2);
    G4SubtractionSolid* geoDetectorFramePart1 = new G4SubtractionSolid("KaptonFrameP1", geoDetectorFrameBox1, geoDetectorFrameBox2);

    G4Box* geoDetectorFramePart2 = new G4Box("detFrameP1", geoDaFrame->GetXHalfLength() + 5*mm, geoDaFrame->GetYHalfLength() + 5*mm, geoDaFrame->GetZHalfLength()*2);
    G4UnionSolid* geoDetFrame = new G4UnionSolid("detFrame", geoDetectorFramePart2, geoDetectorFramePart1, nullptr, G4ThreeVector(0., 0., 0.));
    
    // Logical volumes
    G4LogicalVolume* logDetFrame = new G4LogicalVolume(geoDetFrame, GetMaterial("Air"), "detFrame");
    G4VisAttributes* visDetFrame = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0));
    visDetFrame->SetVisibility(false);
    logDetFrame->SetVisAttributes(visDetFrame);

    detLog["detFrame"] = logDetFrame;
   
}

// Method to add geometry to the mother volume with a copy number
void DetectorDualBB7::AddGeometry(G4LogicalVolume* motherVolume, G4int CopyNo) {
    G4ThreeVector pos(0, 0, 0);
    G4RotationMatrix* rot = new G4RotationMatrix;
    AddGeometry(motherVolume, pos, rot, CopyNo);
}

// Method to add geometry to the mother volume with a translation and rotation
void DetectorDualBB7::AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& vec, G4RotationMatrix* pRot, G4int CopyNo) {
    if(pRot==nullptr) pRot = new G4RotationMatrix;
    G4Transform3D transform(*pRot, vec);
    AddGeometry(motherVolume, &transform, CopyNo);
}

// Method to add geometry to the mother volume with a transformation
void DetectorDualBB7::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {

    if(!fAreVolumensDefined){
        DefineVolumes();
    }

    // Rotation matrix for the second detector
    G4RotationMatrix* rotMat1 = new G4RotationMatrix();

    // Transform for the first detector
    G4Transform3D transform1(G4Translate3D(G4ThreeVector(0., 0., 0.)) * G4Rotate3D(*rotMat1));
    // Create and add the first single BB7 detector
    fDetectorSingleBB7->AddGeometry(GetLogVolume("detFrame"), &transform1, 0);

    // Rotation matrix for the second detector
    G4RotationMatrix* rotMat2 = new G4RotationMatrix();
    rotMat2->rotateX(180 * deg);
    rotMat2->rotateZ(90 * deg);
        
    // Transform for the second detector
    G4Transform3D transform2(G4Translate3D(G4ThreeVector(0., 0., 0.)) * G4Rotate3D(*rotMat2));

    // Create and add the second single BB7 detector
    fDetectorSingleBB7->AddGeometry(GetLogVolume("detFrame"), &transform2, 1);

    G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    detFrame[copyNo] = new G4PVPlacement(
        finalTransform,                     // Rotation and translation
        GetLogVolume("detFrame"),           // Logical volume
        "detFrame_phys",                    // Name
        motherVolume,                       // Mother logical volume
        false,                              // No boolean operations
        copyNo,                             // Copy number
        true);                              // Check overlaps

    if (auto* visman = G4VVisManager::GetConcreteInstance()) {
        visman->NotifyHandlers();
    }

}
