#ifndef MD1_BASE_DIGITIZER_H
#define MD1_BASE_DIGITIZER_H

#include "G4VDigitizerModule.hh"
#include "globals.hh"

class BaseDigitizer : public G4VDigitizerModule {
public:
    BaseDigitizer(const G4String& name, const G4String& detectorName)
        : G4VDigitizerModule(name), fDetectorName(detectorName) {}
    ~BaseDigitizer() override = default;

    const G4String& GetDetectorName() const { return fDetectorName; }

private:
    G4String fDetectorName;
};

#endif // MD1_BASE_DIGITIZER_H
