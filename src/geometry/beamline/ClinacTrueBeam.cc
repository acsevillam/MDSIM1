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
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4PVReplica.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4ProductionCuts.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VisManager.hh"
#include "G4GenericMessenger.hh"
#include "G4RunManager.hh"

// MultiDetector Headers
#include "geometry/beamline/ClinacTrueBeam.hh"
#include "geometry/beamline/MLC80.hh"
#include "MD1PrimaryGeneratorAction1.hh"

using namespace MD1;

/**
 * @brief Constructor for the ClinacTrueBeam class.
 */
ClinacTrueBeam::ClinacTrueBeam()
    : fStepLimit(nullptr), fRot(nullptr) {
    geometryName = "ClinacTrueBeam";
    fJaw1XAperture = 0 * cm;
    fJaw2XAperture = 0 * cm;
    fJaw1YAperture = 0 * cm;
    fJaw2YAperture = 0 * cm;
    fJawXPenumbraCorrection = 1.2;
    fJawYPenumbraCorrection = 1.2;
    fGantryAngle = 0 * deg;
    fCollimatorAngle = 0 * deg;
    fIsocenterDistance = 100 * cm;
    fReferenceDistance = 100 * cm;
    fClinacTrueBeamMessenger = new ClinacTrueBeamMessenger(this) ;
    det_origin = G4ThreeVector(0., 0., 0*cm);
}

/**
 * @brief Defines the materials used in the model.
 */
void ClinacTrueBeam::DefineMaterials() {
    G4NistManager* nist = G4NistManager::Instance();

    G4Material* el_H = nist->FindOrBuildMaterial("G4_H");
    G4Material* el_C = nist->FindOrBuildMaterial("G4_C");
    G4Material* el_N = nist->FindOrBuildMaterial("G4_N");
    G4Material* el_O = nist->FindOrBuildMaterial("G4_O");
    G4Material* el_Si = nist->FindOrBuildMaterial("G4_Si");
    G4Material* el_Cr = nist->FindOrBuildMaterial("G4_Cr");
    G4Material* el_Mn = nist->FindOrBuildMaterial("G4_Mn");
    G4Material* el_Fe = nist->FindOrBuildMaterial("G4_Fe");
    G4Material* el_Ni = nist->FindOrBuildMaterial("G4_Ni");
    G4Material* el_Cu = nist->FindOrBuildMaterial("G4_Cu");
    G4Material* el_W = nist->FindOrBuildMaterial("G4_W");
    G4Material* el_Pb = nist->FindOrBuildMaterial("G4_Pb");

    G4Material* mat_Air = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* mat_Water = nist->FindOrBuildMaterial("G4_WATER");

    G4Material* mat_Ssteel = new G4Material("StainlessSteel", 7.8 * g/cm3, 6);
    mat_Ssteel->AddMaterial(el_Fe, 0.6898);
    mat_Ssteel->AddMaterial(el_Cr, 0.18);
    mat_Ssteel->AddMaterial(el_Ni, 0.10);
    mat_Ssteel->AddMaterial(el_Mn, 0.02);
    mat_Ssteel->AddMaterial(el_Si, 0.01);
    mat_Ssteel->AddMaterial(el_C, 0.0002);

    G4Material* mat_XC10 = new G4Material("CARBON_STEEL", 7.8 * g/cm3, 3);
    mat_XC10->AddMaterial(el_Fe, 0.993);
    mat_XC10->AddMaterial(el_Mn, 0.006);
    mat_XC10->AddMaterial(el_C, 0.001);

    G4Material* mat_WNICU = new G4Material("Denal(WNICU)", 16.8 * g/cm3, 3);
    mat_WNICU->AddMaterial(el_W, 0.905);
    mat_WNICU->AddMaterial(el_Ni, 0.07);
    mat_WNICU->AddMaterial(el_Cu, 0.025);

    G4Material* mat_Kapton = new G4Material("Kapton", 1.42 * g/cm3, 4);
    mat_Kapton->AddMaterial(el_C, 69.1133 * perCent);
    mat_Kapton->AddMaterial(el_O, 20.9235 * perCent);
    mat_Kapton->AddMaterial(el_N, 7.3270 * perCent);
    mat_Kapton->AddMaterial(el_H, 2.6362 * perCent);

    // Store materials
    detMat["StainlessSteel"] = mat_Ssteel;
    detMat["xc10"] = mat_XC10;
    detMat["wnicu"] = mat_WNICU;
    detMat["W"] = el_W;
    detMat["Fe"] = el_Fe;
    detMat["Cu"] = el_Cu;
    detMat["Pb"] = el_Pb;
    detMat["air"] = mat_Air;
    detMat["water"] = mat_Water;
}

