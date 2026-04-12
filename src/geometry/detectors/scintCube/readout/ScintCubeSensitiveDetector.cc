#include "geometry/detectors/scintCube/readout/ScintCubeSensitiveDetector.hh"

#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"

ScintCubeSensitiveDetector::ScintCubeSensitiveDetector(const G4String& name)
    : BaseSensitiveDetector(name, "scintCube"), fHitsCollection(nullptr), fHCID(-1) {
    collectionName.insert("ScintCubeHitsCollection");
}

ScintCubeSensitiveDetector::~ScintCubeSensitiveDetector() = default;

void ScintCubeSensitiveDetector::Initialize(G4HCofThisEvent* hce) {
    fHitsCollection = new ScintCubeHitsCollection(SensitiveDetectorName, collectionName[0]);
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection);
    }
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool ScintCubeSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {
    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) {
        return false;
    }

    auto newHit = std::make_unique<ScintCubeHit>();
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

void ScintCubeSensitiveDetector::EndOfEvent(G4HCofThisEvent* /*hce*/) {}
