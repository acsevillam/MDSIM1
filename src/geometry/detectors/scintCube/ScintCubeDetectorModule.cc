#include "geometry/detectors/scintCube/ScintCubeDetectorModule.hh"

#include <algorithm>
#include <cmath>
#include <map>

#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4StatDouble.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4VAccumulable.hh"

#include "analysis/RunSummaryUtils.hh"
#include "geometry/base/DetectorModuleUtils.hh"
#include "geometry/detectors/scintCube/calibration/ScintCubeDoseCalibrator.hh"
#include "geometry/detectors/scintCube/geometry/DetectorScintCube.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeDigit.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeDigitizer.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeSensitiveDetector.hh"

namespace {

struct ScintCubeCalibratedDoseData {
    G4double value = 0.;
    G4double calibrationError = 0.;

    void Add(G4double nominalValue, G4double absoluteCalibrationError) {
        value += nominalValue;
        calibrationError += std::abs(absoluteCalibrationError);
    }
};

struct ScintCubeResultsEntry {
    G4String summaryLabel;
    ScintCubePhotosensorType photosensorType = ScintCubePhotosensorType::PMT;
    G4int signalEvents = 0;
    G4StatDouble totalEdep;
    G4StatDouble visibleEnergy;
    G4StatDouble producedPhotons;
    G4StatDouble detectedPhotoelectrons;
    G4StatDouble estimatedDoseToWater;
    G4StatDouble estimatedDoseToWaterCalibrationError;
    G4double weightedSignalTimeSum = 0.;
    G4double weightedSignalTimeWeights = 0.;

    explicit ScintCubeResultsEntry(const G4String& label = "")
        : summaryLabel(label) {
        totalEdep.reset();
        visibleEnergy.reset();
        producedPhotons.reset();
        detectedPhotoelectrons.reset();
        estimatedDoseToWater.reset();
        estimatedDoseToWaterCalibrationError.reset();
    }

    void AddEvent(ScintCubePhotosensorType type,
                  G4double edepValue,
                  G4double visibleEnergyValue,
                  G4double producedPhotonsValue,
                  G4double detectedPhotoelectronsValue,
                  G4double meanSignalTime,
                  const ScintCubeCalibratedDoseData& estimatedDoseData) {
        photosensorType = type;
        if (detectedPhotoelectronsValue > 0.) {
            signalEvents += 1;
            weightedSignalTimeSum += meanSignalTime * detectedPhotoelectronsValue;
            weightedSignalTimeWeights += detectedPhotoelectronsValue;
        }
        totalEdep.fill(edepValue);
        visibleEnergy.fill(visibleEnergyValue);
        producedPhotons.fill(producedPhotonsValue);
        detectedPhotoelectrons.fill(detectedPhotoelectronsValue);
        estimatedDoseToWater.fill(estimatedDoseData.value);
        estimatedDoseToWaterCalibrationError.fill(estimatedDoseData.calibrationError);
    }

    void Merge(const ScintCubeResultsEntry& other) {
        signalEvents += other.signalEvents;
        photosensorType = other.photosensorType;
        totalEdep.add(&other.totalEdep);
        visibleEnergy.add(&other.visibleEnergy);
        producedPhotons.add(&other.producedPhotons);
        detectedPhotoelectrons.add(&other.detectedPhotoelectrons);
        estimatedDoseToWater.add(&other.estimatedDoseToWater);
        estimatedDoseToWaterCalibrationError.add(&other.estimatedDoseToWaterCalibrationError);
        weightedSignalTimeSum += other.weightedSignalTimeSum;
        weightedSignalTimeWeights += other.weightedSignalTimeWeights;
    }

    G4double GetWeightedMeanSignalTime() const {
        if (weightedSignalTimeWeights <= 0.) {
            return 0.;
        }
        return weightedSignalTimeSum / weightedSignalTimeWeights;
    }
};

class ScintCubeResultsAccumulable : public G4VAccumulable {
public:
    explicit ScintCubeResultsAccumulable(const G4String& name = G4String())
        : G4VAccumulable(name) {}

    ScintCubeResultsEntry& FindOrCreate(const G4String& summaryLabel) {
        const auto it = std::find_if(
            fResults.begin(), fResults.end(), [&](const auto& entry) { return entry.summaryLabel == summaryLabel; });
        if (it != fResults.end()) {
            return *it;
        }

        fResults.emplace_back(summaryLabel);
        return fResults.back();
    }

    const std::vector<ScintCubeResultsEntry>& GetResults() const { return fResults; }

    void Merge(const G4VAccumulable& other) override {
        const auto& otherResults = static_cast<const ScintCubeResultsAccumulable&>(other);
        for (const auto& otherEntry : otherResults.fResults) {
            auto& entry = FindOrCreate(otherEntry.summaryLabel);
            entry.Merge(otherEntry);
        }
    }

