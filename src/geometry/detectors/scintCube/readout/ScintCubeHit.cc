#include "geometry/detectors/scintCube/readout/ScintCubeHit.hh"

#include <iomanip>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

G4ThreadLocal G4Allocator<ScintCubeHit>* ScintCubeHitAllocator = nullptr;

G4bool ScintCubeHit::operator==(const ScintCubeHit& right) const {
    return (this == &right) ? true : false;
}

void ScintCubeHit::Print() {
    G4cout << " detectorID: " << std::setw(7) << fDetectorID
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " StepLength: " << std::setw(7) << G4BestUnit(fStepLength, "Length")
           << " GlobalTime: " << std::setw(7) << fGlobalTime / ns << " ns"
           << G4endl;
}
