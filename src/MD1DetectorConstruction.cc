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

#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "MD1DetectorConstruction.hh"
#include "geometry/base/DetectorRegistry.hh"
#include "geometry/beamline/ClinacTrueBeam.hh"
#include "geometry/phantoms/PhantomWaterBox.hh"
#include "MD1BOptrGeometryBasedBiasing.hh"

namespace MD1 {

MD1DetectorConstruction::MD1DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fBiasingVolume(nullptr),
      fClinacTrueBeam(std::make_unique<ClinacTrueBeam>()),
      fPhantomWaterBox(std::make_unique<PhantomWaterBox>(40.0 * cm)) {
    DetectorRegistry::GetInstance();
}

MD1DetectorConstruction::~MD1DetectorConstruction() = default;

G4VPhysicalVolume* MD1DetectorConstruction::Construct() {
    auto* nistManager = G4NistManager::Instance();
    auto* materialAir = nistManager->FindOrBuildMaterial("G4_AIR");
    G4bool checkOverlaps = true;

    G4double worldSize = 2.5 * m;
    auto* geoWorld = new G4Box("world_geo", worldSize / 2, worldSize / 2, worldSize / 2);
    auto* logWorld = new G4LogicalVolume(geoWorld, materialAir, "world_log");

    auto* visWorld = new G4VisAttributes(G4Colour(0., 0.5, 0.5, 0.1));
    visWorld->SetVisibility(true);
    visWorld->SetForceWireframe(true);
    logWorld->SetVisAttributes(visWorld);

    auto* physWorld = new G4PVPlacement(nullptr,
                                        G4ThreeVector(),
                                        logWorld,
                                        "world_phys",
                                        nullptr,
                                        false,
                                        0,
                                        checkOverlaps);

    SetupGeometry(logWorld);
    return physWorld;
}

void MD1DetectorConstruction::SetupGeometry(G4LogicalVolume* motherVolume) {
    fClinacTrueBeam->DefineMaterials();
    fClinacTrueBeam->AddGeometry(motherVolume, G4ThreeVector(0, 0, 0 * cm), nullptr, 0);
    fBiasingVolume = fClinacTrueBeam->GetLogVolume("biasTubs");

    const G4double waterBoxDx = 40.0 * cm;
    fPhantomWaterBox->AddGeometry(motherVolume,
                                  G4ThreeVector(0, 0, -(waterBoxDx / 2. - 10.0 * cm)),
                                  nullptr,
                                  0);
    auto* logWaterBox = fPhantomWaterBox->GetLogVolume("WaterBox");

    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->ConstructGeometry(logWaterBox);
    }
}

void MD1DetectorConstruction::ConstructSDandField() {
    auto* sdManager = G4SDManager::GetSDMpointer();
    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->RegisterSensitiveDetectors(sdManager);
    }

    if (fBiasingVolume != nullptr) {
        auto* biasingOperator = new MD1BOptrGeometryBasedBiasing();
        biasingOperator->AttachTo(fBiasingVolume);
    }
}

} // namespace MD1
