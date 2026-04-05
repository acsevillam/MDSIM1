#ifndef MD1_DETECTOR_REGISTRY_MESSENGER_H
#define MD1_DETECTOR_REGISTRY_MESSENGER_H

#include "G4UImessenger.hh"

class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class DetectorRegistry;

class DetectorRegistryMessenger : public G4UImessenger {
public:
    explicit DetectorRegistryMessenger(DetectorRegistry* registry);
    ~DetectorRegistryMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorRegistry* fRegistry;
    G4UIcmdWithAString* fEnableDetectorCmd;
    G4UIcmdWithAString* fDisableDetectorCmd;
    G4UIcmdWithoutParameter* fListDetectorsCmd;
};

#endif // MD1_DETECTOR_REGISTRY_MESSENGER_H