void ClinacTrueBeam::DefineVolumes() {
    if (fAreVolumensDefined) return;
    DefineMaterials();
    fAreVolumensDefined = true;
}

void ClinacTrueBeam::ConstructAccelerator(G4LogicalVolume* motherVolume, G4int copyNo) {

    G4bool checkOverlap = true;
    G4ThreeVector initialCentre = det_origin + G4ThreeVector(fIsocenterDistance*std::sin(fGantryAngle), 0. * mm , fIsocenterDistance*std::cos(fGantryAngle));
    G4RotationMatrix* RotMatGantry = new G4RotationMatrix();
    RotMatGantry->rotateY(fGantryAngle);
    G4RotationMatrix* RotMatCollimator = new G4RotationMatrix();
    RotMatCollimator->rotateZ(fCollimatorAngle);
    G4Transform3D transform(G4Translate3D(initialCentre) * G4Rotate3D(*RotMatGantry) * G4Rotate3D(*RotMatCollimator));

    G4Tubs* accWorldTubs1 = new G4Tubs("accWorldTubs1", 0, 300. * mm, 600. * mm, 0, 360 * deg);
    G4Tubs* accWorldTubs2 = new G4Tubs("accWorldTubs2", 0, 500. * mm, 600. * mm, 0, 360 * deg);
    G4SubtractionSolid* geoAccFrameTubs = new G4SubtractionSolid("accFrameTubs", accWorldTubs1, accWorldTubs2, 0, G4ThreeVector(0, 0, 500 * mm));

    detGeo["accWorldTubs1"] = accWorldTubs1;

    G4LogicalVolume* accWorldLV = new G4LogicalVolume(geoAccFrameTubs, detMat["air"], "accWorldL", 0, 0, 0);
    detFrame[copyNo] = new G4PVPlacement(transform, accWorldLV, "acceleratorTubs", motherVolume, false, 0, checkOverlap);

    G4VisAttributes* simpleAlSVisAtt = new G4VisAttributes(G4Colour::Gray());
    simpleAlSVisAtt->SetForceSolid(true);
    simpleAlSVisAtt->SetVisibility(false);
    accWorldLV->SetVisAttributes(simpleAlSVisAtt);

    ConstructJawsX(accWorldLV, copyNo);
    ConstructJawsY(accWorldLV, copyNo);
    ConstructBasePlate(accWorldLV, copyNo);
    ConstructMLC(accWorldLV, copyNo);
    ConstructBiasVolume(accWorldLV, copyNo);
}

/**
 * @brief Constructs the jaws along the X-axis.
 * @param motherVolume Logical volume of the mother.
 */
