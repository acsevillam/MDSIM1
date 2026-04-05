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
#include "geometry/detectors/BB7/readout/BB7Hit.hh"

G4ThreadLocal G4Allocator<BB7Hit>* BB7HitAllocator = nullptr;

G4bool BB7Hit::operator==(const BB7Hit& right) const {
  return (this == &right) ? true : false;
}

void BB7Hit::Print(){
  G4cout << " detectorID: " << std::setw(7) << fDetectorID
    << " sensorID: " << std::setw(7) << fSensorID
    << " stripID: " << std::setw(7) << fStripID
    << " Edep: " << std::setw(7) << std::setw(7) << G4BestUnit(fEdep, "Energy") << G4endl;
    }