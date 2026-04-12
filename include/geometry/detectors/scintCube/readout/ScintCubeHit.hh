#ifndef MDSIM1_SCINT_CUBE_HIT_HH
#define MDSIM1_SCINT_CUBE_HIT_HH

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "geometry/base/BaseHit.hh"

class ScintCubeHit : public BaseHit {
public:
    ScintCubeHit() : BaseHit("scintCube") {}
    ScintCubeHit(const ScintCubeHit&) = default;
    ~ScintCubeHit() override = default;

    ScintCubeHit& operator=(const ScintCubeHit&) = default;
    G4bool operator==(const ScintCubeHit& other) const;

    inline void* operator new(size_t size);
    inline void operator delete(void* hit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetStepLength(G4double stepLength) { fStepLength = stepLength; }
    void SetGlobalTime(G4double globalTime) { fGlobalTime = globalTime; }
    void SetWeight(G4double weight) { fWeight = weight; }

    G4int GetDetectorID() const { return fDetectorID; }
    G4double GetEdep() const { return fEdep; }
    G4double GetStepLength() const { return fStepLength; }
    G4double GetGlobalTime() const { return fGlobalTime; }
    G4double GetWeight() const { return fWeight; }

private:
    G4int fDetectorID = -1;
    G4double fEdep = 0.;
    G4double fStepLength = 0.;
    G4double fGlobalTime = 0.;
    G4double fWeight = 1.;
};

using ScintCubeHitsCollection = G4THitsCollection<ScintCubeHit>;

extern G4ThreadLocal G4Allocator<ScintCubeHit>* ScintCubeHitAllocator;

inline void* ScintCubeHit::operator new(size_t) {
    if (!ScintCubeHitAllocator) {
        ScintCubeHitAllocator = new G4Allocator<ScintCubeHit>;
    }
    return ScintCubeHitAllocator->MallocSingle();
}

inline void ScintCubeHit::operator delete(void* hit) {
    if (!ScintCubeHitAllocator) {
        ScintCubeHitAllocator = new G4Allocator<ScintCubeHit>;
    }
    ScintCubeHitAllocator->FreeSingle(static_cast<ScintCubeHit*>(hit));
}

#endif // MDSIM1_SCINT_CUBE_HIT_HH
