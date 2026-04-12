#include "geometry/detectors/model11/readout/Model11Digit.hh"

#include <iomanip>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

G4ThreadLocal G4Allocator<Model11Digit>* Model11DigitAllocator = nullptr;

G4bool Model11Digit::operator==(const Model11Digit& right) const {
    return (this == &right) ? true : false;
}

void Model11Digit::Print() {
    G4cout << " DetectorID: " << std::setw(7) << fDetectorID
           << " PhotosensorType: " << GetPhotosensorTypeName()
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " VisibleEnergy: " << std::setw(7) << G4BestUnit(fVisibleEnergy, "Energy")
           << " ProducedPhotons: " << std::setw(7) << fProducedPhotons
           << " DetectedPhotoelectrons: " << std::setw(7) << fDetectedPhotoelectrons
           << " MeanSignalTime: " << std::setw(7) << fMeanSignalTime / ns << " ns"
           << G4endl;
}
