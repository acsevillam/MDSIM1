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

class SphereDigit : public BaseDigit {
public:
    SphereDigit() : BaseDigit("sphere") {}
    SphereDigit(const SphereDigit&) = default;
    ~SphereDigit() override = default;

    SphereDigit& operator=(const SphereDigit&) = default;
    G4bool operator==(const SphereDigit& right) const;

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

using SphereDigitsCollection = G4TDigiCollection<SphereDigit>;

extern G4ThreadLocal G4Allocator<SphereDigit>* SphereDigitAllocator;

inline void* SphereDigit::operator new(size_t) {
    if (!SphereDigitAllocator) {
        SphereDigitAllocator = new G4Allocator<SphereDigit>;
    }
    return SphereDigitAllocator->MallocSingle();
}

inline void SphereDigit::operator delete(void* digit) {
    if (!SphereDigitAllocator) {
        SphereDigitAllocator = new G4Allocator<SphereDigit>;
    }
    SphereDigitAllocator->FreeSingle(static_cast<SphereDigit*>(digit));
}
