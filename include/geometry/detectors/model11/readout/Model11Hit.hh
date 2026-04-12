#ifndef MDSIM1_MODEL11_HIT_HH
#define MDSIM1_MODEL11_HIT_HH

#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "geometry/base/BaseHit.hh"

class Model11Hit : public BaseHit {
public:
    Model11Hit() : BaseHit("model11") {}
    Model11Hit(const Model11Hit&) = default;
    ~Model11Hit() override = default;

    Model11Hit& operator=(const Model11Hit&) = default;
    G4bool operator==(const Model11Hit& other) const;

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

using Model11HitsCollection = G4THitsCollection<Model11Hit>;

extern G4ThreadLocal G4Allocator<Model11Hit>* Model11HitAllocator;

inline void* Model11Hit::operator new(size_t) {
    if (!Model11HitAllocator) {
        Model11HitAllocator = new G4Allocator<Model11Hit>;
    }
    return Model11HitAllocator->MallocSingle();
}

inline void Model11Hit::operator delete(void* hit) {
    if (!Model11HitAllocator) {
        Model11HitAllocator = new G4Allocator<Model11Hit>;
    }
    Model11HitAllocator->FreeSingle(static_cast<Model11Hit*>(hit));
}

#endif // MDSIM1_MODEL11_HIT_HH
