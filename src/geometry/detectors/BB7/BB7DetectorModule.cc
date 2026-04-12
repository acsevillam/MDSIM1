#include "geometry/detectors/BB7/BB7DetectorModule.hh"

#include <algorithm>
#include <map>

#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/base/DosimetricDetectorResults.hh"
#include "geometry/base/DetectorModuleUtils.hh"
#include "geometry/detectors/BB7/calibration/BB7DoseCalibrator.hh"
#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"
#include "geometry/detectors/BB7/readout/BB7Digit.hh"
#include "geometry/detectors/BB7/readout/BB7Digitizer.hh"
#include "geometry/detectors/BB7/readout/BB7ReadoutModel.hh"
#include "geometry/detectors/BB7/readout/BB7SensitiveDetector.hh"

namespace {

struct BB7DetectorRuntimeState final : DetectorRuntimeState {
    G4int signalNtupleId = -1;
    G4int calibrationNtupleId = -1;
    G4int chargeMapId = -1;
    G4int digitsCollectionId = -1;
    MD1::DosimetricDetectorResultsAccumulable resultsAccumulable{"BB7Results"};
    G4bool resultsRegistered = false;
};

void ValidateAnalysisIdsOrThrow(G4int signalNtupleId,
                                G4int calibrationNtupleId,
                                G4int chargeMapId,
                                const G4String& detectorName) {
    if (signalNtupleId < 0 || calibrationNtupleId < 0 || chargeMapId < 0) {
        G4Exception("BB7DetectorModule::ValidateAnalysisIdsOrThrow",
                    "DetectorAnalysisNotInitialized",
                    FatalException,
                    ("Detector module " + detectorName +
                     " has not created its analysis objects before event processing.").c_str());
    }
}

BB7DetectorRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                                const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<BB7DetectorRuntimeState>(
        runtimeState, detectorName, "BB7DetectorModule::GetRuntimeStateOrThrow");
}

const BB7DetectorRuntimeState& GetRuntimeStateOrThrow(const DetectorRuntimeState& runtimeState,
                                                      const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<BB7DetectorRuntimeState>(
        runtimeState, detectorName, "BB7DetectorModule::GetRuntimeStateOrThrow");
}

} // namespace

BB7DetectorModule::BB7DetectorModule()
    : fEnabled(false), fGeometry(std::make_unique<DetectorDualBB7>()) {}

BB7DetectorModule::~BB7DetectorModule() = default;

G4bool BB7DetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void BB7DetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void BB7DetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* bb7SD =
        DetectorModuleUtils::GetOrCreateSensitiveDetector<BB7SensitiveDetector>(sdManager, "BB7SD");

    auto* logicalVolume = DetectorModuleUtils::GetLogicalVolumeOrThrow(
        "SdCube", GetName(), "BB7DetectorModule::GetLogicalVolumeOrThrow");
    logicalVolume->SetSensitiveDetector(bb7SD);
}

void BB7DetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    if (digiManager->FindDigitizerModule("BB7Digitizer") != nullptr) {
        return;
    }
    digiManager->AddNewModule(new BB7Digitizer("BB7Digitizer", BB7ReadoutModel::Build()));
}

std::unique_ptr<DetectorRuntimeState> BB7DetectorModule::CreateRuntimeState() const {
    return std::make_unique<BB7DetectorRuntimeState>();
}

void BB7DetectorModule::PrepareForRun(DetectorRuntimeState& runtimeState, G4bool /*isMaster*/) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (!state.resultsRegistered) {
        G4AccumulableManager::Instance()->Register(&state.resultsAccumulable);
        state.resultsRegistered = true;
    }
}

void BB7DetectorModule::MergeRunResults(DetectorRuntimeState& /*runtimeState*/, G4bool /*isMaster*/) {}

void BB7DetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                       DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.signalNtupleId >= 0 || state.calibrationNtupleId >= 0) {
        return;
    }

    state.signalNtupleId =
        analysisManager->CreateNtuple("BB7Hits", "Physical readout variables related to BB7 detector hits");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "SensorID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "StripID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "EventID");
    analysisManager->FinishNtuple(state.signalNtupleId);

    state.calibrationNtupleId =
        analysisManager->CreateNtuple("BB7DoseCalibration", "BB7 calibrated dose output");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "SensorID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "StripID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "CalibrationError[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EventID");
    analysisManager->FinishNtuple(state.calibrationNtupleId);

    state.chargeMapId = analysisManager->CreateH2("CollectedChargeMap",
                                                  "2D collected charge map reconstruction",
                                                  32, 0, 32, 32, 0, 32);
    analysisManager->SetH2Activation(state.chargeMapId, true);
}

