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
#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "MD1DetectorConstruction.hh"
#include "MD1DetectorConstructionMessenger.hh"
#include "geometry/base/DetectorRegistry.hh"
#include "geometry/base/GenericGeometry.hh"
#include "geometry/beamline/ClinacTrueBeam.hh"
#include "geometry/phantoms/PhantomWaterBox.hh"
#include "geometry/phantoms/PhantomWaterTube.hh"
#include "MD1BOptrGeometryBasedBiasing.hh"

namespace MD1 {

namespace {

constexpr G4double kDefaultWaterPhantomExtent = 40.0 * cm;
constexpr G4double kDefaultWaterTubeRadius = kDefaultWaterPhantomExtent / 2.;
constexpr G4double kDefaultWaterTubeHeight = kDefaultWaterPhantomExtent;

} // namespace

MD1DetectorConstruction::MD1DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fBiasingVolume(nullptr),
      fClinacTrueBeam(std::make_unique<ClinacTrueBeam>()),
      fDefaultWaterPhantomType(WaterPhantomType::WaterBox),
      fPhantomWaterBox(std::make_unique<PhantomWaterBox>(kDefaultWaterPhantomExtent)),
      fPhantomWaterTube(
          std::make_unique<PhantomWaterTube>(kDefaultWaterTubeRadius, kDefaultWaterTubeHeight)),
      fMessenger(std::make_unique<MD1DetectorConstructionMessenger>(this)) {
    DetectorRegistry::GetInstance();
}

MD1DetectorConstruction::~MD1DetectorConstruction() = default;

void MD1DetectorConstruction::SetDefaultWaterPhantomType(const G4String& phantomType) {
    if (phantomType == "waterBox" || phantomType == "WaterBox" || phantomType == "box") {
        fDefaultWaterPhantomType = WaterPhantomType::WaterBox;
        return;
    }

    if (phantomType == "waterTube" || phantomType == "WaterTube" || phantomType == "tube") {
        fDefaultWaterPhantomType = WaterPhantomType::WaterTube;
        return;
    }

    G4Exception("MD1DetectorConstruction::SetDefaultWaterPhantomType",
                "InvalidWaterPhantomType",
                FatalException,
                ("Unknown water phantom type: " + phantomType +
                 ". Supported values are waterBox and waterTube.")
                    .c_str());
}

G4String MD1DetectorConstruction::GetDefaultWaterPhantomTypeName() const {
    return (fDefaultWaterPhantomType == WaterPhantomType::WaterBox) ? "waterBox" : "waterTube";
}

GenericGeometry* MD1DetectorConstruction::GetActiveWaterPhantom() const {
    if (fDefaultWaterPhantomType == WaterPhantomType::WaterTube) {
        return fPhantomWaterTube.get();
    }
    return fPhantomWaterBox.get();
}

G4String MD1DetectorConstruction::GetActiveWaterPhantomLogicalVolumeName() const {
    return (fDefaultWaterPhantomType == WaterPhantomType::WaterTube) ? "WaterTube" : "WaterBox";
}

G4double MD1DetectorConstruction::GetActiveWaterPhantomHeight() const {
    return (fDefaultWaterPhantomType == WaterPhantomType::WaterTube)
               ? fPhantomWaterTube->GetWaterTubeHeight()
               : fPhantomWaterBox->GetWaterBoxDz();
}

G4VPhysicalVolume* MD1DetectorConstruction::Construct() {
    auto* nistManager = G4NistManager::Instance();
    auto* materialAir = nistManager->FindOrBuildMaterial("G4_AIR");
    G4bool checkOverlaps = true;

    G4double worldSize = 2.5 * m;
    auto* geoWorld = new G4Box("world_geo", worldSize / 2, worldSize / 2, worldSize / 2);
    auto* logWorld = new G4LogicalVolume(geoWorld, materialAir, "world_log");

    auto* visWorld = new G4VisAttributes(G4Colour(0., 0.0, 0.1, 0.05));
    visWorld->SetVisibility(true);
    visWorld->SetForceSolid(true);
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

    auto* activeWaterPhantom = GetActiveWaterPhantom();
    const G4double waterPhantomHeight = GetActiveWaterPhantomHeight();
    activeWaterPhantom->AddGeometry(motherVolume,
                                    G4ThreeVector(0, 0, -(waterPhantomHeight / 2. - 10.0 * cm)),
                                    nullptr,
                                    0);
    auto* logWaterPhantom =
        activeWaterPhantom->GetLogVolume(GetActiveWaterPhantomLogicalVolumeName());

    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->ConstructGeometry(logWaterPhantom);
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
