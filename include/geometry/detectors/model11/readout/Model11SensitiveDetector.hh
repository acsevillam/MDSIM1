#ifndef MDSIM1_MODEL11_SENSITIVE_DETECTOR_HH
#define MDSIM1_MODEL11_SENSITIVE_DETECTOR_HH

#include "geometry/base/BaseSensitiveDetector.hh"
#include "geometry/detectors/model11/readout/Model11Hit.hh"

class G4HCofThisEvent;
class G4Step;
class G4TouchableHistory;

class Model11SensitiveDetector : public BaseSensitiveDetector {
public:
    explicit Model11SensitiveDetector(const G4String& name);
    ~Model11SensitiveDetector() override;

    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    Model11HitsCollection* fHitsCollection;
    G4int fHCID;
};

#endif // MDSIM1_MODEL11_SENSITIVE_DETECTOR_HH