std::vector<G4String> BB7DetectorModule::GetSummaryLabels() const {
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    std::vector<G4String> labels;
    labels.reserve(copyNumbers.size());
    for (const auto copyNumber : copyNumbers) {
        labels.push_back(GetSummaryLabel(copyNumber));
    }
    return labels;
}

G4String BB7DetectorModule::GetSummaryLabel(G4int detectorID) const {
    return GetName() + "[" + std::to_string(detectorID) + "]";
}

void BB7DetectorModule::ProcessEvent(const G4Event* event,
                                     G4AnalysisManager* analysisManager,
                                     G4DigiManager* digiManager,
                                     DetectorRuntimeState& runtimeState) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateAnalysisIdsOrThrow(
        state.signalNtupleId, state.calibrationNtupleId, state.chargeMapId, GetName());
    auto* digitizer = DetectorModuleUtils::GetDigitizerOrThrow<BB7Digitizer>(
        digiManager, "BB7Digitizer", GetName(), "BB7DetectorModule::GetDigitizerOrThrow");
    digitizer->Digitize();

    if (state.digitsCollectionId < 0) {
        state.digitsCollectionId = DetectorModuleUtils::GetDigiCollectionIdOrThrow(
            digiManager, "BB7DigitsCollection", GetName(), "BB7DetectorModule::GetDigiCollectionIdOrThrow");
    }
    auto* digitsCollection =
        static_cast<const BB7DigitsCollection*>(digiManager->GetDigiCollection(state.digitsCollectionId));
    if (digitsCollection == nullptr) {
        return;
    }

    struct EventTotals {
        G4double totalEdep = 0.;
        G4double totalCollectedCharge = 0.;
        G4double totalDose = 0.;
        MD1::CalibratedDoseToWaterData estimatedDoseToWater;
    };
    std::map<G4int, EventTotals> eventTotalsByDetector;

    for (size_t i = 0; i < digitsCollection->entries(); ++i) {
        auto* digit = (*digitsCollection)[i];
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 1, digit->GetSensorID());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 2, digit->GetStripID());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 3, digit->GetEdep());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 4, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 5, digit->GetDose());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 6, event->GetEventID());
        analysisManager->AddNtupleRow(state.signalNtupleId);

        const BB7DoseCalibrator calibrator(fGeometry->GetCalibrationParameters(digit->GetDetectorID()));
        const auto calibratedDose = calibrator.Calibrate(*digit);
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 1, digit->GetSensorID());
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 2, digit->GetStripID());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 3, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 4, calibratedDose.estimatedDoseToWater);
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 5, calibratedDose.calibrationError);
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 6, event->GetEventID());
        analysisManager->AddNtupleRow(state.calibrationNtupleId);

        for (size_t pStripID = 0; pStripID < 32; ++pStripID) {
            if (digit->GetSensorID() == 0) {
                analysisManager->FillH2(
                    state.chargeMapId, digit->GetStripID(), pStripID, digit->GetCollectedCharge() / 32.);
            }
            if (digit->GetSensorID() == 1) {
                analysisManager->FillH2(
                    state.chargeMapId, pStripID, digit->GetStripID(), digit->GetCollectedCharge() / 32.);
            }
        }

        auto& eventTotals = eventTotalsByDetector[digit->GetDetectorID()];
        eventTotals.totalEdep += digit->GetEdep();
        eventTotals.totalCollectedCharge += digit->GetCollectedCharge();
        eventTotals.totalDose += digit->GetDose();
        eventTotals.estimatedDoseToWater.Add(calibratedDose.estimatedDoseToWater,
                                             calibratedDose.calibrationError);
    }

    for (const auto& [detectorID, eventTotals] : eventTotalsByDetector) {
        auto& summary = state.resultsAccumulable.FindOrCreate(GetSummaryLabel(detectorID));
        summary.AddEvent(eventTotals.totalEdep,
                         eventTotals.totalCollectedCharge,
                         eventTotals.totalDose,
                         eventTotals.estimatedDoseToWater);
    }
}

void BB7DetectorModule::PrintResults(const G4Run* /*run*/,
                                     const DetectorRuntimeState& runtimeState,
                                     const MD1::DetectorPrintContext& context) const {
    const auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    MD1::PrintDosimetricDetectorResults(
        "BB7", GetSummaryLabels(), state.resultsAccumulable, context);
}
