#include "geometry/base/DosimetricDetectorResults.hh"

#include <algorithm>
#include <cmath>

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "analysis/RunSummaryUtils.hh"

namespace MD1 {

namespace {

const DosimetricDetectorResultsEntry* FindEntryByLabel(
    const std::vector<DosimetricDetectorResultsEntry>& entries,
    const G4String& summaryLabel) {
    const auto it = std::find_if(entries.begin(),
                                 entries.end(),
                                 [&](const auto& entry) { return entry.summaryLabel == summaryLabel; });
    return (it != entries.end()) ? &(*it) : nullptr;
}

} // namespace

void CalibratedDoseToWaterData::Add(G4double nominalValue, G4double absoluteCalibrationError) {
    value += nominalValue;
    calibrationError += std::abs(absoluteCalibrationError);
}

DosimetricDetectorResultsEntry::DosimetricDetectorResultsEntry(const G4String& label)
    : summaryLabel(label) {
    totalEdep.reset();
    collectedCharge.reset();
    dose.reset();
    estimatedDoseToWater.reset();
    estimatedDoseToWaterCalibrationError.reset();
}

void DosimetricDetectorResultsEntry::AddEvent(
    G4double edep,
    G4double collectedChargeValue,
    G4double doseValue,
    const CalibratedDoseToWaterData& estimatedDoseData) {
    events += 1;
    totalEdep.fill(edep);
    collectedCharge.fill(collectedChargeValue);
    dose.fill(doseValue);
    estimatedDoseToWater.fill(estimatedDoseData.value);
    estimatedDoseToWaterCalibrationError.fill(estimatedDoseData.calibrationError);
}

void DosimetricDetectorResultsEntry::Merge(const DosimetricDetectorResultsEntry& other) {
    events += other.events;
    totalEdep.add(&other.totalEdep);
    collectedCharge.add(&other.collectedCharge);
    dose.add(&other.dose);
    estimatedDoseToWater.add(&other.estimatedDoseToWater);
    estimatedDoseToWaterCalibrationError.add(&other.estimatedDoseToWaterCalibrationError);
}

DosimetricDetectorResultsAccumulable::DosimetricDetectorResultsAccumulable(const G4String& name)
    : G4VAccumulable(name) {}

DosimetricDetectorResultsEntry& DosimetricDetectorResultsAccumulable::FindOrCreate(
    const G4String& summaryLabel) {
    const auto it = std::find_if(fResults.begin(),
                                 fResults.end(),
                                 [&](const auto& entry) { return entry.summaryLabel == summaryLabel; });
    if (it != fResults.end()) {
        return *it;
    }

    fResults.emplace_back(summaryLabel);
    return fResults.back();
}

void DosimetricDetectorResultsAccumulable::Merge(const G4VAccumulable& other) {
    const auto& otherResults = static_cast<const DosimetricDetectorResultsAccumulable&>(other);
    for (const auto& otherEntry : otherResults.fResults) {
        auto& entry = FindOrCreate(otherEntry.summaryLabel);
        entry.Merge(otherEntry);
    }
}

void DosimetricDetectorResultsAccumulable::Reset() {
    fResults.clear();
}

void PrintDosimetricDetectorResults(const G4String& detectorTitle,
                                    const std::vector<G4String>& summaryLabels,
                                    const DosimetricDetectorResultsAccumulable& results,
                                    const DetectorPrintContext& context) {
    for (const auto& summaryLabel : summaryLabels) {
        const DosimetricDetectorResultsEntry emptyResults(summaryLabel);
        const auto* detectorResults = FindEntryByLabel(results.GetResults(), summaryLabel);
        if (detectorResults == nullptr) {
            detectorResults = &emptyResults;
        }

        const G4int detectedEvents = detectorResults->events;
        const auto edepValues =
            RunSummary::BuildStatDisplayValues(detectorResults->totalEdep, context.nofEvents);
        const auto chargeValues =
            RunSummary::BuildStatDisplayValues(detectorResults->collectedCharge, context.nofEvents);
        const auto doseValues =
            RunSummary::BuildStatDisplayValues(detectorResults->dose, context.nofEvents);
        const auto estimatedDoseValues =
            RunSummary::BuildStatDisplayValues(detectorResults->estimatedDoseToWater, context.nofEvents);
        const auto calibrationErrorValues = RunSummary::BuildStatDisplayValues(
            detectorResults->estimatedDoseToWaterCalibrationError, context.nofEvents);
        const auto chargeScaledValues =
            RunSummary::BuildScaledStatDisplayValues(chargeValues,
                                                     context.scaleFactorMU,
                                                     context.scaleFactorMUError,
                                                     context.simulatedMU);
        const auto doseScaledValues =
            RunSummary::BuildScaledStatDisplayValues(doseValues,
                                                     context.scaleFactorMU,
                                                     context.scaleFactorMUError,
                                                     context.simulatedMU);
        const auto estimatedDoseScaledValues =
            RunSummary::BuildScaledStatDisplayValues(estimatedDoseValues,
                                                     context.scaleFactorMU,
                                                     context.scaleFactorMUError,
                                                     context.simulatedMU,
                                                     calibrationErrorValues.meanPerEvent);

        G4cout << G4endl
               << "---------------- " << detectorTitle << " Results: " << summaryLabel
               << " ----------------" << G4endl
               << "(1)  Events with signal: " << detectedEvents << G4endl
               << "(2)  Fraction with signal: "
               << G4double(detectedEvents) / G4double(context.nofEvents) << G4endl
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
               << chargeValues.meanPerEvent / (1e-9 * coulomb) << " nC"
               << " mc_err = " << chargeValues.mcError / (1e-9 * coulomb) << " nC"
               << " (" << chargeValues.relativeMcErrorPercent << " %)"
               << " rms = " << chargeValues.rms / (1e-9 * coulomb) << " nC" << G4endl
               << "(6)  Scaled dose in detector sensitive volume (" << context.simulatedMU << "UM): "
               << doseScaledValues.mean / (1e-2 * gray) << " cGy"
               << " mc_err = " << doseScaledValues.mcError / (1e-2 * gray) << " cGy"
               << " (" << doseScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << doseScaledValues.muError / (1e-2 * gray) << " cGy"
               << " (" << doseScaledValues.relativeMuErrorPercent << " %)"
               << " total_err = " << doseScaledValues.totalError / (1e-2 * gray) << " cGy"
               << " (" << doseScaledValues.relativeTotalErrorPercent << " %)"
               << " rms = " << doseScaledValues.rms / (1e-2 * gray) << " cGy" << G4endl
               << "(7)  Scaled collected charge (" << context.simulatedMU << "UM): "
               << chargeScaledValues.mean / (1e-9 * coulomb) << " nC"
               << " mc_err = " << chargeScaledValues.mcError / (1e-9 * coulomb) << " nC"
               << " (" << chargeScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << chargeScaledValues.muError / (1e-9 * coulomb) << " nC"
               << " (" << chargeScaledValues.relativeMuErrorPercent << " %)"
               << " total_err = " << chargeScaledValues.totalError / (1e-9 * coulomb) << " nC"
               << " (" << chargeScaledValues.relativeTotalErrorPercent << " %)"
               << " rms = " << chargeScaledValues.rms / (1e-9 * coulomb) << " nC" << G4endl
               << "(8)  Estimated absorbed dose in water (" << context.simulatedMU << "UM): "
               << estimatedDoseScaledValues.mean / (1e-2 * gray) << " cGy"
               << " mc_err = " << estimatedDoseScaledValues.mcError / (1e-2 * gray) << " cGy"
               << " (" << estimatedDoseScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << estimatedDoseScaledValues.muError / (1e-2 * gray) << " cGy"
               << " (" << estimatedDoseScaledValues.relativeMuErrorPercent << " %)"
               << " det_err = " << estimatedDoseScaledValues.detectorError / (1e-2 * gray) << " cGy"
               << " (" << estimatedDoseScaledValues.relativeDetectorErrorPercent << " %)"
               << " total_err = " << estimatedDoseScaledValues.totalError / (1e-2 * gray) << " cGy"
               << " (" << estimatedDoseScaledValues.relativeTotalErrorPercent << " %)"
               << " rms = " << estimatedDoseScaledValues.rms / (1e-2 * gray) << " cGy" << G4endl
               << "------------------------------------------------------------------" << G4endl;
    }
}

} // namespace MD1
