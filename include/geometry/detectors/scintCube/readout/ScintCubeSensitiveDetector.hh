#ifndef MDSIM1_SCINT_CUBE_SENSITIVE_DETECTOR_HH
#define MDSIM1_SCINT_CUBE_SENSITIVE_DETECTOR_HH

#include "geometry/base/BaseSensitiveDetector.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeHit.hh"

class G4HCofThisEvent;
class G4Step;
class G4TouchableHistory;

class ScintCubeSensitiveDetector : public BaseSensitiveDetector {
public:
    explicit ScintCubeSensitiveDetector(const G4String& name);
    ~ScintCubeSensitiveDetector() override;

    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    ScintCubeHitsCollection* fHitsCollection;
    G4int fHCID;
};

#endif // MDSIM1_SCINT_CUBE_SENSITIVE_DETECTOR_HH
