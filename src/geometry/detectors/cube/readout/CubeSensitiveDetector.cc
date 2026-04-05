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
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"

// MultiDetector Headers
#include "geometry/detectors/cube/readout/CubeSensitiveDetector.hh"

CubeSensitiveDetector::CubeSensitiveDetector(const G4String& name)
    : BaseSensitiveDetector(name, "cube"), fHitsCollection(nullptr), fHCID(-1) {
    collectionName.insert("CubeHitsCollection");
}

CubeSensitiveDetector::~CubeSensitiveDetector() = default;

void CubeSensitiveDetector::Initialize(G4HCofThisEvent* hce) {
    fHitsCollection = new CubeHitsCollection(SensitiveDetectorName, collectionName[0]);
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool CubeSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {
    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) {
        return false;
    }
    const G4double weight = step->GetPreStepPoint()->GetWeight();

    auto newHit = std::make_unique<CubeHit>();
    newHit->SetEdep(edep);
    newHit->SetWeight(weight);

    auto touchable = step->GetPreStepPoint()->GetTouchable();
    G4int detectorID = 0;
    if (touchable->GetVolume() != nullptr) {
        detectorID = touchable->GetVolume()->GetCopyNo();
    }
    newHit->SetDetectorID(detectorID);
    fHitsCollection->insert(newHit.release());

    return true;
}

void CubeSensitiveDetector::EndOfEvent(G4HCofThisEvent* /*hce*/) {}
