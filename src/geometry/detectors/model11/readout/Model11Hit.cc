#include "geometry/detectors/model11/readout/Model11Hit.hh"

#include <iomanip>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

G4ThreadLocal G4Allocator<Model11Hit>* Model11HitAllocator = nullptr;

G4bool Model11Hit::operator==(const Model11Hit& right) const {
    return (this == &right) ? true : false;
}

void Model11Hit::Print() {
    G4cout << " detectorID: " << std::setw(7) << fDetectorID
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " StepLength: " << std::setw(7) << G4BestUnit(fStepLength, "Length")
           << " GlobalTime: " << std::setw(7) << fGlobalTime / ns << " ns"
           << G4endl;
}
