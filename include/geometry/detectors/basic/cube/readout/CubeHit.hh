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

#ifndef CUBE_HIT_H
#define CUBE_HIT_H

// Geant4 Headers
#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "geometry/base/BaseHit.hh"

class CubeHit : public BaseHit {
public:
    CubeHit() : BaseHit("cube") {}
    CubeHit(const CubeHit&) = default;
    ~CubeHit() override = default;

    CubeHit& operator=(const CubeHit&) = default;
    G4bool operator==(const CubeHit& other) const;

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

using CubeHitsCollection = G4THitsCollection<CubeHit>;

extern G4ThreadLocal G4Allocator<CubeHit>* CubeHitAllocator;

inline void* CubeHit::operator new(size_t) {
    if (!CubeHitAllocator) {
        CubeHitAllocator = new G4Allocator<CubeHit>;
    }
    return CubeHitAllocator->MallocSingle();
}

inline void CubeHit::operator delete(void* hit) {
    if (!CubeHitAllocator) {
        CubeHitAllocator = new G4Allocator<CubeHit>;
    }
    CubeHitAllocator->FreeSingle(static_cast<CubeHit*>(hit));
}

#endif // CUBE_HIT_H
