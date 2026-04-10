#include "geometry/detectors/BB7/BB7DetectorModule.hh"

#include <algorithm>
#include <map>

#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4SDManager.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/base/DetectorModuleUtils.hh"
#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"
#include "geometry/detectors/BB7/readout/BB7Digit.hh"
#include "geometry/detectors/BB7/readout/BB7Digitizer.hh"
#include "geometry/detectors/BB7/readout/BB7ReadoutModel.hh"
#include "geometry/detectors/BB7/readout/BB7SensitiveDetector.hh"

namespace {

struct BB7DetectorRuntimeState final : DetectorRuntimeState {
    G4int ntupleId = -1;
    G4int chargeMapId = -1;
    G4int digitsCollectionId = -1;
};

void ValidateAnalysisIdsOrThrow(G4int ntupleId, G4int chargeMapId, const G4String& detectorName) {
    if (ntupleId < 0 || chargeMapId < 0) {
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

void BB7DetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                       DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.ntupleId >= 0) {
        return;
    }

    state.ntupleId = analysisManager->CreateNtuple("BB7Hits", "Variables related to BB7 detector hits");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "SensorID");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "StripID");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "EventID");
    analysisManager->FinishNtuple(state.ntupleId);

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

DetectorEventData BB7DetectorModule::ProcessEvent(const G4Event* event,
                                                  G4AnalysisManager* analysisManager,
                                                  G4DigiManager* digiManager,
                                                  DetectorRuntimeState& runtimeState) {
    DetectorEventData eventData;
    if (!fEnabled || !HasPlacedGeometry()) {
        return eventData;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateAnalysisIdsOrThrow(state.ntupleId, state.chargeMapId, GetName());
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
        return eventData;
    }

    std::map<G4int, std::size_t> instanceIndices;

    auto getInstanceData = [&](G4int detectorID) -> DetectorInstanceEventData& {
        const auto it = instanceIndices.find(detectorID);
        if (it != instanceIndices.end()) {
            return eventData.instanceData[it->second];
        }

        DetectorInstanceEventData instanceData;
        instanceData.summaryLabel = GetSummaryLabel(detectorID);
        eventData.instanceData.push_back(instanceData);
        const std::size_t instanceIndex = eventData.instanceData.size() - 1;
        instanceIndices.emplace(detectorID, instanceIndex);
        return eventData.instanceData[instanceIndex];
    };

    for (size_t i = 0; i < digitsCollection->entries(); ++i) {
        auto* digit = (*digitsCollection)[i];
        analysisManager->FillNtupleDColumn(state.ntupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(state.ntupleId, 1, digit->GetSensorID());
        analysisManager->FillNtupleDColumn(state.ntupleId, 2, digit->GetStripID());
        analysisManager->FillNtupleDColumn(state.ntupleId, 3, digit->GetEdep());
        analysisManager->FillNtupleDColumn(state.ntupleId, 4, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(state.ntupleId, 5, digit->GetDose());
        analysisManager->FillNtupleDColumn(state.ntupleId, 6, digit->GetEstimatedDoseToWater());
        analysisManager->FillNtupleDColumn(state.ntupleId, 7, event->GetEventID());
        analysisManager->AddNtupleRow(state.ntupleId);

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

        eventData.totalEdep += digit->GetEdep();
        eventData.totalCollectedCharge += digit->GetCollectedCharge();
        eventData.totalDose += digit->GetDose();
        eventData.totalEstimatedDoseToWater += digit->GetEstimatedDoseToWater();
        eventData.hasSignal = true;

        auto& instanceData = getInstanceData(digit->GetDetectorID());
        instanceData.totalEdep += digit->GetEdep();
        instanceData.totalCollectedCharge += digit->GetCollectedCharge();
        instanceData.totalDose += digit->GetDose();
        instanceData.estimatedDoseToWater.Add(digit->GetEstimatedDoseToWater(),
                                              digit->GetEstimatedDoseToWaterCalibrationError());
    }

    return eventData;
}
