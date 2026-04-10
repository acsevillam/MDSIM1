#include "geometry/detectors/cube/CubeDetectorModule.hh"

#include <algorithm>
#include <map>

#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/base/DetectorModuleUtils.hh"
#include "geometry/detectors/cube/geometry/DetectorCube.hh"
#include "geometry/detectors/cube/readout/CubeDigit.hh"
#include "geometry/detectors/cube/readout/CubeDigitizer.hh"
#include "geometry/detectors/cube/readout/CubeReadoutModel.hh"
#include "geometry/detectors/cube/readout/CubeSensitiveDetector.hh"

namespace {

struct CubeDetectorRuntimeState final : DetectorRuntimeState {
    G4int ntupleId = -1;
    G4int digitsCollectionId = -1;
};

void ValidateNtupleIdOrThrow(G4int ntupleId, const G4String& detectorName) {
    if (ntupleId < 0) {
        G4Exception("CubeDetectorModule::ValidateNtupleIdOrThrow",
                    "DetectorAnalysisNotInitialized",
                    FatalException,
                    ("Detector module " + detectorName +
                     " has not created its analysis ntuple before event processing.").c_str());
    }
}

CubeDetectorRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                                 const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<CubeDetectorRuntimeState>(
        runtimeState, detectorName, "CubeDetectorModule::GetRuntimeStateOrThrow");
}

} // namespace

CubeDetectorModule::CubeDetectorModule()
    : fEnabled(false), fGeometry(std::make_unique<DetectorCube>(2.0 * mm, "G4_Si")) {}

CubeDetectorModule::~CubeDetectorModule() = default;

G4bool CubeDetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void CubeDetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    if (!CubeReadoutModel::HasCalibrationFactor(fGeometry->GetCubeMaterial(),
                                                fGeometry->GetCubeSide(),
                                                fGeometry->GetEnvelopeMaterial(),
                                                fGeometry->GetEnvelopeThickness(),
                                                fGeometry->GetCalibrationFactor())) {
        G4Exception("CubeDetectorModule::ConstructGeometry",
                    "CubeCalibrationMissing",
                    FatalException,
                    ("Cube detector is enabled but no calibration factor was found for material " +
                     fGeometry->GetCubeMaterial() +
                     ", side " + std::to_string(fGeometry->GetCubeSide() / mm) + " mm, envelope material " +
                     fGeometry->GetEnvelopeMaterial() +
                     " and envelope thickness " + std::to_string(fGeometry->GetEnvelopeThickness() / mm) +
                     " mm.").c_str());
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void CubeDetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* cubeSD =
        DetectorModuleUtils::GetOrCreateSensitiveDetector<CubeSensitiveDetector>(sdManager, "CubeSD");

    DetectorModuleUtils::GetLogicalVolumeOrThrow(
        "DetectorCube", GetName(), "CubeDetectorModule::GetLogicalVolumeOrThrow");
    fGeometry->AttachSensitiveDetector(cubeSD);
}

void CubeDetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    if (digiManager->FindDigitizerModule("CubeDigitizer") != nullptr) {
        return;
    }

    const auto readoutParameters = CubeReadoutModel::Build(
        fGeometry->GetCubeMaterial(),
        fGeometry->GetCubeSide(),
        fGeometry->GetEnvelopeMaterial(),
        fGeometry->GetEnvelopeThickness(),
        fGeometry->GetCalibrationFactor(),
        fGeometry->GetCalibrationFactorError());
    digiManager->AddNewModule(new CubeDigitizer("CubeDigitizer", readoutParameters));
}

std::unique_ptr<DetectorRuntimeState> CubeDetectorModule::CreateRuntimeState() const {
    return std::make_unique<CubeDetectorRuntimeState>();
}

void CubeDetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                        DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.ntupleId >= 0) {
        return;
    }

    state.ntupleId = analysisManager->CreateNtuple("CubeHits", "Variables related to cube detector hits");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.ntupleId, "EventID");
    analysisManager->FinishNtuple(state.ntupleId);
}

std::vector<G4String> CubeDetectorModule::GetSummaryLabels() const {
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    std::vector<G4String> labels;
    labels.reserve(copyNumbers.size());
    for (const auto copyNumber : copyNumbers) {
        labels.push_back(GetSummaryLabel(copyNumber));
    }
    return labels;
}

G4String CubeDetectorModule::GetSummaryLabel(G4int detectorID) const {
    return GetName() + "[" + std::to_string(detectorID) + "]";
}

DetectorEventData CubeDetectorModule::ProcessEvent(const G4Event* event,
                                                   G4AnalysisManager* analysisManager,
                                                   G4DigiManager* digiManager,
                                                   DetectorRuntimeState& runtimeState) {
    DetectorEventData eventData;
    if (!fEnabled || !HasPlacedGeometry()) {
        return eventData;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateNtupleIdOrThrow(state.ntupleId, GetName());
    auto* digitizer = DetectorModuleUtils::GetDigitizerOrThrow<CubeDigitizer>(
        digiManager, "CubeDigitizer", GetName(), "CubeDetectorModule::GetDigitizerOrThrow");
    digitizer->Digitize();

    if (state.digitsCollectionId < 0) {
        state.digitsCollectionId = DetectorModuleUtils::GetDigiCollectionIdOrThrow(
            digiManager, "CubeDigitsCollection", GetName(), "CubeDetectorModule::GetDigiCollectionIdOrThrow");
    }
    auto* digitsCollection =
        static_cast<const CubeDigitsCollection*>(digiManager->GetDigiCollection(state.digitsCollectionId));
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
        analysisManager->FillNtupleDColumn(state.ntupleId, 1, digit->GetEdep());
        analysisManager->FillNtupleDColumn(state.ntupleId, 2, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(state.ntupleId, 3, digit->GetDose());
        analysisManager->FillNtupleDColumn(state.ntupleId, 4, digit->GetEstimatedDoseToWater());
        analysisManager->FillNtupleDColumn(state.ntupleId, 5, event->GetEventID());
        analysisManager->AddNtupleRow(state.ntupleId);

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