void ClinacTrueBeam::ConstructJawsX(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4bool checkOverlap = false;

    G4double boxSide = 201. * mm;
    G4double box1HalfLengthZ = 3. * mm;
    G4double box2HalfLengthZ = 25. * mm;
    G4double box3HalfLengthZ = 35. * mm;
    G4double box4HalfLengthZ = 27. * mm;

    G4Material* el_Pb = GetMaterial("Pb");
    G4Material* XC10 = GetMaterial("xc10");
    G4Material* WNICU = GetMaterial("wnicu");

    G4ThreeVector boxJawXSide = {boxSide, boxSide, 100. * mm};

    G4Box* boxJaw1X = new G4Box("Jaw1Xbox", boxJawXSide.getX() / 2., boxJawXSide.getY() / 2., boxJawXSide.getZ() / 2.);
    G4LogicalVolume* boxJaw1XLV = new G4LogicalVolume(boxJaw1X, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "Jaw1XboxLV", 0, 0, 0);
    G4Box* boxJaw2X = new G4Box("Jaw2Xbox", boxJawXSide.getX() / 2., boxJawXSide.getY() / 2., boxJawXSide.getZ() / 2.);
    G4LogicalVolume* boxJaw2XLV = new G4LogicalVolume(boxJaw2X, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "Jaw2XboxLV", 0, 0, 0);

    fJaw1XInitialPos.setX(boxJawXSide.getX() / 2.);
    fJaw1XInitialPos.setY(0.);
    fJaw1XInitialPos.setZ(235.5 * mm + boxJawXSide.getZ() / 2.);

    G4RotationMatrix* cRotationJaw1X = new G4RotationMatrix();
    cRotationJaw1X->rotateY(std::fabs(std::atan((fJaw1XAperture * fJawXPenumbraCorrection) / (-(fReferenceDistance)))));

    G4ThreeVector position = *cRotationJaw1X * -fJaw1XInitialPos;
    *cRotationJaw1X = cRotationJaw1X->inverse();

    auto boxJaw1X_phys = new G4PVPlacement(cRotationJaw1X, position, boxJaw1XLV, "Jaw1XPV", motherVolume, false, 0, checkOverlap);

    fJaw2XInitialPos.setX(-fJaw1XInitialPos.getX());
    fJaw2XInitialPos.setY(fJaw1XInitialPos.getY());
    fJaw2XInitialPos.setZ(fJaw1XInitialPos.getZ());

    G4RotationMatrix* cRotationJaw2X = new G4RotationMatrix();
    cRotationJaw2X->rotateY(-std::fabs(std::atan((fJaw2XAperture * fJawXPenumbraCorrection) / (-(fReferenceDistance)))));

    position = *cRotationJaw2X * -fJaw2XInitialPos;
    *cRotationJaw2X = cRotationJaw2X->inverse();

    auto boxJaw2X_phys = new G4PVPlacement(cRotationJaw2X, position, boxJaw2XLV, "Jaw2XPV", motherVolume, false, 0, checkOverlap);

    // Physical volumes
    G4Box* box1 = new G4Box("Jaws1XBox1", boxSide / 2., boxSide / 2., box1HalfLengthZ / 2.);
    G4Box* box2 = new G4Box("Jaws1XBox2", boxSide / 2., boxSide / 2., box2HalfLengthZ / 2.);
    G4Box* box3 = new G4Box("Jaws1XBox3", boxSide / 2., boxSide / 2., box3HalfLengthZ / 2.);
    G4Box* box4 = new G4Box("Jaws1XBox4", boxSide / 2., boxSide / 2., box4HalfLengthZ / 2.);
    G4LogicalVolume* box1LV1 = new G4LogicalVolume(box1, XC10, "Jaws1XLV1", 0, 0, 0);
    G4LogicalVolume* box2LV1 = new G4LogicalVolume(box2, el_Pb, "Jaws1XLV2", 0, 0, 0);
    G4LogicalVolume* box3LV1 = new G4LogicalVolume(box3, WNICU, "Jaws1XLV3", 0, 0, 0);
    G4LogicalVolume* box4LV1 = new G4LogicalVolume(box4, el_Pb, "Jaws1XLV4", 0, 0, 0);

    G4LogicalVolume* box1LV2 = new G4LogicalVolume(box1, XC10, "Jaws2XLV1", 0, 0, 0);
    G4LogicalVolume* box2LV2 = new G4LogicalVolume(box2, el_Pb, "Jaws2XLV2", 0, 0, 0);
    G4LogicalVolume* box3LV2 = new G4LogicalVolume(box3, WNICU, "Jaws2XLV3", 0, 0, 0);
    G4LogicalVolume* box4LV2 = new G4LogicalVolume(box4, el_Pb, "Jaws2XLV4", 0, 0, 0);

    G4ThreeVector centre = {0., 0., 0.};
    G4double zCentreCurrentBox = -boxJawXSide.getZ() / 2. + box1HalfLengthZ / 2. ;

    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1XPV1", box1LV1, boxJaw1X_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2XPV1", box1LV2, boxJaw2X_phys, false, 0, checkOverlap);

    zCentreCurrentBox += (box1HalfLengthZ + box2HalfLengthZ) / 2. ;
    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1XPV2", box2LV1, boxJaw1X_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2XPV2", box2LV2, boxJaw2X_phys, false, 0, checkOverlap);

    zCentreCurrentBox += (box2HalfLengthZ + box3HalfLengthZ) / 2. ;
    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1XPV3", box3LV1, boxJaw1X_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2XPV3", box3LV2, boxJaw2X_phys, false, 0, checkOverlap);

    zCentreCurrentBox += (box3HalfLengthZ + box4HalfLengthZ) / 2. ;
    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1XPV4", box4LV1, boxJaw1X_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2XPV4", box4LV2, boxJaw2X_phys, false, 0, checkOverlap);

    // Visualization
    G4VisAttributes* visAtt = new G4VisAttributes(G4Colour::Yellow());
    visAtt->SetVisibility(false);
    visAtt->SetForceSolid(false);

    G4VisAttributes* simpleAlSVisAttPb = new G4VisAttributes(G4Colour::Blue());
    simpleAlSVisAttPb->SetVisibility(true);
    simpleAlSVisAttPb->SetForceSolid(true);

    G4VisAttributes* simpleAlSVisAttXC10 = new G4VisAttributes(G4Colour::Green());
    simpleAlSVisAttXC10->SetVisibility(true);
    simpleAlSVisAttXC10->SetForceSolid(true);

    G4VisAttributes* simpleAlSVisAttWNICU = new G4VisAttributes(G4Colour::Red());
    simpleAlSVisAttWNICU->SetVisibility(true);
    simpleAlSVisAttWNICU->SetForceSolid(true);

    box1LV1->SetVisAttributes(simpleAlSVisAttXC10);
    box2LV1->SetVisAttributes(simpleAlSVisAttPb);
    box3LV1->SetVisAttributes(simpleAlSVisAttWNICU);
    box4LV1->SetVisAttributes(simpleAlSVisAttPb);

    box1LV2->SetVisAttributes(simpleAlSVisAttXC10);
    box2LV2->SetVisAttributes(simpleAlSVisAttPb);
    box3LV2->SetVisAttributes(simpleAlSVisAttWNICU);
    box4LV2->SetVisAttributes(simpleAlSVisAttPb);

    boxJaw1XLV->SetVisAttributes(visAtt);
    boxJaw2XLV->SetVisAttributes(visAtt);

    // Region for cuts
    G4Region* regVol = new G4Region("JawsXR");
    G4ProductionCuts* cuts = new G4ProductionCuts();
    cuts->SetProductionCut(2. * cm);
    regVol->SetProductionCuts(cuts);
    box1LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box1LV1);
    box2LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box2LV1);
    box3LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box3LV1);
    box4LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box4LV1);

    box1LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box1LV2);
    box2LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box2LV2);
    box3LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box3LV2);
    box4LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box4LV2);

    detPhys["Jaw1X_" + std::to_string(copyNo)] = boxJaw1X_phys;
    detPhys["Jaw2X_" + std::to_string(copyNo)] = boxJaw2X_phys;
}