    void Reset() override {
        fResults.clear();
    }

private:
    std::vector<ScintCubeResultsEntry> fResults;
};

struct ScintCubeDetectorRuntimeState final : DetectorRuntimeState {
    G4int signalNtupleId = -1;
    G4int calibrationNtupleId = -1;
    G4int digitsCollectionId = -1;
    ScintCubeResultsAccumulable resultsAccumulable{"ScintCubeResults"};
    G4bool resultsRegistered = false;
};

const ScintCubeResultsEntry* FindEntryByLabel(const ScintCubeResultsAccumulable& results,
                                              const G4String& summaryLabel) {
    const auto& entries = results.GetResults();
    const auto it = std::find_if(
        entries.begin(), entries.end(), [&](const auto& entry) { return entry.summaryLabel == summaryLabel; });
    return (it != entries.end()) ? &(*it) : nullptr;
}

void ValidateAnalysisIdsOrThrow(G4int signalNtupleId,
                                G4int calibrationNtupleId,
                                const G4String& detectorName) {
    if (signalNtupleId < 0 || calibrationNtupleId < 0) {
        G4Exception("ScintCubeDetectorModule::ValidateAnalysisIdsOrThrow",
                    "DetectorAnalysisNotInitialized",
                    FatalException,
                    ("Detector module " + detectorName +
                     " has not created its analysis ntuples before event processing.").c_str());
    }
}

ScintCubeDetectorRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                                      const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<ScintCubeDetectorRuntimeState>(
        runtimeState, detectorName, "ScintCubeDetectorModule::GetRuntimeStateOrThrow");
}

const ScintCubeDetectorRuntimeState& GetRuntimeStateOrThrow(const DetectorRuntimeState& runtimeState,
                                                            const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<ScintCubeDetectorRuntimeState>(
        runtimeState, detectorName, "ScintCubeDetectorModule::GetRuntimeStateOrThrow");
}

} // namespace

ScintCubeDetectorModule::ScintCubeDetectorModule()
    : fEnabled(false), fGeometry(std::make_unique<DetectorScintCube>()) {}

ScintCubeDetectorModule::~ScintCubeDetectorModule() = default;

G4bool ScintCubeDetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void ScintCubeDetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr || !HasPlacedGeometry()) {
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void ScintCubeDetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* scintCubeSD = DetectorModuleUtils::GetOrCreateSensitiveDetector<ScintCubeSensitiveDetector>(
        sdManager, "ScintCubeSD");
    fGeometry->AttachSensitiveDetector(scintCubeSD);
}

void ScintCubeDetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    if (digiManager->FindDigitizerModule("ScintCubeDigitizer") != nullptr) {
        return;
    }

    digiManager->AddNewModule(
        new ScintCubeDigitizer("ScintCubeDigitizer", fGeometry->GetReadoutParametersByDetector()));
}

std::unique_ptr<DetectorRuntimeState> ScintCubeDetectorModule::CreateRuntimeState() const {
    return std::make_unique<ScintCubeDetectorRuntimeState>();
}

void ScintCubeDetectorModule::PrepareForRun(DetectorRuntimeState& runtimeState, G4bool /*isMaster*/) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (!state.resultsRegistered) {
        G4AccumulableManager::Instance()->Register(&state.resultsAccumulable);
        state.resultsRegistered = true;
    }
}

void ScintCubeDetectorModule::MergeRunResults(DetectorRuntimeState& /*runtimeState*/, G4bool /*isMaster*/) {}

void ScintCubeDetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                             DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.signalNtupleId >= 0 || state.calibrationNtupleId >= 0) {
        return;
    }

    state.signalNtupleId =
        analysisManager->CreateNtuple("ScintCubeSignal", "Physical scintCube digitization output");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "PhotosensorType");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "VisibleEnergy[eV]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "ProducedPhotons");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "DetectedPhotoelectrons");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "MeanSignalTime[ns]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "EventID");
    analysisManager->FinishNtuple(state.signalNtupleId);

    state.calibrationNtupleId =
        analysisManager->CreateNtuple("ScintCubeDoseCalibration", "ScintCube calibrated dose output");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "PhotosensorType");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "DetectedPhotoelectrons");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "CalibrationError[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EventID");
    analysisManager->FinishNtuple(state.calibrationNtupleId);
}

std::vector<G4String> ScintCubeDetectorModule::GetSummaryLabels() const {
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    std::vector<G4String> labels;
    labels.reserve(copyNumbers.size());
    for (const auto copyNumber : copyNumbers) {
        labels.push_back(GetSummaryLabel(copyNumber));
    }
    return labels;
}

