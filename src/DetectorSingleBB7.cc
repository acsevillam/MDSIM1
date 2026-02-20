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

// Multidetector header files
#include "DetectorSingleBB7.hh"

// Geant4 header files
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4PVReplica.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VisManager.hh"

// Constructor implementation
DetectorSingleBB7::DetectorSingleBB7()
    : fStepLimit(nullptr), fNbOfStrips(32) {}

// Method to define the materials used in the detector
void DetectorSingleBB7::DefineMaterials() {
    if (fAreVolumensDefined) return;
    
    G4NistManager* nist = G4NistManager::Instance();
    
    G4Material* materialSi = nist->FindOrBuildMaterial("G4_Si");
    G4Material* materialAl = nist->FindOrBuildMaterial("G4_Al");
    G4Material* materialKapton = nist->FindOrBuildMaterial("G4_KAPTON");
    G4Material* materialAir = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* materialSiO2 = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
    G4Material* materialSin = new G4Material("SiliconNitride", 3.17*g/cm3, 2);
    materialSin->AddElement(nist->FindOrBuildElement("Si"), 1);
    materialSin->AddElement(nist->FindOrBuildElement("N"), 1);
    G4Material* materialSip = new G4Material("Polyimide", 1.42*g/cm3, 4);
    materialSip->AddElement(nist->FindOrBuildElement("H"), 10);
    materialSip->AddElement(nist->FindOrBuildElement("C"), 22);
    materialSip->AddElement(nist->FindOrBuildElement("N"), 2);
    materialSip->AddElement(nist->FindOrBuildElement("O"), 5);

    // Store materials in arrays
    detMat["Sin"] = materialSin;
    detMat["Sip"] = materialSip;
    detMat["Si"] = materialSi;
    detMat["Kapton"] = materialKapton;
    detMat["Al"] = materialAl;
    detMat["SiO2"] = materialSiO2;
    detMat["Air"] = materialAir;
}