void ClinacTrueBeam::ConstructJawsY(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4bool checkOverlap = false;

    G4double boxSide = 201. * mm;
    G4double box1HalfLengthZ = 15. * mm;
    G4double box2HalfLengthZ = 30. * mm;
    G4double box3HalfLengthZ = 35. * mm;
    G4double box4HalfLengthZ = 20. * mm;

    G4Material* el_Pb = GetMaterial("Pb");
    G4Material* XC10 = GetMaterial("xc10");
    G4Material* WNICU = GetMaterial("wnicu");

    G4ThreeVector boxJawYSide = {boxSide, boxSide, 100 * mm};

    G4Box* boxJaw1Y = new G4Box("Jaw1Ybox", boxJawYSide.getX() / 2., boxJawYSide.getY() / 2., boxJawYSide.getZ() / 2.);
    G4LogicalVolume* boxJaw1YLV = new G4LogicalVolume(boxJaw1Y, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "Jaw1YboxLV", 0, 0, 0);
    G4Box* boxJaw2Y = new G4Box("Jaw2Ybox", boxJawYSide.getX() / 2., boxJawYSide.getY() / 2., boxJawYSide.getZ() / 2.);
    G4LogicalVolume* boxJaw2YLV = new G4LogicalVolume(boxJaw2Y, G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"), "Jaw2YboxLV", 0, 0, 0);

    fJaw1YInitialPos.setX(0.);
    fJaw1YInitialPos.setY(boxJawYSide.getX() / 2.);
    fJaw1YInitialPos.setZ((235.5 + 117.5) * mm + boxJawYSide.getZ() / 2.);

    G4RotationMatrix* cRotationJaw1Y = new G4RotationMatrix();
    cRotationJaw1Y->rotateX(-std::fabs(std::atan((fJaw1YAperture * fJawYPenumbraCorrection) / (-(fReferenceDistance)))));

    G4ThreeVector position = *cRotationJaw1Y * -fJaw1YInitialPos;
    *cRotationJaw1Y = cRotationJaw1Y->inverse();

    auto boxJaw1Y_phys = new G4PVPlacement(cRotationJaw1Y, position, boxJaw1YLV, "Jaw1YPV", motherVolume, false, 0, checkOverlap);

    fJaw2YInitialPos.setX(fJaw1YInitialPos.getX());
    fJaw2YInitialPos.setY(-fJaw1YInitialPos.getY());
    fJaw2YInitialPos.setZ(fJaw1YInitialPos.getZ());

    G4RotationMatrix* cRotationJaw2Y = new G4RotationMatrix();
    cRotationJaw2Y->rotateX(std::fabs(std::atan((fJaw2YAperture * fJawYPenumbraCorrection) / (-(fReferenceDistance)))));

    position = *cRotationJaw2Y * -fJaw2YInitialPos;
    *cRotationJaw2Y = cRotationJaw2Y->inverse();

    auto boxJaw2Y_phys = new G4PVPlacement(cRotationJaw2Y, position, boxJaw2YLV, "Jaw2YPV", motherVolume, false, 0, checkOverlap);

    // Physical volumes
    G4Box *box1 = new G4Box("Jaws1YBox1", boxSide / 2. , boxSide / 2., box1HalfLengthZ / 2.);
    G4Box* box2 = new G4Box("Jaws1YBox2", boxSide / 2., boxSide / 2., box2HalfLengthZ / 2.);
    G4Box* box3 = new G4Box("Jaws1YBox3", boxSide / 2., boxSide / 2., box3HalfLengthZ / 2.);
    G4Box* box4 = new G4Box("Jaws1YBox4", boxSide / 2., boxSide / 2., box4HalfLengthZ / 2.);
    G4LogicalVolume* box1LV1 = new G4LogicalVolume(box1, XC10,  "Jaws1YLV1", 0,0,0);
    G4LogicalVolume* box2LV1 = new G4LogicalVolume(box2, el_Pb, "Jaws1YLV2", 0, 0, 0);
    G4LogicalVolume* box3LV1 = new G4LogicalVolume(box3, WNICU, "Jaws1YLV3", 0, 0, 0);
    G4LogicalVolume* box4LV1 = new G4LogicalVolume(box4, el_Pb, "Jaws1YLV4", 0, 0, 0);

    G4LogicalVolume* box1LV2 = new G4LogicalVolume(box1, XC10,  "Jaws2YLV1", 0,0,0);
    G4LogicalVolume* box2LV2 = new G4LogicalVolume(box2, el_Pb, "Jaws2YLV2", 0, 0, 0);
    G4LogicalVolume* box3LV2 = new G4LogicalVolume(box3, WNICU, "Jaws2YLV3", 0, 0, 0);
    G4LogicalVolume* box4LV2 = new G4LogicalVolume(box4, el_Pb, "Jaws2YLV4", 0, 0, 0);

    G4ThreeVector centre = {0., 0., 0.};
	G4double zCentreCurrentBox = -boxJawYSide.getZ() / 2. + box2HalfLengthZ / 2. ;

	centre.setZ(zCentreCurrentBox);
    //zCentreCurrentBox += (box1HalfLengthZ + box2HalfLengthZ) / 2. + 117.5 * mm;

    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1YPV2", box2LV1, boxJaw1Y_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2YPV2", box2LV2, boxJaw2Y_phys, false, 0, checkOverlap);

    zCentreCurrentBox += (box2HalfLengthZ + box3HalfLengthZ) / 2.;
    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1YPV3", box3LV1, boxJaw1Y_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2YPV3", box3LV2, boxJaw2Y_phys, false, 0, checkOverlap);

    zCentreCurrentBox += (box3HalfLengthZ + box4HalfLengthZ) / 2. ;
    centre.setZ(zCentreCurrentBox);
    new G4PVPlacement(nullptr, centre, "Jaws1YPV4", box4LV1, boxJaw1Y_phys, false, 0, checkOverlap);
    new G4PVPlacement(nullptr, centre, "Jaws2YPV4", box4LV2, boxJaw2Y_phys, false, 0, checkOverlap);

    // Visualization
    G4VisAttributes* visAtt = new G4VisAttributes(G4Colour::Yellow());
    visAtt->SetVisibility(false);
    visAtt->SetForceSolid(false);

    G4VisAttributes* simpleAlSVisAttPb = new G4VisAttributes(G4Colour::Blue());
    simpleAlSVisAttPb->SetVisibility(true);
    simpleAlSVisAttPb->SetForceSolid(true);

    G4VisAttributes* simpleAlSVisAttXC10 = new G4VisAttributes(G4Colour::Green());
    simpleAlSVisAttXC10->SetVisibility(true);
    simpleAlSVisAttXC10->SetForceSolid(true);

    G4VisAttributes* simpleAlSVisAttWNICU = new G4VisAttributes(G4Colour::Red());
    simpleAlSVisAttWNICU->SetVisibility(true);
    simpleAlSVisAttWNICU->SetForceSolid(true);

    box2LV1->SetVisAttributes(simpleAlSVisAttPb);
    box3LV1->SetVisAttributes(simpleAlSVisAttWNICU);
    box4LV1->SetVisAttributes(simpleAlSVisAttPb);

    box2LV2->SetVisAttributes(simpleAlSVisAttPb);
    box3LV2->SetVisAttributes(simpleAlSVisAttWNICU);
    box4LV2->SetVisAttributes(simpleAlSVisAttPb);

    boxJaw1YLV->SetVisAttributes(visAtt);
    boxJaw2YLV->SetVisAttributes(visAtt);

    // Region for cuts
    G4Region* regVol = new G4Region("JawsYR");
    G4ProductionCuts* cuts = new G4ProductionCuts();
    cuts->SetProductionCut(2. * cm);
    regVol->SetProductionCuts(cuts);

    box2LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box2LV1);
    box3LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box3LV1);
    box4LV1->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box4LV1);

    box2LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box2LV2);
    box3LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box3LV2);
    box4LV2->SetRegion(regVol);
    regVol->AddRootLogicalVolume(box4LV2);

    detPhys["Jaw1Y_" + std::to_string(copyNo)] = boxJaw1Y_phys;
    detPhys["Jaw2Y_" + std::to_string(copyNo)] = boxJaw2Y_phys;

}

