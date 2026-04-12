#include "analysis/RunSummaryUtils.hh"

#include <cmath>

namespace MD1::RunSummary {

G4double BuildRelativePercent(G4double absoluteError, G4double referenceValue) {
    if (referenceValue == 0.) {
        return 0.;
    }
    return 100. * absoluteError / std::abs(referenceValue);
}

StatDisplayValues BuildStatDisplayValues(const G4StatDouble& stat, G4int nofEvents) {
    StatDisplayValues values;
    if (nofEvents <= 0) {
        return values;
    }

    G4StatDouble statCopy = stat;
    values.meanPerEvent = statCopy.mean(nofEvents);
    values.rms = statCopy.rms(nofEvents, nofEvents);
    values.mcError = values.rms / std::sqrt(static_cast<G4double>(nofEvents));
    values.relativeMcErrorPercent = BuildRelativePercent(values.mcError, values.meanPerEvent);
    return values;
}

ScaledStatDisplayValues BuildScaledStatDisplayValues(const StatDisplayValues& values,
                                                     G4double scaleFactor,
                                                     G4double scaleFactorError,
                                                     G4double simulatedMU,
                                                     G4double detectorErrorPerEvent) {
    ScaledStatDisplayValues scaledValues;
    const G4double scaling = scaleFactor * simulatedMU;
    scaledValues.mean = values.meanPerEvent * scaling;
    scaledValues.rms = values.rms * scaling;
    scaledValues.mcError = values.mcError * scaling;
    scaledValues.muError = std::abs(values.meanPerEvent) * simulatedMU * scaleFactorError;
    scaledValues.detectorError = std::abs(detectorErrorPerEvent) * scaling;
    scaledValues.totalError =
        std::sqrt(scaledValues.mcError * scaledValues.mcError +
                  scaledValues.muError * scaledValues.muError +
                  scaledValues.detectorError * scaledValues.detectorError);
    scaledValues.relativeMcErrorPercent = BuildRelativePercent(scaledValues.mcError, scaledValues.mean);
    scaledValues.relativeMuErrorPercent = BuildRelativePercent(scaledValues.muError, scaledValues.mean);
    scaledValues.relativeDetectorErrorPercent =
        BuildRelativePercent(scaledValues.detectorError, scaledValues.mean);
    scaledValues.relativeTotalErrorPercent =
        BuildRelativePercent(scaledValues.totalError, scaledValues.mean);
    return scaledValues;
}

} // namespace MD1::RunSummary