// Method to define the geometrical volumes of the detector
void DetectorSingleBB7::DefineVolumes() {
    if (fAreVolumensDefined) return;
    DefineMaterials();

    // Geometrical volumes
    G4Box* geoSiO2Layer = new G4Box("SiO2Layer", 2.1*mm/2, 67.2*mm/2, 0.0003*mm/2);
    G4Box* geoAlTopLayer = new G4Box("AlTopLayer", 2.0*mm/2, 67.2*mm/2, 0.0003*mm/2);
    G4Box* geoSinLayer = new G4Box("SinLayer", 2.1*mm/2, 67.2*mm/2, 0.4994*mm/2);
    G4Box* geoSipLayer = new G4Box("SipLayer", 2.0*mm/2, 67.2*mm/2, 0.2497*mm/2);
    G4Box* geoSdCube = new G4Box("SdCube", 2.0*mm/2, 67.2*mm/2, (0.4994*mm - 0.2497*mm)/2);
    G4Box* geoAlBottomLayer = new G4Box("AlBottomLayer", 2.1*mm/2, 67.2*mm/2, 0.0003*mm/2);
    G4Box* geoKaptonLayer = new G4Box("KaptonLayer", 2.1*mm/2, 67.2*mm/2, 0.250*mm/2);
    G4Box* geoDeFrame = new G4Box("DeFrame", 2.1*mm/2, 67.2*mm/2, (0.0003*mm + 0.4994*mm + 0.0003*mm + 0.250*mm)/2);
    G4Box* geoDaFrame = new G4Box("DaFrame", 32 * 2.1*mm/2, 67.2*mm/2, geoDeFrame->GetZHalfLength());

    G4Box* geoSiO2FrameBox1 = new G4Box("SiO2FrameBox1", geoDaFrame->GetXHalfLength() + 1*mm, geoDaFrame->GetYHalfLength() + 1*mm, (0.0003*mm + 0.4994*mm + 0.0003*mm)/2);
    G4Box* geoSiO2FrameBox2 = new G4Box("SiO2FrameBox2", geoDaFrame->GetXHalfLength(), geoDaFrame->GetYHalfLength(), geoDaFrame->GetZHalfLength() * 2);
    G4SubtractionSolid* geoSiO2Frame = new G4SubtractionSolid("SiO2Frame", geoSiO2FrameBox1, geoSiO2FrameBox2);

    G4Box* geoKaptonFrameBox1 = new G4Box("KaptonFrameBox1", geoDaFrame->GetXHalfLength() + 5*mm, geoDaFrame->GetYHalfLength() + 5*mm, 0.250*mm/2);
    G4Box* geoKaptonFrameBox2 = new G4Box("KaptonFrameBox2", geoDaFrame->GetXHalfLength(), geoDaFrame->GetYHalfLength(), 0.250*mm);
    G4Box* geoKaptonFrameBox3 = new G4Box("KaptonFrameBox3", geoKaptonFrameBox1->GetXHalfLength(), geoKaptonFrameBox1->GetYHalfLength(), 1.45*mm/2);
    G4Box* geoKaptonFrameBox4 = new G4Box("KaptonFrameBox4", geoSiO2FrameBox1->GetXHalfLength(), geoSiO2FrameBox1->GetYHalfLength(), 1.45*mm);
    G4SubtractionSolid* geoKaptonFramePart1 = new G4SubtractionSolid("KaptonFrameP1", geoKaptonFrameBox1, geoKaptonFrameBox2);
    G4SubtractionSolid* geoKaptonFramePart2 = new G4SubtractionSolid("KaptonFrameP2", geoKaptonFrameBox3, geoKaptonFrameBox4);
    G4UnionSolid* geoKaptonFrame = new G4UnionSolid("KaptonFrame", geoKaptonFramePart1, geoKaptonFramePart2, nullptr, G4ThreeVector(0., 0., -geoKaptonFrameBox1->GetZHalfLength() + geoKaptonFrameBox3->GetZHalfLength()));

    G4Box* geoChipFramePart1 = new G4Box("ChipFrameP1", geoDaFrame->GetXHalfLength() + 5*mm, geoDaFrame->GetYHalfLength() + 5*mm, geoDaFrame->GetZHalfLength());
    G4UnionSolid* geoChipFrame = new G4UnionSolid("ChipFrame", geoChipFramePart1, geoKaptonFramePart2, nullptr, G4ThreeVector(0., 0., -geoChipFramePart1->GetZHalfLength() + geoKaptonFrameBox3->GetZHalfLength()));

    // Store geometrical volumes in arrays
    detGeo["SiO2Layer"] = geoSiO2Layer;
    detGeo["AlTopLayer"] = geoAlTopLayer;
    detGeo["SinLayer"] = geoSinLayer;
    detGeo["SipLayer"] = geoSipLayer;
    detGeo["SdCube"] = geoSdCube;
    detGeo["AlBottomLayer"] = geoAlBottomLayer;
    detGeo["KaptonLayer"] = geoKaptonLayer;
    detGeo["DeFrame"] = geoDeFrame;
    detGeo["DaFrame"] = geoDaFrame;
    detGeo["SiO2FrameBox1"] = geoSiO2FrameBox1;
    detGeo["SiO2Frame"] = geoSiO2Frame;
    detGeo["KaptonFrameBox1"] = geoKaptonFrameBox1;
    detGeo["KaptonFrame"] = geoKaptonFrame;
    detGeo["ChipFramePart1"] = geoChipFramePart1;
    detGeo["ChipFrame"] = geoChipFrame;

    // Logical volumes
    G4LogicalVolume* logSiO2Layer = new G4LogicalVolume(geoSiO2Layer, detMat["SiO2"], "SiO2Layer");
    G4LogicalVolume* logAlTopLayer = new G4LogicalVolume(geoAlTopLayer, detMat["Al"], "AlTopLayer");
    G4LogicalVolume* logSinLayer = new G4LogicalVolume(geoSinLayer, detMat["Sin"], "SinLayer");
    G4LogicalVolume* logSipLayer = new G4LogicalVolume(geoSipLayer, detMat["Sip"], "SipLayer");
    G4LogicalVolume* logSdCube = new G4LogicalVolume(geoSdCube, detMat["Si"], "SdCube");
    G4LogicalVolume* logAlBottomLayer = new G4LogicalVolume(geoAlBottomLayer, detMat["Al"], "AlBottomLayer");
    G4LogicalVolume* logKaptonLayer = new G4LogicalVolume(geoKaptonLayer, detMat["Kapton"], "KaptonLayer");
    G4LogicalVolume* logDeFrame = new G4LogicalVolume(geoDeFrame, detMat["Air"], "DeFrame");
    G4LogicalVolume* logDaFrame = new G4LogicalVolume(geoDaFrame, detMat["Air"], "DaFrame");
    G4LogicalVolume* logSiO2Frame = new G4LogicalVolume(geoSiO2Frame, detMat["SiO2"], "SiO2Frame");
    G4LogicalVolume* logKaptonFrame = new G4LogicalVolume(geoKaptonFrame, detMat["Kapton"], "KaptonFrame");
    G4LogicalVolume* logChipFrame = new G4LogicalVolume(geoChipFrame, GetMaterial("Kapton"), "ChipFrame");

    // Store logical volumes in arrays
    detLog["SiO2Layer"] = logSiO2Layer;
    detLog["AlTopLayer"] = logAlTopLayer;
    detLog["SinLayer"] = logSinLayer;
    detLog["SipLayer"] = logSipLayer;
    detLog["SdCube"] = logSdCube;
    detLog["AlBottomLayer"] = logAlBottomLayer;
    detLog["KaptonLayer"] = logKaptonLayer;
    detLog["DeFrame"] = logDeFrame;
    detLog["DaFrame"] = logDaFrame;
    detLog["SiO2Frame"] = logSiO2Frame;
    detLog["KaptonFrame"] = logKaptonFrame;
    detLog["ChipFrame"] = logChipFrame;

    // Visualization attributes
    G4VisAttributes* visAttr;
    visAttr = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logSiO2Layer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logAlTopLayer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logSinLayer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logSipLayer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logSdCube->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logAlBottomLayer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 1.0));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logKaptonLayer->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logDeFrame->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.3, 0.3, 0.3));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logDaFrame->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logSiO2Frame->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8));
    visAttr->SetVisibility(true);
    visAttr->SetForceSolid(true);
    logKaptonFrame->SetVisAttributes(visAttr);

    visAttr = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 1.0));
    visAttr->SetForceWireframe(true);
    visAttr->SetVisibility(true);
    logChipFrame->SetVisAttributes(visAttr);

    fAreVolumensDefined = true;
}

