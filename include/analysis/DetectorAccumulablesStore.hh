#ifndef MD1_DETECTOR_ACCUMULABLES_STORE_HH
#define MD1_DETECTOR_ACCUMULABLES_STORE_HH

#include <vector>

#include "G4StatDouble.hh"
#include "globals.hh"

#include "geometry/base/DetectorEventData.hh"

namespace MD1 {

struct CalibratedDoseToWaterRunAccumulables {
    G4StatDouble value;
    G4StatDouble calibrationError;

    explicit CalibratedDoseToWaterRunAccumulables(const G4String& detectorName);

    void Fill(const CalibratedDoseToWaterData& data);
    void Merge(const CalibratedDoseToWaterRunAccumulables& other);
};

struct DetectorRunAccumulables {
    G4String name;
    G4int events = 0;
    G4StatDouble totalEdep;
    G4StatDouble collectedCharge;
    G4StatDouble dose;
    CalibratedDoseToWaterRunAccumulables estimatedDoseToWater;

    explicit DetectorRunAccumulables(const G4String& detectorName);

    void Merge(const DetectorRunAccumulables& other);
};

class DetectorAccumulablesStore {
  public:
    void PrepareForRun(G4bool isMaster);
    void MergeWorkerResults(G4bool isMaster);
    std::vector<DetectorRunAccumulables> GetAccumulablesForSummary(G4bool isMaster) const;

    DetectorRunAccumulables& FindOrCreate(const G4String& detectorName);

  private:
    std::vector<DetectorRunAccumulables> fLocalAccumulables;
    std::size_t fGeneration = 0;
};

} // namespace MD1

#endif // MD1_DETECTOR_ACCUMULABLES_STORE_HH
