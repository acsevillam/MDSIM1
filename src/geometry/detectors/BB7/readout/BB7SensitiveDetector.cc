/*
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
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/readout/BB7SensitiveDetector.hh"

BB7SensitiveDetector::BB7SensitiveDetector(const G4String& name)
    : BaseSensitiveDetector(name, "BB7"), fHitsCollection(nullptr), fHCID(-1) {
    collectionName.insert("BB7HitsCollection");
}

BB7SensitiveDetector::~BB7SensitiveDetector() = default;

void BB7SensitiveDetector::Initialize(G4HCofThisEvent* hce) {
    // Create a new hits collection
    fHitsCollection = new BB7HitsCollection(SensitiveDetectorName, collectionName[0]);

    // Add the collection to the event
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool BB7SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {
    // Energy deposit
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false;
    const G4double weight = step->GetPreStepPoint()->GetWeight();

    // Create a new hit
    auto newHit = std::make_unique<BB7Hit>();

    // Set hit information
    newHit->SetEdep(edep);
    newHit->SetWeight(weight);

    // Get detector ID, sensor ID, strip ID from the touchable history
    auto touchable = step->GetPreStepPoint()->GetTouchable();
    G4int detectorID = 0;
    G4int sensorID = 0;
    G4int stripID = 0;

    const G4int historyDepth = touchable->GetHistoryDepth();
    if (historyDepth >= 5) {
        detectorID = touchable->GetVolume(5)->GetCopyNo();
    } else if (touchable->GetVolume() != nullptr) {
        detectorID = touchable->GetVolume()->GetCopyNo();
    }

    if (historyDepth >= 4) {
        sensorID = touchable->GetVolume(4)->GetCopyNo();
    }

    if (historyDepth >= 2) {
        stripID = touchable->GetVolume(2)->GetCopyNo();
    }

    newHit->SetDetectorID(detectorID);
    newHit->SetSensorID(sensorID);
    newHit->SetStripID(stripID);

    // Print hit information
    // newHit->Print();

    // Add hit to the collection
    fHitsCollection->insert(newHit.release());

    return true;
}

void BB7SensitiveDetector::EndOfEvent(G4HCofThisEvent* /*hce*/) {}
