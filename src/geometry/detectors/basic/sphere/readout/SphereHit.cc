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
#include "G4UnitsTable.hh"

// MultiDetector Headers
#include "geometry/detectors/basic/sphere/readout/SphereHit.hh"

G4ThreadLocal G4Allocator<SphereHit>* SphereHitAllocator = nullptr;

G4bool SphereHit::operator==(const SphereHit& right) const {
    return (this == &right) ? true : false;
}

void SphereHit::Print() {
    G4cout << " detectorID: " << std::setw(7) << fDetectorID
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << G4endl;
}
