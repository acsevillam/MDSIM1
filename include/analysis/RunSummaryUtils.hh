#ifndef MD1_RUN_SUMMARY_UTILS_HH
#define MD1_RUN_SUMMARY_UTILS_HH

#include <vector>

#include "G4StatDouble.hh"
#include "globals.hh"

#include "analysis/DetectorAccumulablesStore.hh"

namespace MD1::RunSummary {

struct StatDisplayValues {
    G4double meanPerEvent = 0.;
    G4double rms = 0.;
    G4double mcError = 0.;
    G4double relativeMcErrorPercent = 0.;
};

struct ScaledStatDisplayValues {
    G4double mean = 0.;
    G4double rms = 0.;
    G4double mcError = 0.;
    G4double muError = 0.;
    G4double detectorError = 0.;
    G4double totalError = 0.;
    G4double relativeMcErrorPercent = 0.;
    G4double relativeMuErrorPercent = 0.;
    G4double relativeDetectorErrorPercent = 0.;
    G4double relativeTotalErrorPercent = 0.;
};

G4double BuildRelativePercent(G4double absoluteError, G4double referenceValue);

StatDisplayValues BuildStatDisplayValues(const G4StatDouble& stat, G4int nofEvents);

ScaledStatDisplayValues BuildScaledStatDisplayValues(const StatDisplayValues& values,
                                                     G4double scaleFactor,
                                                     G4double scaleFactorError,
                                                     G4double simulatedMU,
                                                     G4double detectorErrorPerEvent = 0.);

G4double BuildCombinedCalibrationErrorPerEvent(
    const std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const std::vector<G4String>& activeSummaryLabels,
    G4int nofEvents);

void PrintDetectorSummary(const DetectorRunAccumulables& detectorAccumulables,
                          G4int nofEvents,
                          G4int simulatedMU,
                          G4double scaleFactorMU,
                          G4double scaleFactorMUError);

} // namespace MD1::RunSummary

#endif // MD1_RUN_SUMMARY_UTILS_HH
