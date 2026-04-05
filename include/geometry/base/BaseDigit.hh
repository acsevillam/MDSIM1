#ifndef MD1_BASE_DIGIT_H
#define MD1_BASE_DIGIT_H

#include "G4VDigi.hh"
#include "globals.hh"

class BaseDigit : public G4VDigi {
public:
    explicit BaseDigit(const G4String& detectorName = "") : fDetectorName(detectorName) {}
    ~BaseDigit() override = default;

    const G4String& GetDetectorName() const { return fDetectorName; }
    void SetDetectorName(const G4String& detectorName) { fDetectorName = detectorName; }

private:
    G4String fDetectorName;
};

#endif // MD1_BASE_DIGIT_H