void ClinacTrueBeam::ConstructBasePlate(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4bool checkOverlap = true;

    G4Tubs* basePlateTubs = new G4Tubs("basePlate1", 0, 300. * mm, 20 / 2. * mm, 0, 360 * deg);
    G4Box* basePlateBox = new G4Box("Jaws1XBox1", 210 / 2. * mm, 210 / 2. * mm, 40 / 2. * mm);
    G4SubtractionSolid* geoBasePlate = new G4SubtractionSolid("basePlate", basePlateTubs, basePlateBox, 0, G4ThreeVector(0, 0, 0));
    G4LogicalVolume* logBasePlate = new G4LogicalVolume(geoBasePlate, detMat["xc10"], "basePlate", 0, 0, 0);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -470) * mm, logBasePlate, "basePlate", motherVolume, false, 0, checkOverlap);

    G4VisAttributes* simpleAlSVisAtt = new G4VisAttributes(G4Colour::Yellow());
    logBasePlate->SetVisAttributes(simpleAlSVisAtt);
}

void ClinacTrueBeam::ConstructMLC(G4LogicalVolume* motherVolume, G4int copyNo) {
    MLC80* mLC80 = new MLC80();
    mLC80->DefineMaterials();
    mLC80->DefineVolumes();
    mLC80->AddGeometry(motherVolume, copyNo);
}

