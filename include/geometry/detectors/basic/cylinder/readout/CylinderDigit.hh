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

#pragma once


// Geant4 Headers
#include "G4Allocator.hh"
#include "G4TDigiCollection.hh"
#include "geometry/base/BaseDigit.hh"

class CylinderDigit : public BaseDigit {
public:
    CylinderDigit() : BaseDigit("cylinder") {}
    CylinderDigit(const CylinderDigit&) = default;
    ~CylinderDigit() override = default;

    CylinderDigit& operator=(const CylinderDigit&) = default;
    G4bool operator==(const CylinderDigit& right) const;

    inline void* operator new(size_t);
    inline void operator delete(void* digit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetCollectedCharge(G4double collectedCharge) { fCollectedCharge = collectedCharge; }
    void SetDose(G4double dose) { fDose = dose; }

    G4int GetDetectorID() const { return fDetectorID; }
    G4double GetEdep() const { return fEdep; }
    G4double GetCollectedCharge() const { return fCollectedCharge; }
    G4double GetDose() const { return fDose; }

private:
    G4int fDetectorID = -1;
    G4double fEdep = 0.;
    G4double fCollectedCharge = 0.;
    G4double fDose = 0.;
};

using CylinderDigitsCollection = G4TDigiCollection<CylinderDigit>;

extern G4ThreadLocal G4Allocator<CylinderDigit>* CylinderDigitAllocator;

inline void* CylinderDigit::operator new(size_t) {
    if (!CylinderDigitAllocator) {
        CylinderDigitAllocator = new G4Allocator<CylinderDigit>;
    }
    return CylinderDigitAllocator->MallocSingle();
}

inline void CylinderDigit::operator delete(void* digit) {
    if (!CylinderDigitAllocator) {
        CylinderDigitAllocator = new G4Allocator<CylinderDigit>;
    }
    CylinderDigitAllocator->FreeSingle(static_cast<CylinderDigit*>(digit));
}