// Method to add geometry to the mother volume with a copy number
void DetectorSingleBB7::AddGeometry(G4LogicalVolume* motherVolume, G4int CopyNo) {
    G4ThreeVector pos(0, 0, 0);
    G4RotationMatrix* rot = new G4RotationMatrix;
    AddGeometry(motherVolume, pos, rot, CopyNo);
}

// Method to add geometry to the mother volume with a translation and rotation
void DetectorSingleBB7::AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& vec, G4RotationMatrix* pRot, G4int CopyNo) {
    if(pRot==nullptr) pRot = new G4RotationMatrix;
    G4Transform3D transform(*pRot, vec);
    AddGeometry(motherVolume, &transform, CopyNo);
}

// Method to add geometry to the mother volume with a transformation
void DetectorSingleBB7::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    auto geoChipFramePart1 = (G4Box*) (GetGeoVolume("ChipFramePart1"));
    
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }
    if(!fAreVolumensAssembled){
        // Place the volumes
        G4double posZ1, posZ2, posZ3;
        auto geoDaFrame = (G4Box*) (GetGeoVolume("DaFrame"));
        posZ1 = geoDaFrame->GetZHalfLength();

        auto geoSiO2Layer = (G4Box*) (GetGeoVolume("SiO2Layer"));
        posZ1 -= geoSiO2Layer->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ1),                   // Translation position
            GetLogVolume("SiO2Layer"),                    // Logical volume
            "SiO2Layer",                                  // Name
            GetLogVolume("DeFrame"),                      // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, 0),                       // Translation position
            GetLogVolume("AlTopLayer"),                   // Logical volume
            "AlTopLayer",                                 // Name
            GetLogVolume("SiO2Layer"),                    // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoSinLayer = (G4Box*) (GetGeoVolume("SinLayer"));
        posZ1 -= geoSiO2Layer->GetZHalfLength() + geoSinLayer->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ1),                   // Translation position
            GetLogVolume("SinLayer"),                     // Logical volume
            "SinLayer",                                   // Name
            GetLogVolume("DeFrame"),                      // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoSipLayer = (G4Box*) (GetGeoVolume("SipLayer"));
        posZ2 = geoSinLayer->GetZHalfLength();
        posZ2 -= geoSipLayer->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ2),                   // Translation position
            GetLogVolume("SipLayer"),                     // Logical volume
            "SipLayer",                                   // Name
            GetLogVolume("SinLayer"),                     // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoSdCube = (G4Box*) (GetGeoVolume("SdCube"));
        posZ2 -= geoSipLayer->GetZHalfLength() + geoSdCube->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ2),                   // Translation position
            GetLogVolume("SdCube"),                       // Logical volume
            "SdCube",                                     // Name
            GetLogVolume("SinLayer"),                     // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoAlBottomLayer = (G4Box*) (GetGeoVolume("AlBottomLayer"));
        posZ1 -= geoSinLayer->GetZHalfLength() + geoAlBottomLayer->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ1),                   // Translation position
            GetLogVolume("AlBottomLayer"),                // Logical volume
            "AlBottomLayer",                              // Name
            GetLogVolume("DeFrame"),                      // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoKaptonLayer = (G4Box*) (GetGeoVolume("KaptonLayer"));
        posZ1 -= geoAlBottomLayer->GetZHalfLength() + geoKaptonLayer->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ1),                   // Translation position
            GetLogVolume("KaptonLayer"),                  // Logical volume
            "KaptonLayer",                                // Name
            GetLogVolume("DeFrame"),                      // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoDeFrame = (G4Box*) (GetGeoVolume("DeFrame"));
        new G4PVReplica(
            "DeFrame",                                    // Name
            GetLogVolume("DeFrame"),                      // Logical volume
            GetLogVolume("DaFrame"),                      // Mother logical volume
            kXAxis,                                       // Axis of replication
            fNbOfStrips,                                  // Number of copies
            2 * geoDeFrame->GetXHalfLength(),             // Width of replica
            0);                                           // Offset

        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, 0),                       // Translation position
            GetLogVolume("DaFrame"),                      // Logical volume
            "DaFrame",                                    // Name
            GetLogVolume("ChipFrame"),                    // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoSiO2FrameBox1 = (G4Box*) (GetGeoVolume("SiO2FrameBox1"));
        posZ3 = geoChipFramePart1->GetZHalfLength();
        posZ3 -= geoSiO2FrameBox1->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ3),                   // Translation position
            GetLogVolume("SiO2Frame"),                    // Logical volume
            "SiO2Frame",                                  // Name
            GetLogVolume("ChipFrame"),                    // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        auto geoKaptonFrameBox1 = (G4Box*) (GetGeoVolume("KaptonFrameBox1"));
        posZ3 -= geoSiO2FrameBox1->GetZHalfLength() + geoKaptonFrameBox1->GetZHalfLength();
        new G4PVPlacement(
            nullptr,                                      // No rotation
            G4ThreeVector(0, 0, posZ3),                   // Translation position
            GetLogVolume("KaptonFrame"),                  // Logical volume
            "KaptonFrame",                                // Name
            GetLogVolume("ChipFrame"),                    // Mother logical volume
            false,                                        // No boolean operations
            0,                                            // Copy number
            true);                                        // Check overlaps

        fAreVolumensAssembled = true;
    }

    G4ThreeVector det_origin = G4ThreeVector(0., 0., geoChipFramePart1->GetZHalfLength());  
    G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    detFrame[copyNo] = new G4PVPlacement(
        finalTransform,                               // Rotation and translation
        GetLogVolume("ChipFrame"),                    // Logical volume
        "ChipFrame",                                  // Name
        motherVolume,                                 // Mother logical volume
        false,                                        // No boolean operations
        copyNo,                                       // Copy number
        true);                                        // Check overlaps

}