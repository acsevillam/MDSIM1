/*
 *
 * Geant4 MultiDetector Simulation
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

#include "geometry/detectors/basic/sphere/SphereDetectorModule.hh"

#include <algorithm>
#include <map>

#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/base/DosimetricDetectorResults.hh"
#include "geometry/base/DetectorModuleUtils.hh"
#include "geometry/detectors/basic/sphere/calibration/SphereDoseCalibrator.hh"
#include "geometry/detectors/basic/sphere/geometry/DetectorSphere.hh"
#include "geometry/detectors/basic/sphere/readout/SphereDigit.hh"
#include "geometry/detectors/basic/sphere/readout/SphereDigitizer.hh"
#include "geometry/detectors/basic/sphere/readout/SphereReadoutModel.hh"
#include "geometry/detectors/basic/sphere/readout/SphereSensitiveDetector.hh"

namespace {

struct SphereDetectorRuntimeState final : DetectorRuntimeState {
    G4int signalNtupleId = -1;
    G4int calibrationNtupleId = -1;
    G4int digitsCollectionId = -1;
    MD1::DosimetricDetectorResultsAccumulable resultsAccumulable{"SphereResults"};
    G4bool resultsRegistered = false;
};

void ValidateAnalysisIdsOrThrow(G4int signalNtupleId,
                                G4int calibrationNtupleId,
                                const G4String& detectorName) {
    if (signalNtupleId < 0 || calibrationNtupleId < 0) {
        G4Exception("SphereDetectorModule::ValidateAnalysisIdsOrThrow",
                    "DetectorAnalysisNotInitialized",
                    FatalException,
                    ("Detector module " + detectorName +
                     " has not created its analysis ntuples before event processing.").c_str());
    }
}

SphereDetectorRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                                   const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<SphereDetectorRuntimeState>(
        runtimeState, detectorName, "SphereDetectorModule::GetRuntimeStateOrThrow");
}

const SphereDetectorRuntimeState& GetRuntimeStateOrThrow(const DetectorRuntimeState& runtimeState,
                                                         const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<SphereDetectorRuntimeState>(
        runtimeState, detectorName, "SphereDetectorModule::GetRuntimeStateOrThrow");
}

} // namespace

SphereDetectorModule::SphereDetectorModule()
    : fEnabled(false), fGeometry(std::make_unique<DetectorSphere>(1.0 * mm, "G4_Si")) {}

SphereDetectorModule::~SphereDetectorModule() = default;

G4bool SphereDetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void SphereDetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    if (!SphereDoseCalibrator::HasCalibrationFactor(fGeometry->GetSphereMaterial(),
                                                    fGeometry->GetSphereRadius(),
                                                    fGeometry->GetEnvelopeMaterial(),
                                                    fGeometry->GetEnvelopeThickness(),
                                                    fGeometry->GetCalibrationFactor())) {
        G4Exception("SphereDetectorModule::ConstructGeometry",
                    "SphereCalibrationMissing",
                    FatalException,
                    ("Sphere detector is enabled but no calibration factor was found for material " +
                     fGeometry->GetSphereMaterial() + ", radius " +
                     std::to_string(fGeometry->GetSphereRadius() / mm) + " mm, envelope material " +
                     fGeometry->GetEnvelopeMaterial() + " and envelope thickness " +
                     std::to_string(fGeometry->GetEnvelopeThickness() / mm) + " mm.").c_str());
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void SphereDetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* sphereSD =
        DetectorModuleUtils::GetOrCreateSensitiveDetector<SphereSensitiveDetector>(sdManager,
                                                                                   "SphereSD");

    DetectorModuleUtils::GetLogicalVolumeOrThrow("DetectorSphere",
                                                 GetName(),
                                                 "SphereDetectorModule::GetLogicalVolumeOrThrow");
    fGeometry->AttachSensitiveDetector(sphereSD);
}

void SphereDetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    if (digiManager->FindDigitizerModule("SphereDigitizer") != nullptr) {
        return;
    }

    const auto readoutParameters =
        SphereReadoutModel::Build(fGeometry->GetSphereMaterial(),
                                  fGeometry->GetSphereRadius(),
                                  fGeometry->GetEnvelopeMaterial(),
                                  fGeometry->GetEnvelopeThickness());
    digiManager->AddNewModule(new SphereDigitizer("SphereDigitizer", readoutParameters));
}

std::unique_ptr<DetectorRuntimeState> SphereDetectorModule::CreateRuntimeState() const {
    return std::make_unique<SphereDetectorRuntimeState>();
}

void SphereDetectorModule::PrepareForRun(DetectorRuntimeState& runtimeState, G4bool /*isMaster*/) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (!state.resultsRegistered) {
        G4AccumulableManager::Instance()->Register(&state.resultsAccumulable);
        state.resultsRegistered = true;
    }
}

