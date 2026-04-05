#ifndef MD1_BASE_SENSITIVE_DETECTOR_H
#define MD1_BASE_SENSITIVE_DETECTOR_H

#include "G4VSensitiveDetector.hh"
#include "globals.hh"

class BaseSensitiveDetector : public G4VSensitiveDetector {
public:
    BaseSensitiveDetector(const G4String& name, const G4String& detectorName)
        : G4VSensitiveDetector(name), fDetectorName(detectorName) {}
    ~BaseSensitiveDetector() override = default;

    const G4String& GetDetectorName() const { return fDetectorName; }

private:
    G4String fDetectorName;
};

#endif // MD1_BASE_SENSITIVE_DETECTOR_H
