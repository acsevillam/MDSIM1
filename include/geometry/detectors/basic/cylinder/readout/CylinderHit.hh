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

class CylinderHit : public BaseHit {
public:
    CylinderHit() : BaseHit("cylinder") {}
    CylinderHit(const CylinderHit&) = default;
    ~CylinderHit() override = default;

    CylinderHit& operator=(const CylinderHit&) = default;
    G4bool operator==(const CylinderHit& other) const;

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

using CylinderHitsCollection = G4THitsCollection<CylinderHit>;

extern G4ThreadLocal G4Allocator<CylinderHit>* CylinderHitAllocator;

inline void* CylinderHit::operator new(size_t) {
    if (!CylinderHitAllocator) {
        CylinderHitAllocator = new G4Allocator<CylinderHit>;
    }
    return CylinderHitAllocator->MallocSingle();
}

inline void CylinderHit::operator delete(void* hit) {
    if (!CylinderHitAllocator) {
        CylinderHitAllocator = new G4Allocator<CylinderHit>;
    }
    CylinderHitAllocator->FreeSingle(static_cast<CylinderHit*>(hit));
}