G4String ScintCubeDetectorModule::GetSummaryLabel(G4int detectorID) const {
    return GetName() + "[" + std::to_string(detectorID) + "]";
}

void ScintCubeDetectorModule::ProcessEvent(const G4Event* event,
                                           G4AnalysisManager* analysisManager,
                                           G4DigiManager* digiManager,
                                           DetectorRuntimeState& runtimeState) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateAnalysisIdsOrThrow(state.signalNtupleId, state.calibrationNtupleId, GetName());

    auto* digitizer = DetectorModuleUtils::GetDigitizerOrThrow<ScintCubeDigitizer>(
        digiManager, "ScintCubeDigitizer", GetName(), "ScintCubeDetectorModule::GetDigitizerOrThrow");
    digitizer->Digitize();

    if (state.digitsCollectionId < 0) {
        state.digitsCollectionId = DetectorModuleUtils::GetDigiCollectionIdOrThrow(
            digiManager,
            "ScintCubeDigitsCollection",
            GetName(),
            "ScintCubeDetectorModule::GetDigiCollectionIdOrThrow");
    }

    auto* digitsCollection = static_cast<const ScintCubeDigitsCollection*>(
        digiManager->GetDigiCollection(state.digitsCollectionId));
    if (digitsCollection == nullptr) {
        return;
    }

    struct EventTotals {
        ScintCubePhotosensorType photosensorType = ScintCubePhotosensorType::PMT;
        G4double totalEdep = 0.;
        G4double totalVisibleEnergy = 0.;
        G4double totalProducedPhotons = 0.;
        G4double totalDetectedPhotoelectrons = 0.;
        G4double weightedMeanSignalTime = 0.;
        ScintCubeCalibratedDoseData estimatedDoseToWater;
    };
    std::map<G4int, EventTotals> eventTotalsByDetector;

    for (size_t i = 0; i < digitsCollection->entries(); ++i) {
        auto* digit = (*digitsCollection)[i];
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(
            state.signalNtupleId, 1, static_cast<G4double>(static_cast<int>(digit->GetPhotosensorType())));
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 2, digit->GetEdep());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 3, digit->GetVisibleEnergy());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 4, digit->GetProducedPhotons());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 5, digit->GetDetectedPhotoelectrons());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 6, digit->GetMeanSignalTime() / ns);
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 7, event->GetEventID());
        analysisManager->AddNtupleRow(state.signalNtupleId);

        const ScintCubeDoseCalibrator calibrator(
            fGeometry->GetCalibrationParameters(digit->GetDetectorID()));
        const auto calibratedDose = calibrator.Calibrate(*digit);

        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 1, static_cast<G4double>(static_cast<int>(digit->GetPhotosensorType())));
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 2, digit->GetDetectedPhotoelectrons());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 3, calibratedDose.estimatedDoseToWater);
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 4, calibratedDose.calibrationError);
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 5, event->GetEventID());
        analysisManager->AddNtupleRow(state.calibrationNtupleId);

        auto& eventTotals = eventTotalsByDetector[digit->GetDetectorID()];
        eventTotals.photosensorType = digit->GetPhotosensorType();
        eventTotals.totalEdep += digit->GetEdep();
        eventTotals.totalVisibleEnergy += digit->GetVisibleEnergy();
        eventTotals.totalProducedPhotons += digit->GetProducedPhotons();
        eventTotals.totalDetectedPhotoelectrons += digit->GetDetectedPhotoelectrons();
        eventTotals.weightedMeanSignalTime +=
            digit->GetMeanSignalTime() * digit->GetDetectedPhotoelectrons();
        eventTotals.estimatedDoseToWater.Add(
            calibratedDose.estimatedDoseToWater, calibratedDose.calibrationError);
    }

    for (const auto& [detectorID, eventTotals] : eventTotalsByDetector) {
        auto& summary = state.resultsAccumulable.FindOrCreate(GetSummaryLabel(detectorID));
        const G4double meanSignalTime =
            (eventTotals.totalDetectedPhotoelectrons > 0.)
                ? eventTotals.weightedMeanSignalTime / eventTotals.totalDetectedPhotoelectrons
                : 0.;
        summary.AddEvent(eventTotals.photosensorType,
                         eventTotals.totalEdep,
                         eventTotals.totalVisibleEnergy,
                         eventTotals.totalProducedPhotons,
                         eventTotals.totalDetectedPhotoelectrons,
                         meanSignalTime,
                         eventTotals.estimatedDoseToWater);
    }
}