void ClinacTrueBeam::ConstructBiasVolume(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4bool checkOverlap = true;

    G4Tubs* geoBiasTubs = new G4Tubs("biasTubs", 0, 300. * mm, 4 / 2. * mm, 0, 360 * deg);
    G4LogicalVolume* logBiasTubs = new G4LogicalVolume(geoBiasTubs, detMat["air"], "biasTubs", 0, 0, 0);
    detLog["biasTubs"] = logBiasTubs;

    G4VisAttributes* simpleAlSVisAtt = new G4VisAttributes(G4Colour(0., 0.0, 0.1, 0.05));
    simpleAlSVisAtt->SetForceSolid(true);
    simpleAlSVisAtt->SetVisibility(true);
    logBiasTubs->SetVisAttributes(simpleAlSVisAtt);

    G4Tubs* geoAccFrameTubs = (G4Tubs*)detGeo["accWorldTubs1"];

    G4ThreeVector centre = {0., 0., 0.};
    G4double zCentreCurrentTubs = -geoAccFrameTubs->GetZHalfLength() + geoBiasTubs->GetZHalfLength();
    centre.setZ(zCentreCurrentTubs);
    new G4PVPlacement(nullptr, centre, logBiasTubs, "biasTubs", motherVolume, false, 0, checkOverlap);
    zCentreCurrentTubs += 2 * geoBiasTubs->GetZHalfLength();
    centre.setZ(zCentreCurrentTubs);
    new G4PVPlacement(nullptr, centre, logBiasTubs, "biasTubs", motherVolume, false, 1, checkOverlap);
    zCentreCurrentTubs += 2 * geoBiasTubs->GetZHalfLength();
    centre.setZ(zCentreCurrentTubs);
    new G4PVPlacement(nullptr, centre, logBiasTubs, "biasTubs", motherVolume, false, 2, checkOverlap);
    zCentreCurrentTubs += 2 * geoBiasTubs->GetZHalfLength();
    centre.setZ(zCentreCurrentTubs);
    new G4PVPlacement(nullptr, centre, logBiasTubs, "biasTubs", motherVolume, false, 3, checkOverlap);
    zCentreCurrentTubs += 2 * geoBiasTubs->GetZHalfLength();
    centre.setZ(zCentreCurrentTubs);
    new G4PVPlacement(nullptr, centre, logBiasTubs, "biasTubs", motherVolume, false, 4, checkOverlap);
}

void ClinacTrueBeam::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4ThreeVector pos(0, 0, 0);
    G4RotationMatrix* rot = new G4RotationMatrix;
    AddGeometry(motherVolume, pos, rot, copyNo);
}

