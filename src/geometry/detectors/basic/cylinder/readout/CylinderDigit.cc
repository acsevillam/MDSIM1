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
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

// MultiDetector Headers
#include "geometry/detectors/basic/cylinder/readout/CylinderDigit.hh"

G4ThreadLocal G4Allocator<CylinderDigit>* CylinderDigitAllocator = nullptr;

G4bool CylinderDigit::operator==(const CylinderDigit& right) const {
    return (this == &right) ? true : false;
}

void CylinderDigit::Print() {
    G4cout << " DetectorID: " << std::setw(7) << fDetectorID
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " CollectedCharge: " << std::setw(7) << fCollectedCharge / (10e-9 * coulomb) << " nC"
           << " Dose: " << std::setw(7) << G4BestUnit(fDose, "Dose")
           << G4endl;
}
