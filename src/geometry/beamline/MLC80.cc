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

#include "G4Material.hh"
#include "geometry/beamline/MLC80.hh"
#include "geometry/beamline/MLC80Messenger.hh"

#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4MaterialTable.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4GeometryManager.hh"
#include "G4BooleanSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4VSolid.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VisManager.hh"

#include "G4ios.hh"

MLC80::MLC80() {
    fNLeafs = 40; // Number of leafs per bank

   // default parameter values

  fLeafYPositionsA[0] = 20.*cm;
  fLeafYPositionsA[1] = 20.*cm;
  fLeafYPositionsA[2] = 20.*cm;
  fLeafYPositionsA[3] = 20.*cm;
  fLeafYPositionsA[4] = 20.*cm;
  fLeafYPositionsA[5] = 20.*cm;
  fLeafYPositionsA[6] = 20.*cm;
  fLeafYPositionsA[7] = 20.*cm;
  fLeafYPositionsA[8] = 20.*cm;
  fLeafYPositionsA[9] = 20.*cm;
  fLeafYPositionsA[10] = 20.*cm;
  fLeafYPositionsA[11] = 20.*cm;
  fLeafYPositionsA[12] = 20.*cm;
  fLeafYPositionsA[13] = 20.*cm;
  fLeafYPositionsA[14] = 20.*cm;
  fLeafYPositionsA[15] = 20.*cm;
  fLeafYPositionsA[16] = 20.*cm;
  fLeafYPositionsA[17] = 20.*cm;
  fLeafYPositionsA[18] = 20.*cm;
  fLeafYPositionsA[19] = 20.*cm;
  fLeafYPositionsA[20] = 20.*cm;
  fLeafYPositionsA[21] = 20.*cm;
  fLeafYPositionsA[22] = 20.*cm;
  fLeafYPositionsA[23] = 20.*cm;
  fLeafYPositionsA[24] = 20.*cm;
  fLeafYPositionsA[25] = 20.*cm;
  fLeafYPositionsA[26] = 20.*cm;
  fLeafYPositionsA[27] = 20.*cm;
  fLeafYPositionsA[28] = 20.*cm;
  fLeafYPositionsA[29] = 20.*cm;
  fLeafYPositionsA[30] = 20.*cm;
  fLeafYPositionsA[31] = 20.*cm;
  fLeafYPositionsA[32] = 20.*cm;
  fLeafYPositionsA[33] = 20.*cm;
  fLeafYPositionsA[34] = 20.*cm;
  fLeafYPositionsA[35] = 20.*cm;
  fLeafYPositionsA[36] = 20.*cm;
  fLeafYPositionsA[37] = 20.*cm;
  fLeafYPositionsA[38] = 20.*cm;
  fLeafYPositionsA[39] = 20.*cm;

  fLeafYPositionsB[0] = 20.*cm;
  fLeafYPositionsB[1] = 20.*cm;
  fLeafYPositionsB[2] = 20.*cm;
  fLeafYPositionsB[3] = 20.*cm;
  fLeafYPositionsB[4] = 20.*cm;
  fLeafYPositionsB[5] = 20.*cm;
  fLeafYPositionsB[6] = 20.*cm;
  fLeafYPositionsB[7] = 20.*cm;
  fLeafYPositionsB[8] = 20.*cm;
  fLeafYPositionsB[9] = 20.*cm;
  fLeafYPositionsB[10] = 20.*cm;
  fLeafYPositionsB[11] = 20.*cm;
  fLeafYPositionsB[12] = 20.*cm;
  fLeafYPositionsB[13] = 20.*cm;
  fLeafYPositionsB[14] = 20.*cm;
  fLeafYPositionsB[15] = 20.*cm;
  fLeafYPositionsB[16] = 20.*cm;
  fLeafYPositionsB[17] = 20.*cm;
  fLeafYPositionsB[18] = 20.*cm;
  fLeafYPositionsB[19] = 20.*cm;
  fLeafYPositionsB[20] = 20.*cm;
  fLeafYPositionsB[21] = 20.*cm;
  fLeafYPositionsB[22] = 20.*cm;
  fLeafYPositionsB[23] = 20.*cm;
  fLeafYPositionsB[24] = 20.*cm;
  fLeafYPositionsB[25] = 20.*cm;
  fLeafYPositionsB[26] = 20.*cm;
  fLeafYPositionsB[27] = 20.*cm;
  fLeafYPositionsB[28] = 20.*cm;
  fLeafYPositionsB[29] = 20.*cm;
  fLeafYPositionsB[30] = 20.*cm;
  fLeafYPositionsB[31] = 20.*cm;
  fLeafYPositionsB[32] = 20.*cm;
  fLeafYPositionsB[33] = 20.*cm;
  fLeafYPositionsB[34] = 20.*cm;
  fLeafYPositionsB[35] = 20.*cm;
  fLeafYPositionsB[36] = 20.*cm;
  fLeafYPositionsB[37] = 20.*cm;
  fLeafYPositionsB[38] = 20.*cm;
  fLeafYPositionsB[39] = 20.*cm;

  fMLC80Messenger = new MLC80Messenger(this);

}

