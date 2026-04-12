#include "geometry/detectors/scintCube/readout/ScintCubeDigit.hh"

#include <iomanip>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

G4ThreadLocal G4Allocator<ScintCubeDigit>* ScintCubeDigitAllocator = nullptr;

G4bool ScintCubeDigit::operator==(const ScintCubeDigit& right) const {
    return (this == &right) ? true : false;
}

void ScintCubeDigit::Print() {
    G4cout << " DetectorID: " << std::setw(7) << fDetectorID
           << " PhotosensorType: " << GetPhotosensorTypeName()
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " VisibleEnergy: " << std::setw(7) << G4BestUnit(fVisibleEnergy, "Energy")
           << " ProducedPhotons: " << std::setw(7) << fProducedPhotons
           << " DetectedPhotoelectrons: " << std::setw(7) << fDetectedPhotoelectrons
           << " MeanSignalTime: " << std::setw(7) << fMeanSignalTime / ns << " ns"
           << G4endl;
}