void SphereDetectorModule::MergeRunResults(DetectorRuntimeState& /*runtimeState*/, G4bool /*isMaster*/) {}

void SphereDetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                          DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.signalNtupleId >= 0 || state.calibrationNtupleId >= 0) {
        return;
    }

    state.signalNtupleId =
        analysisManager->CreateNtuple("SphereHits", "Physical readout variables related to sphere detector hits");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "EventID");
    analysisManager->FinishNtuple(state.signalNtupleId);

    state.calibrationNtupleId =
        analysisManager->CreateNtuple("SphereDoseCalibration", "Sphere calibrated dose output");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "CalibrationError[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EventID");
    analysisManager->FinishNtuple(state.calibrationNtupleId);
}

std::vector<G4String> SphereDetectorModule::GetSummaryLabels() const {
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    std::vector<G4String> labels;
    labels.reserve(copyNumbers.size());
    for (const auto copyNumber : copyNumbers) {
        labels.push_back(GetSummaryLabel(copyNumber));
    }
    return labels;
}

G4String SphereDetectorModule::GetSummaryLabel(G4int detectorID) const {
    return GetName() + "[" + std::to_string(detectorID) + "]";
}

void SphereDetectorModule::ProcessEvent(const G4Event* event,
                                        G4AnalysisManager* analysisManager,
                                        G4DigiManager* digiManager,
                                        DetectorRuntimeState& runtimeState) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateAnalysisIdsOrThrow(state.signalNtupleId, state.calibrationNtupleId, GetName());
    auto* digitizer = DetectorModuleUtils::GetDigitizerOrThrow<SphereDigitizer>(
        digiManager, "SphereDigitizer", GetName(), "SphereDetectorModule::GetDigitizerOrThrow");
    digitizer->Digitize();

    const SphereDoseCalibrator calibrator(SphereDoseCalibrator::BuildParameters(
        fGeometry->GetSphereMaterial(),
        fGeometry->GetSphereRadius(),
        fGeometry->GetEnvelopeMaterial(),
        fGeometry->GetEnvelopeThickness(),
        fGeometry->GetCalibrationFactor(),
        fGeometry->GetCalibrationFactorError()));

    if (state.digitsCollectionId < 0) {
        state.digitsCollectionId =
            DetectorModuleUtils::GetDigiCollectionIdOrThrow(digiManager,
                                                            "SphereDigitsCollection",
                                                            GetName(),
                                                            "SphereDetectorModule::GetDigiCollectionIdOrThrow");
    }
    auto* digitsCollection = static_cast<const SphereDigitsCollection*>(
        digiManager->GetDigiCollection(state.digitsCollectionId));
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
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 1, digit->GetEdep());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 2, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 3, digit->GetDose());
        analysisManager->FillNtupleDColumn(state.signalNtupleId, 4, event->GetEventID());
        analysisManager->AddNtupleRow(state.signalNtupleId);

        const auto calibratedDose = calibrator.Calibrate(*digit);
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 1, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 2, calibratedDose.estimatedDoseToWater);
        analysisManager->FillNtupleDColumn(
            state.calibrationNtupleId, 3, calibratedDose.calibrationError);
        analysisManager->FillNtupleDColumn(state.calibrationNtupleId, 4, event->GetEventID());
        analysisManager->AddNtupleRow(state.calibrationNtupleId);

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

void SphereDetectorModule::PrintResults(const G4Run* /*run*/,
                                        const DetectorRuntimeState& runtimeState,
                                        const MD1::DetectorPrintContext& context) const {
    const auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    MD1::PrintDosimetricDetectorResults(
        "Sphere", GetSummaryLabels(), state.resultsAccumulable, context);
}
