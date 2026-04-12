#ifndef MD1_DOSIMETRIC_DETECTOR_RESULTS_HH
#define MD1_DOSIMETRIC_DETECTOR_RESULTS_HH

#include <vector>

#include "G4StatDouble.hh"
#include "G4VAccumulable.hh"
#include "globals.hh"

#include "geometry/base/DetectorPrintContext.hh"

namespace MD1 {

struct CalibratedDoseToWaterData {
    G4double value = 0.;
    G4double calibrationError = 0.;

    void Add(G4double nominalValue, G4double absoluteCalibrationError);
};

struct DosimetricDetectorResultsEntry {
    G4String summaryLabel;
    G4int events = 0;
    G4StatDouble totalEdep;
    G4StatDouble collectedCharge;
    G4StatDouble dose;
    G4StatDouble estimatedDoseToWater;
    G4StatDouble estimatedDoseToWaterCalibrationError;

    explicit DosimetricDetectorResultsEntry(const G4String& label = "");

    void AddEvent(G4double edep,
                  G4double collectedChargeValue,
                  G4double doseValue,
                  const CalibratedDoseToWaterData& estimatedDoseData);
    void Merge(const DosimetricDetectorResultsEntry& other);
};

class DosimetricDetectorResultsAccumulable : public G4VAccumulable {
public:
    explicit DosimetricDetectorResultsAccumulable(const G4String& name = G4String());

    DosimetricDetectorResultsEntry& FindOrCreate(const G4String& summaryLabel);
    const std::vector<DosimetricDetectorResultsEntry>& GetResults() const { return fResults; }

    void Merge(const G4VAccumulable& other) override;
    void Reset() override;

private:
    std::vector<DosimetricDetectorResultsEntry> fResults;
};

void PrintDosimetricDetectorResults(const G4String& detectorTitle,
                                    const std::vector<G4String>& summaryLabels,
                                    const DosimetricDetectorResultsAccumulable& results,
                                    const DetectorPrintContext& context);

} // namespace MD1

#endif // MD1_DOSIMETRIC_DETECTOR_RESULTS_HH
