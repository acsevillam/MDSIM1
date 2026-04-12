/*
 *
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
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/readout/BB7Digit.hh"

G4ThreadLocal G4Allocator<BB7Digit>* BB7DigitAllocator = nullptr;

/**
 * @brief Equality operator.
 * @param right The digit to compare with.
 * @return True if the digits are equal, false otherwise.
 */
G4bool BB7Digit::operator==(const BB7Digit& right) const {
    return (this == &right) ? true : false;
}

/**
 * @brief Print the digit information.
 */
void BB7Digit::Print() {
    G4cout << " DetectorID: " << std::setw(7) << fDetectorID
           << " Edep: " << std::setw(7) << G4BestUnit(fEdep, "Energy")
           << " CollectedCharge: " << std::setw(7) << fCollectedCharge / (10e-9 * coulomb) << " nC"
           << " Dose: " << std::setw(7) << G4BestUnit(fDose, "Dose")
           << G4endl;
}