void ClinacTrueBeam::AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& position, G4RotationMatrix* rotation, G4int copyNo) {
    if (rotation == nullptr) rotation = new G4RotationMatrix;
    G4Transform3D transform(*rotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void ClinacTrueBeam::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    ConstructAccelerator(motherVolume,copyNo);
}

void ClinacTrueBeam::SetJaw1X(const G4int& copyNo, const G4double& value) {
    fJaw1XAperture = value;
    auto it = detPhys.find("Jaw1X_" + std::to_string(copyNo));
    if (it == detPhys.end()) {
        return;
    }

    G4RotationMatrix* cRotationJaw1X = new G4RotationMatrix();
    cRotationJaw1X->rotateY(std::fabs(std::atan((fJaw1XAperture * fJawXPenumbraCorrection) / (-(fReferenceDistance)))));
    G4ThreeVector newPosition = *cRotationJaw1X * -fJaw1XInitialPos;
    *cRotationJaw1X = cRotationJaw1X->inverse();
    it->second->SetTranslation(newPosition);
    it->second->SetRotation(cRotationJaw1X);
    G4cout << "Jaw1X position: " << "[" << newPosition.x() << ", " <<newPosition.y() << ", "  << newPosition.z() << "]" << G4endl;
    G4cout << "Jaw1X rotation angle: " << cRotationJaw1X->getDelta() / deg << "deg" << "axis = [" << cRotationJaw1X->getAxis().getX() << ", "  << cRotationJaw1X->getAxis().getY() << ", "  << cRotationJaw1X->getAxis().getZ() << "]" << G4endl;
    UpdateGeometry();
}

void ClinacTrueBeam::SetJaw2X(const G4int& copyNo, const G4double& value) {
    fJaw2XAperture = value;
    auto it = detPhys.find("Jaw2X_" + std::to_string(copyNo));
    if (it == detPhys.end()) {
        return;
    }

    G4RotationMatrix* cRotationJaw2X = new G4RotationMatrix();
    cRotationJaw2X->rotateY(-std::fabs(std::atan((fJaw2XAperture * fJawXPenumbraCorrection) / (-(fReferenceDistance)))));
    G4ThreeVector newPosition = *cRotationJaw2X * -fJaw2XInitialPos;
    *cRotationJaw2X = cRotationJaw2X->inverse();
    it->second->SetTranslation(newPosition);
    it->second->SetRotation(cRotationJaw2X);
    G4cout << "Jaw2X position: " << "[" << newPosition.x() << ", " <<newPosition.y() << ", "  << newPosition.z() << "]" << G4endl;
    G4cout << "Jaw2X rotation angle: " << cRotationJaw2X->getDelta() / deg << "deg" << "axis = [" << cRotationJaw2X->getAxis().getX() << ", "  << cRotationJaw2X->getAxis().getY() << ", "  << cRotationJaw2X->getAxis().getZ() << "]" << G4endl;
    UpdateGeometry();
}

void ClinacTrueBeam::SetJaw1Y(const G4int& copyNo, const G4double& value) {
    fJaw1YAperture = value;
    auto it = detPhys.find("Jaw1Y_" + std::to_string(copyNo));
    if (it == detPhys.end()) {
        return;
    }

    G4RotationMatrix* cRotationJaw1Y = new G4RotationMatrix();
    cRotationJaw1Y->rotateX(-std::fabs(std::atan((fJaw1YAperture * fJawYPenumbraCorrection) / (-(fReferenceDistance)))));
    G4ThreeVector newPosition = *cRotationJaw1Y * -fJaw1YInitialPos;
    *cRotationJaw1Y = cRotationJaw1Y->inverse();
    it->second->SetTranslation(newPosition);
    it->second->SetRotation(cRotationJaw1Y);
    G4cout << "Jaw1Y position: " << "[" << newPosition.x() << ", " <<newPosition.y() << ", "  << newPosition.z() << "]" << G4endl;
    G4cout << "Jaw1Y rotation angle: " << cRotationJaw1Y->getDelta() / deg << "deg" << "axis = [" << cRotationJaw1Y->getAxis().getX() << ", "  << cRotationJaw1Y->getAxis().getY() << ", "  << cRotationJaw1Y->getAxis().getZ() << "]" << G4endl;
    UpdateGeometry();
}

void ClinacTrueBeam::SetJaw2Y(const G4int& copyNo, const G4double& value) {
    fJaw2YAperture = value;
    auto it = detPhys.find("Jaw2Y_" + std::to_string(copyNo));
    if (it == detPhys.end()) {
        return;
    }

    G4RotationMatrix* cRotationJaw2Y = new G4RotationMatrix();
    cRotationJaw2Y->rotateX(std::fabs(std::atan((fJaw2YAperture * fJawYPenumbraCorrection) / (-(fReferenceDistance)))));
    G4ThreeVector newPosition = *cRotationJaw2Y * -fJaw2YInitialPos;
    *cRotationJaw2Y = cRotationJaw2Y->inverse();
    it->second->SetTranslation(newPosition);
    it->second->SetRotation(cRotationJaw2Y);
    G4cout << "Jaw2Y position: " << "[" << newPosition.x() << ", " <<newPosition.y() << ", "  << newPosition.z() << "]" << G4endl;
    G4cout << "Jaw2Y rotation angle: " << cRotationJaw2Y->getDelta() / deg << "deg" << "axis = [" << cRotationJaw2Y->getAxis().getX() << ", "  << cRotationJaw2Y->getAxis().getY() << ", "  << cRotationJaw2Y->getAxis().getZ() << "]" << G4endl;
    UpdateGeometry();
}

void ClinacTrueBeam::RotateGantryTo(const G4int& copyNo, const G4double& angle) {
    fGantryAngle = angle;
    if (detFrame.find(copyNo) == detFrame.end()) {
        return;
    }

    G4ThreeVector initialCentre = det_origin + G4ThreeVector(fIsocenterDistance*std::sin(fGantryAngle), 0. * mm , fIsocenterDistance*std::cos(fGantryAngle));
    G4RotationMatrix* RotMatGantry = new G4RotationMatrix();
    RotMatGantry->rotateY(fGantryAngle);
    G4RotationMatrix* RotMatCollimator = new G4RotationMatrix();
    RotMatCollimator->rotateZ(fCollimatorAngle);
    G4Transform3D transform(G4Translate3D(initialCentre) * G4Rotate3D(*RotMatGantry) * G4Rotate3D(*RotMatCollimator));
    ApplyTransformation(copyNo,transform);
}

void ClinacTrueBeam::RotateGantry(const G4int& copyNo, const G4double& delta) {
    fGantryAngle += delta;
    if (detFrame.find(copyNo) == detFrame.end()) {
        return;
    }

    G4ThreeVector initialCentre = det_origin + G4ThreeVector(fIsocenterDistance*std::sin(fGantryAngle), 0. * mm , fIsocenterDistance*std::cos(fGantryAngle));
    G4RotationMatrix* RotMatGantry = new G4RotationMatrix();
    RotMatGantry->rotateY(fGantryAngle);
    G4RotationMatrix* RotMatCollimator = new G4RotationMatrix();
    RotMatCollimator->rotateZ(fCollimatorAngle);
    G4Transform3D transform(G4Translate3D(initialCentre) * G4Rotate3D(*RotMatGantry) * G4Rotate3D(*RotMatCollimator));
    ApplyTransformation(copyNo,transform);
}

void ClinacTrueBeam::RotateCollimatorTo(const G4int& copyNo, const G4double& angle) {
    fCollimatorAngle = angle;
    if (detFrame.find(copyNo) == detFrame.end()) {
        return;
    }

    G4ThreeVector initialCentre = det_origin + G4ThreeVector(fIsocenterDistance*std::sin(fGantryAngle), 0. * mm , fIsocenterDistance*std::cos(fGantryAngle));
    G4RotationMatrix* RotMatGantry = new G4RotationMatrix();
    RotMatGantry->rotateY(fGantryAngle);
    G4RotationMatrix* RotMatCollimator = new G4RotationMatrix();
    RotMatCollimator->rotateZ(fCollimatorAngle);
    G4Transform3D transform(G4Translate3D(initialCentre) * G4Rotate3D(*RotMatGantry) * G4Rotate3D(*RotMatCollimator));
    ApplyTransformation(copyNo,transform);

}

void ClinacTrueBeam::RotateCollimator(const G4int& copyNo, const G4double& delta) {
    fCollimatorAngle += delta;
    if (detFrame.find(copyNo) == detFrame.end()) {
        return;
    }

    G4ThreeVector initialCentre = det_origin + G4ThreeVector(fIsocenterDistance*std::sin(fGantryAngle), 0. * mm , fIsocenterDistance*std::cos(fGantryAngle));
    G4RotationMatrix* RotMatGantry = new G4RotationMatrix();
    RotMatGantry->rotateY(fGantryAngle);
    G4RotationMatrix* RotMatCollimator = new G4RotationMatrix();
    RotMatCollimator->rotateZ(fCollimatorAngle);
    G4Transform3D transform(G4Translate3D(initialCentre) * G4Rotate3D(*RotMatGantry) * G4Rotate3D(*RotMatCollimator));
    ApplyTransformation(copyNo,transform);

}
