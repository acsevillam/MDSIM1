#include "analysis/RunSummaryUtils.hh"

#include <algorithm>
#include <cmath>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

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

G4double BuildCombinedCalibrationErrorPerEvent(
    const std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const std::vector<G4String>& activeSummaryLabels,
    G4int nofEvents) {
    G4double calibrationErrorSquared = 0.;
    for (const auto& detectorAccumulablesEntry : detectorAccumulables) {
        if (std::find(activeSummaryLabels.begin(),
                      activeSummaryLabels.end(),
                      detectorAccumulablesEntry.name) == activeSummaryLabels.end()) {
            continue;
        }

        const auto detectorCalibrationErrorValues = BuildStatDisplayValues(
            detectorAccumulablesEntry.estimatedDoseToWater.calibrationError,
            nofEvents);
        const G4double detectorCalibrationError = detectorCalibrationErrorValues.meanPerEvent;
        calibrationErrorSquared += detectorCalibrationError * detectorCalibrationError;
    }
    return std::sqrt(calibrationErrorSquared);
}

void PrintDetectorSummary(const DetectorRunAccumulables& detectorAccumulables,
                          G4int nofEvents,
                          G4int simulatedMU,
                          G4double scaleFactorMU,
                          G4double scaleFactorMUError) {
    const G4int detectedEvents = detectorAccumulables.events;
    const auto edepValues = BuildStatDisplayValues(detectorAccumulables.totalEdep, nofEvents);
    const auto chargeValues = BuildStatDisplayValues(detectorAccumulables.collectedCharge, nofEvents);
    const auto doseValues = BuildStatDisplayValues(detectorAccumulables.dose, nofEvents);
    const auto estimatedDoseToWaterValues =
        BuildStatDisplayValues(detectorAccumulables.estimatedDoseToWater.value, nofEvents);
    const auto estimatedDoseToWaterCalibrationErrorValues =
        BuildStatDisplayValues(detectorAccumulables.estimatedDoseToWater.calibrationError,
                               nofEvents);
    const auto chargeScaledValues =
        BuildScaledStatDisplayValues(chargeValues, scaleFactorMU, scaleFactorMUError, simulatedMU);
    const auto doseScaledValues =
        BuildScaledStatDisplayValues(doseValues, scaleFactorMU, scaleFactorMUError, simulatedMU);
    const auto estimatedDoseToWaterScaledValues =
        BuildScaledStatDisplayValues(estimatedDoseToWaterValues,
                                     scaleFactorMU,
                                     scaleFactorMUError,
                                     simulatedMU,
                                     estimatedDoseToWaterCalibrationErrorValues.meanPerEvent);

    G4cout << G4endl
           << "---------------- Detector Summary: " << detectorAccumulables.name << " ----------------" << G4endl
           << "(1)  Events with signal: " << detectedEvents << G4endl
           << "(2)  Fraction with signal: " << G4double(detectedEvents) / G4double(nofEvents) << G4endl
           << "(3)  Mean deposited energy per event: "
           << G4BestUnit(edepValues.meanPerEvent, "Energy")
           << " mc_err = " << G4BestUnit(edepValues.mcError, "Energy")
           << " (" << edepValues.relativeMcErrorPercent << " %)"
           << " rms = " << G4BestUnit(edepValues.rms, "Energy") << G4endl
           << "(4)  Mean dose per event in detector sensitive volume: "
           << G4BestUnit(doseValues.meanPerEvent, "Dose")
           << " mc_err = " << G4BestUnit(doseValues.mcError, "Dose")
           << " (" << doseValues.relativeMcErrorPercent << " %)"
           << " rms = " << G4BestUnit(doseValues.rms, "Dose") << G4endl
           << "(5)  Mean collected charge per event: "
           << chargeValues.meanPerEvent / (1e-9*coulomb) << " nC"
           << " mc_err = " << chargeValues.mcError / (1e-9*coulomb) << " nC"
           << " (" << chargeValues.relativeMcErrorPercent << " %)"
           << " rms = " << chargeValues.rms / (1e-9*coulomb) << " nC" << G4endl
           << "(6)  Calculated total dose in detector sensitive volume (" << simulatedMU << "UM): "
           << doseScaledValues.mean / (1e-2*gray) << " cGy"
           << " mc_err = " << doseScaledValues.mcError / (1e-2*gray) << " cGy"
           << " (" << doseScaledValues.relativeMcErrorPercent << " %)"
           << " mu_err = " << doseScaledValues.muError / (1e-2*gray) << " cGy"
           << " (" << doseScaledValues.relativeMuErrorPercent << " %)"
           << " total_err = " << doseScaledValues.totalError / (1e-2*gray) << " cGy"
           << " (" << doseScaledValues.relativeTotalErrorPercent << " %)"
           << " rms = " << doseScaledValues.rms / (1e-2*gray) << " cGy" << G4endl
           << "(7)  Scaled charge (" << simulatedMU << "UM): "
           << chargeScaledValues.mean / (1e-9*coulomb) << " nC"
           << " mc_err = " << chargeScaledValues.mcError / (1e-9*coulomb) << " nC"
           << " (" << chargeScaledValues.relativeMcErrorPercent << " %)"
           << " mu_err = " << chargeScaledValues.muError / (1e-9*coulomb) << " nC"
           << " (" << chargeScaledValues.relativeMuErrorPercent << " %)"
           << " total_err = " << chargeScaledValues.totalError / (1e-9*coulomb) << " nC"
           << " (" << chargeScaledValues.relativeTotalErrorPercent << " %)"
           << " rms = " << chargeScaledValues.rms / (1e-9*coulomb) << " nC" << G4endl
           << "(8)  Estimated total absorbed dose in water (" << simulatedMU << "UM): "
           << estimatedDoseToWaterScaledValues.mean / (1e-2*gray) << " cGy"
           << " mc_err = " << estimatedDoseToWaterScaledValues.mcError / (1e-2*gray) << " cGy"
           << " (" << estimatedDoseToWaterScaledValues.relativeMcErrorPercent << " %)"
           << " mu_err = " << estimatedDoseToWaterScaledValues.muError / (1e-2*gray) << " cGy"
           << " (" << estimatedDoseToWaterScaledValues.relativeMuErrorPercent << " %)"
           << " det_err = " << estimatedDoseToWaterScaledValues.detectorError / (1e-2*gray) << " cGy"
           << " (" << estimatedDoseToWaterScaledValues.relativeDetectorErrorPercent << " %)"
           << " total_err = " << estimatedDoseToWaterScaledValues.totalError / (1e-2*gray) << " cGy"
           << " (" << estimatedDoseToWaterScaledValues.relativeTotalErrorPercent << " %)"
           << " rms = " << estimatedDoseToWaterScaledValues.rms / (1e-2*gray) << " cGy" << G4endl
           << "------------------------------------------------------------------" << G4endl
           << "Notes: rms = G4StatDouble::rms(nofEvents, nofEvents); "
           << "mc_err = rms / sqrt(N) with N = total events per run. "
           << "mu_err comes from fScaleFactorMUError, det_err from detector calibration uncertainty, "
           << "and total_err is the quadrature sum of independent contributions."
           << G4endl;
}

} // namespace MD1::RunSummary
