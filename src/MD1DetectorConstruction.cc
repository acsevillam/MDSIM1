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
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4PSEnergyDeposit3D.hh"
#include "G4PSNofStep3D.hh"
#include "G4PSCellFlux3D.hh"
#include "G4PSPassageCellFlux3D.hh"
#include "G4PSFlatSurfaceFlux3D.hh"
#include "G4PSFlatSurfaceCurrent3D.hh"
#include "G4PSPassageCellCurrent3D.hh"
#include "G4SDParticleWithEnergyFilter.hh"
#include "G4SDParticleFilter.hh"
#include "G4SDChargedFilter.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4GeometryManager.hh"
#include "G4DigiManager.hh"

// MultiDetector Headers
#include "MD1DetectorConstruction.hh"
#include "ClinacTrueBeam.hh"
#include "MLC80.hh"
#include "PhantomWaterBox.hh"
#include "DetectorSingleBB7.hh"
#include "DetectorDualBB7.hh"
#include "BB7SensitiveDetector.hh"
#include "MD1BOptrGeometryBasedBiasing.hh"

namespace MD1 {

G4VPhysicalVolume* MD1DetectorConstruction::Construct() {  
    // Get NIST material manager
    G4NistManager* nistManager = G4NistManager::Instance();

    // Define materials
    G4Material* materialAir = nistManager->FindOrBuildMaterial("G4_AIR");
    G4Material* materialWater = nistManager->FindOrBuildMaterial("G4_WATER");

    // Option to switch on/off checking of volumes overlaps
    G4bool checkOverlaps = true;

    // Geometrical volume
    G4double worldSize = 2.5 * m;
    G4Box* geoWorld = new G4Box("world_geo", worldSize/2, worldSize/2, worldSize/2);

    // Logical volume
    G4LogicalVolume* logWorld = new G4LogicalVolume(geoWorld, materialAir, "world_log");

    // General attributes
    G4VisAttributes* visWorld = new G4VisAttributes(G4Colour(0., 0.5, 0.5, 0.1));
    visWorld->SetVisibility(true);
    visWorld->SetForceWireframe(true);
    logWorld->SetVisAttributes(visWorld);

    // Physical volume
    G4VPhysicalVolume* physWorld = new G4PVPlacement(
        nullptr,                 // No rotation
        G4ThreeVector(),         // At (0,0,0)
        logWorld,                // Its logical volume
        "world_phys",            // Its name
        nullptr,                 // Its mother volume
        false,                   // No boolean operation
        0,                       // Copy number
        checkOverlaps            // Overlaps checking
    );

    SetupGeometry(logWorld);

    // Always return the physical World
    return physWorld;
}

void MD1DetectorConstruction::SetupGeometry(G4LogicalVolume* motherVolume) {

    // Create and add geometry for PhantomWaterBox
    ClinacTrueBeam* clinacTrueBeam = new ClinacTrueBeam();
    clinacTrueBeam->DefineMaterials();
    clinacTrueBeam->AddGeometry(motherVolume, G4ThreeVector(0, 0, 0*cm), nullptr, 0);
    fBiasingVolume = clinacTrueBeam->GetLogVolume("biasTubs");

    // Define the size of the water box
    G4double waterBoxDx = 40.0 * cm;
    // Create and add geometry for PhantomWaterBox
    PhantomWaterBox* phantomWaterBox = new PhantomWaterBox(waterBoxDx);
    phantomWaterBox->AddGeometry(motherVolume, G4ThreeVector(0, 0, -(waterBoxDx/2. - 10.0*cm)), nullptr, 0);
    G4LogicalVolume* logWaterBox = phantomWaterBox->GetLogVolume("WaterBox");
    phantomWaterBox->RemoveGeometry(0);

    // Instantiate the dual BB7 detector
    DetectorDualBB7* detectorDualBB7 = new DetectorDualBB7();
    // Rotation matrix for the detector
    G4RotationMatrix* rotMat1 = new G4RotationMatrix();
    rotMat1->rotateX(0 * deg);

    detectorDualBB7->AddGeometry(logWaterBox, G4ThreeVector(0, 0, -10)*cm, rotMat1, 0);
    detectorDualBB7->RemoveGeometry(0);

}

void MD1DetectorConstruction::ConstructSDandField()
{
    // Crear una instancia del detector sensible
    auto BB7SD = new BB7SensitiveDetector("BB7SD");

    // Registrar el detector sensible con el volumen lógico
    G4SDManager::GetSDMpointer()->AddNewDetector(BB7SD);
    SetSensitiveDetector("SdCube", BB7SD);

    MD1BOptrGeometryBasedBiasing* biasingOperator = new MD1BOptrGeometryBasedBiasing();
    biasingOperator->AttachTo(fBiasingVolume);   

}

} // namespace MD1
