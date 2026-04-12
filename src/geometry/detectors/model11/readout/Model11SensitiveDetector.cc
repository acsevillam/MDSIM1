#include "geometry/detectors/model11/readout/Model11SensitiveDetector.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"

Model11SensitiveDetector::Model11SensitiveDetector(const G4String& name)
    : BaseSensitiveDetector(name, "model11"), fHitsCollection(nullptr), fHCID(-1) {
    collectionName.insert("Model11HitsCollection");
}

Model11SensitiveDetector::~Model11SensitiveDetector() = default;

void Model11SensitiveDetector::Initialize(G4HCofThisEvent* hce) {
    fHitsCollection = new Model11HitsCollection(SensitiveDetectorName, collectionName[0]);
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool Model11SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {
    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) {
        return false;
    }

    auto newHit = std::make_unique<Model11Hit>();
    newHit->SetEdep(edep);
    newHit->SetStepLength(step->GetStepLength());
    newHit->SetGlobalTime(step->GetPreStepPoint()->GetGlobalTime());
    newHit->SetWeight(step->GetPreStepPoint()->GetWeight());

    auto* touchable = step->GetPreStepPoint()->GetTouchable();
    G4int detectorID = 0;
    if (touchable->GetVolume() != nullptr) {
        detectorID = touchable->GetVolume()->GetCopyNo();
    }
    newHit->SetDetectorID(detectorID);
    fHitsCollection->insert(newHit.release());

    return true;
}

void Model11SensitiveDetector::EndOfEvent(G4HCofThisEvent* /*hce*/) {}
