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
#include "G4THitsCollection.hh"
#include "geometry/base/BaseHit.hh"

class SphereHit : public BaseHit {
public:
    SphereHit() : BaseHit("sphere") {}
    SphereHit(const SphereHit&) = default;
    ~SphereHit() override = default;

    SphereHit& operator=(const SphereHit&) = default;
    G4bool operator==(const SphereHit& other) const;

    inline void* operator new(size_t size);
    inline void operator delete(void* hit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetWeight(G4double weight) { fWeight = weight; }

    G4int GetDetectorID() const { return fDetectorID; }
    G4double GetEdep() const { return fEdep; }
    G4double GetWeight() const { return fWeight; }

private:
    G4int fDetectorID;
    G4double fEdep;
    G4double fWeight = 1.;
};

using SphereHitsCollection = G4THitsCollection<SphereHit>;

extern G4ThreadLocal G4Allocator<SphereHit>* SphereHitAllocator;

inline void* SphereHit::operator new(size_t) {
    if (!SphereHitAllocator) {
        SphereHitAllocator = new G4Allocator<SphereHit>;
    }
    return SphereHitAllocator->MallocSingle();
}

inline void SphereHit::operator delete(void* hit) {
    if (!SphereHitAllocator) {
        SphereHitAllocator = new G4Allocator<SphereHit>;
    }
    SphereHitAllocator->FreeSingle(static_cast<SphereHit*>(hit));
}