void MLC80::DefineMaterials() {
    // Define materials
    G4double a; // atomic mass
    G4double z; // atomic number
    G4double density;
    G4String name;

    density = 18.0 * g / cm3;
    a = 183.85 * g / mole;
    G4Material* mat_Tungsten = new G4Material(name = "Tungsten", z = 74., a, density);

    // Store materials
    detMat["tungsten"] = mat_Tungsten;
}

void MLC80::DefineVolumes() {
    // Colors
    G4Colour cyan(0.0, 1.0, 1.0);

    // Rotation matrix for leaf end
    G4RotationMatrix* rotateLeaf = new G4RotationMatrix();
    rotateLeaf->rotateY(90.0 * deg);

    // Single leaf dimensions
    G4double preLeafDim_x = 2.715 * mm;
    G4double preLeafDim_y = 64.72066 * mm;
    G4double preLeafDim_z = 29.3 * mm;
    G4Box* preLeaf_box = new G4Box("preLeaf_box", preLeafDim_x, preLeafDim_y, preLeafDim_z);

    G4double preLeafPos_x = 0.0 * m;
    G4double preLeafPos_y = 0.0 * m;
    G4double preLeafPos_z = -50.0 * cm;
    G4DisplacedSolid* disPreLeaf = new G4DisplacedSolid("disPreLeaf", preLeaf_box, 0,
                                                        G4ThreeVector(preLeafPos_x, preLeafPos_y, preLeafPos_z));

    G4double innerRadiusOfTheLeafEnd = 0.01 * mm;
    G4double outerRadiusOfTheLeafEnd = 80.0 * mm;
    G4double hightOfTheLeafEnd = 2.715 * mm;
    G4double startAngleOfTheLeafEnd = 0.0 * deg;
    G4double spanningAngleOfTheLeafEnd = 180.0 * deg;
    G4Tubs* aLeafEnd = new G4Tubs("aLeafEnd", innerRadiusOfTheLeafEnd,
                                  outerRadiusOfTheLeafEnd, hightOfTheLeafEnd,
                                  startAngleOfTheLeafEnd, spanningAngleOfTheLeafEnd);

    G4double gapDim_x = 90.0 * mm;
    G4double gapDim_y = 74.18 * mm;
    G4double gapDim_z = 3.0 * mm;
    G4Box* gap = new G4Box("gap", gapDim_x, gapDim_y, gapDim_z);

    G4SubtractionSolid* leafEnd = new G4SubtractionSolid("leafEnd", aLeafEnd, gap);

    G4double leafEndPosX = 0.0 * cm;
    G4double leafEndPosY = -9.4413249 * mm;
    G4double leafEndPosZ = -50.0 * cm;

    G4DisplacedSolid* disLeafEnd = new G4DisplacedSolid("disLeafEnd", leafEnd, rotateLeaf,
                                                        G4ThreeVector(leafEndPosX, leafEndPosY, leafEndPosZ));

    G4UnionSolid* fullLeaf = new G4UnionSolid("fullLeaf", disPreLeaf, disLeafEnd);

    // Cut dimensions for the leaf
    G4double cutADim_x = 0.20 * mm;
    G4double cutADim_y = 68.0 * mm;
    G4double cutADim_z = 15.925 * mm;
    G4Box* cutA = new G4Box("cutA", cutADim_x, cutADim_y, cutADim_z);

    G4double cutAPosX = -2.515 * mm;
    G4double cutAPosY = 3.0 * mm;
    G4double cutAPosZ = -48.6625 * cm;
    G4DisplacedSolid* disCutA = new G4DisplacedSolid("disCutA", cutA, 0,
                                                     G4ThreeVector(cutAPosX, cutAPosY, cutAPosZ));

    G4SubtractionSolid* nearlyLeaf = new G4SubtractionSolid("nearlyLeaf", fullLeaf, disCutA);

    G4double cutBDim_x = 0.1625 * mm;
    G4double cutBDim_y = 68.0 * mm;
    G4double cutBDim_z = 14.415 * mm;
    G4Box* cutB = new G4Box("cutB", cutBDim_x, cutBDim_y, cutBDim_z);

    G4double cutBPosX = 2.5525 * mm;
    G4double cutBPosY = 3.0 * mm;
    G4double cutBPosZ = -51.4885 * cm;
    G4DisplacedSolid* disCutB = new G4DisplacedSolid("disCutB", cutB, 0,
                                                     G4ThreeVector(cutBPosX, cutBPosY, cutBPosZ));

    G4SubtractionSolid* leaf = new G4SubtractionSolid("leaf", nearlyLeaf, disCutB);

    // Define logical volume for the leaf
    detLog["leaf"] = new G4LogicalVolume(leaf, detMat["tungsten"], "leafLog", 0, 0, 0);

    G4VisAttributes* simpleTungstenSVisAtt = new G4VisAttributes(cyan);
    simpleTungstenSVisAtt->SetVisibility(true);
    simpleTungstenSVisAtt->SetForceSolid(true);
    detLog["leaf"]->SetVisAttributes(simpleTungstenSVisAtt);
}

