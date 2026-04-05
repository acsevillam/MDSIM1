#ifndef MD1_BASE_HIT_H
#define MD1_BASE_HIT_H

#include "G4VHit.hh"
#include "globals.hh"

class BaseHit : public G4VHit {
public:
    explicit BaseHit(const G4String& detectorName = "") : fDetectorName(detectorName) {}
    ~BaseHit() override = default;

    const G4String& GetDetectorName() const { return fDetectorName; }
    void SetDetectorName(const G4String& detectorName) { fDetectorName = detectorName; }

private:
    G4String fDetectorName;
};

#endif // MD1_BASE_HIT_H