void ScintCubeDetectorModule::PrintResults(const G4Run* /*run*/,
                                           const DetectorRuntimeState& runtimeState,
                                           const MD1::DetectorPrintContext& context) const {
    const auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    for (const auto detectorID : copyNumbers) {
        const auto summaryLabel = GetSummaryLabel(detectorID);
        const auto* detectorResults = FindEntryByLabel(state.resultsAccumulable, summaryLabel);
        const ScintCubeResultsEntry emptyResults(summaryLabel);
        if (detectorResults == nullptr) {
            detectorResults = &emptyResults;
        }

        const auto& config = fGeometry->GetDetectorConfig(detectorID);
        const auto edepValues =
            MD1::RunSummary::BuildStatDisplayValues(detectorResults->totalEdep, context.nofEvents);
        const auto visibleEnergyValues =
            MD1::RunSummary::BuildStatDisplayValues(detectorResults->visibleEnergy, context.nofEvents);
        const auto producedPhotonValues =
            MD1::RunSummary::BuildStatDisplayValues(detectorResults->producedPhotons, context.nofEvents);
        const auto detectedPhotoelectronValues =
            MD1::RunSummary::BuildStatDisplayValues(detectorResults->detectedPhotoelectrons, context.nofEvents);
        const auto estimatedDoseValues = MD1::RunSummary::BuildStatDisplayValues(
            detectorResults->estimatedDoseToWater, context.nofEvents);
        const auto calibrationErrorValues = MD1::RunSummary::BuildStatDisplayValues(
            detectorResults->estimatedDoseToWaterCalibrationError, context.nofEvents);
        const auto scaledPhotoelectrons = MD1::RunSummary::BuildScaledStatDisplayValues(
            detectedPhotoelectronValues,
            context.scaleFactorMU,
            context.scaleFactorMUError,
            context.simulatedMU);
        const auto estimatedDoseScaledValues = MD1::RunSummary::BuildScaledStatDisplayValues(
            estimatedDoseValues,
            context.scaleFactorMU,
            context.scaleFactorMUError,
            context.simulatedMU,
            calibrationErrorValues.meanPerEvent);

        G4cout << G4endl
               << "---------------- ScintCube Results: " << summaryLabel << " ----------------" << G4endl
               << "(1)  Events with detected signal: " << detectorResults->signalEvents << G4endl
               << "(2)  Fraction with detected signal: "
               << G4double(detectorResults->signalEvents) / G4double(context.nofEvents) << G4endl
               << "(3)  PhotosensorType: "
               << ScintCubePhotosensorTypeToString(config.readoutParameters.photosensorType) << G4endl
               << "(4)  Mean deposited energy per event: "
               << G4BestUnit(edepValues.meanPerEvent, "Energy")
               << " mc_err = " << G4BestUnit(edepValues.mcError, "Energy")
               << " (" << edepValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(edepValues.rms, "Energy") << G4endl
               << "(5)  Mean visible energy per event: "
               << G4BestUnit(visibleEnergyValues.meanPerEvent, "Energy")
               << " mc_err = " << G4BestUnit(visibleEnergyValues.mcError, "Energy")
               << " (" << visibleEnergyValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(visibleEnergyValues.rms, "Energy") << G4endl
               << "(6)  Mean produced scintillation photons per event: "
               << producedPhotonValues.meanPerEvent
               << " mc_err = " << producedPhotonValues.mcError
               << " (" << producedPhotonValues.relativeMcErrorPercent << " %)"
               << " rms = " << producedPhotonValues.rms << G4endl
               << "(7)  Mean detected photoelectrons per event: "
               << detectedPhotoelectronValues.meanPerEvent
               << " mc_err = " << detectedPhotoelectronValues.mcError
               << " (" << detectedPhotoelectronValues.relativeMcErrorPercent << " %)"
               << " rms = " << detectedPhotoelectronValues.rms << G4endl
               << "(8)  Mean signal time weighted by photoelectrons: "
               << detectorResults->GetWeightedMeanSignalTime() / ns << " ns" << G4endl
               << "(9)  Scaled detected photoelectrons (" << context.simulatedMU << "UM): "
               << scaledPhotoelectrons.mean
               << " mc_err = " << scaledPhotoelectrons.mcError
               << " (" << scaledPhotoelectrons.relativeMcErrorPercent << " %)"
               << " mu_err = " << scaledPhotoelectrons.muError
               << " (" << scaledPhotoelectrons.relativeMuErrorPercent << " %)"
               << " total_err = " << scaledPhotoelectrons.totalError
               << " (" << scaledPhotoelectrons.relativeTotalErrorPercent << " %)"
               << " rms = " << scaledPhotoelectrons.rms << G4endl
               << "(10) Estimated absorbed dose in water (" << context.simulatedMU << "UM): "
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