void MLC80::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4ThreeVector pos(0, 0, 0);
    G4RotationMatrix* rot = new G4RotationMatrix;
    AddGeometry(motherVolume, pos, rot, copyNo);
}

void MLC80::AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& vec, G4RotationMatrix* pRot, G4int copyNo) {
    if (pRot == nullptr) pRot = new G4RotationMatrix;
    G4Transform3D transform(*pRot, vec);
    AddGeometry(motherVolume, &transform, copyNo);
}

void MLC80::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    G4double leafAPosX[41];
    G4double leafAPosYF[41];
    G4double leafAPosZ = -2.75 * cm;

    leafAPosX[0] = -111.335 * mm;

    G4int CopyNoA = 0;

    for (G4int i = 0; i < fNLeafs; ++i) {
        leafAPosYF[i] = -(((fLeafYPositionsA[i] * (100.0 - 47.25)) / 100.0) + 70.5587 * mm);
        leafAPosX[i + 1] = leafAPosX[i] + 5.431 * mm;
        new G4PVPlacement(0, G4ThreeVector(leafAPosX[i + 1], leafAPosYF[i], leafAPosZ),
                          detLog["leaf"], "leafA", motherVolume, false, CopyNoA);
        CopyNoA++;
    }

    // Rotation matrix for leaves B
    G4RotationMatrix* rotateLeavesB = new G4RotationMatrix();
    rotateLeavesB->rotateX(180.0 * deg);

    G4double leafBPosX[41];
    G4double leafBPosYF[41];
    G4double leafBPosZ = -102.75 * cm;

    leafBPosX[0] = -111.335 * mm;
    G4int CopyNoB = 0;

    for (G4int i = 0; i < fNLeafs; ++i) {
        leafBPosYF[i] = (((fLeafYPositionsB[i] * (100.0 - 47.25)) / 100.0) + 70.5587 * mm);
        leafBPosX[i + 1] = leafBPosX[i] + 5.431 * mm;
        new G4PVPlacement(rotateLeavesB, G4ThreeVector(leafBPosX[i + 1], leafBPosYF[i], leafBPosZ),
                          detLog["leaf"], "leafB", motherVolume, false, CopyNoB);
        CopyNoB++;
    }

    PrintParametersMLC();
}

void MLC80::PrintParametersMLC() {
    for (G4int i = 0; i < 40; ++i) {
        G4cout << "leaf A" << i << " position " << fLeafYPositionsA[i] / cm << " cm " << G4endl;
    }

    for (G4int i = 0; i < 40; ++i) {
        G4cout << "leaf B" << i << " position " << fLeafYPositionsB[i] / cm << " cm " << G4endl;
    }
}