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

#include "geometry/detectors/basic/cylinder/CylinderDetectorModule.hh"

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
#include "geometry/detectors/basic/cylinder/calibration/CylinderDoseCalibrator.hh"
#include "geometry/detectors/basic/cylinder/geometry/DetectorCylinder.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderDigit.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderDigitizer.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderReadoutModel.hh"
#include "geometry/detectors/basic/cylinder/readout/CylinderSensitiveDetector.hh"

namespace {

struct CylinderDetectorRuntimeState final : DetectorRuntimeState {
    G4int signalNtupleId = -1;
    G4int calibrationNtupleId = -1;
    G4int digitsCollectionId = -1;
    MD1::DosimetricDetectorResultsAccumulable resultsAccumulable{"CylinderResults"};
    G4bool resultsRegistered = false;
};

void ValidateAnalysisIdsOrThrow(G4int signalNtupleId,
                                G4int calibrationNtupleId,
                                const G4String& detectorName) {
    if (signalNtupleId < 0 || calibrationNtupleId < 0) {
        G4Exception("CylinderDetectorModule::ValidateAnalysisIdsOrThrow",
                    "DetectorAnalysisNotInitialized",
                    FatalException,
                    ("Detector module " + detectorName +
                     " has not created its analysis ntuples before event processing.").c_str());
    }
}

CylinderDetectorRuntimeState& GetRuntimeStateOrThrow(DetectorRuntimeState& runtimeState,
                                                     const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<CylinderDetectorRuntimeState>(
        runtimeState, detectorName, "CylinderDetectorModule::GetRuntimeStateOrThrow");
}

const CylinderDetectorRuntimeState& GetRuntimeStateOrThrow(const DetectorRuntimeState& runtimeState,
                                                           const G4String& detectorName) {
    return DetectorModuleUtils::GetRuntimeStateOrThrow<CylinderDetectorRuntimeState>(
        runtimeState, detectorName, "CylinderDetectorModule::GetRuntimeStateOrThrow");
}

} // namespace

CylinderDetectorModule::CylinderDetectorModule()
    : fEnabled(false),
      fGeometry(std::make_unique<DetectorCylinder>(1.0 * mm, 2.0 * mm, "G4_Si")) {}

CylinderDetectorModule::~CylinderDetectorModule() = default;

G4bool CylinderDetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void CylinderDetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    if (!CylinderDoseCalibrator::HasCalibrationFactor(fGeometry->GetCylinderMaterial(),
                                                      fGeometry->GetCylinderRadius(),
                                                      fGeometry->GetCylinderHeight(),
                                                      fGeometry->GetEnvelopeMaterial(),
                                                      fGeometry->GetEnvelopeThickness(),
                                                      fGeometry->GetCalibrationFactor())) {
        G4Exception("CylinderDetectorModule::ConstructGeometry",
                    "CylinderCalibrationMissing",
                    FatalException,
                    ("Cylinder detector is enabled but no calibration factor was found for material " +
                     fGeometry->GetCylinderMaterial() +
                     ", radius " + std::to_string(fGeometry->GetCylinderRadius() / mm) +
                     " mm, height " + std::to_string(fGeometry->GetCylinderHeight() / mm) +
                     " mm, envelope material " + fGeometry->GetEnvelopeMaterial() +
                     " and envelope thickness " +
                     std::to_string(fGeometry->GetEnvelopeThickness() / mm) + " mm.").c_str());
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void CylinderDetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* cylinderSD =
        DetectorModuleUtils::GetOrCreateSensitiveDetector<CylinderSensitiveDetector>(sdManager,
                                                                                     "CylinderSD");

    DetectorModuleUtils::GetLogicalVolumeOrThrow("DetectorCylinder",
                                                 GetName(),
                                                 "CylinderDetectorModule::GetLogicalVolumeOrThrow");
    fGeometry->AttachSensitiveDetector(cylinderSD);
}

void CylinderDetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    if (digiManager->FindDigitizerModule("CylinderDigitizer") != nullptr) {
        return;
    }

    const auto readoutParameters =
        CylinderReadoutModel::Build(fGeometry->GetCylinderMaterial(),
                                    fGeometry->GetCylinderRadius(),
                                    fGeometry->GetCylinderHeight(),
                                    fGeometry->GetEnvelopeMaterial(),
                                    fGeometry->GetEnvelopeThickness());
    digiManager->AddNewModule(new CylinderDigitizer("CylinderDigitizer", readoutParameters));
}

std::unique_ptr<DetectorRuntimeState> CylinderDetectorModule::CreateRuntimeState() const {
    return std::make_unique<CylinderDetectorRuntimeState>();
}

void CylinderDetectorModule::PrepareForRun(DetectorRuntimeState& runtimeState, G4bool /*isMaster*/) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (!state.resultsRegistered) {
        G4AccumulableManager::Instance()->Register(&state.resultsAccumulable);
        state.resultsRegistered = true;
    }
}

void CylinderDetectorModule::MergeRunResults(DetectorRuntimeState& /*runtimeState*/,
                                             G4bool /*isMaster*/) {}

void CylinderDetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager,
                                            DetectorRuntimeState& runtimeState) {
    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    if (state.signalNtupleId >= 0 || state.calibrationNtupleId >= 0) {
        return;
    }

    state.signalNtupleId = analysisManager->CreateNtuple(
        "CylinderHits", "Physical readout variables related to cylinder detector hits");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(state.signalNtupleId, "EventID");
    analysisManager->FinishNtuple(state.signalNtupleId);

    state.calibrationNtupleId =
        analysisManager->CreateNtuple("CylinderDoseCalibration", "Cylinder calibrated dose output");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "CalibrationError[Gy]");
    analysisManager->CreateNtupleDColumn(state.calibrationNtupleId, "EventID");
    analysisManager->FinishNtuple(state.calibrationNtupleId);
}

std::vector<G4String> CylinderDetectorModule::GetSummaryLabels() const {
    auto copyNumbers = fGeometry->GetPlacementCopyNumbers();
    std::sort(copyNumbers.begin(), copyNumbers.end());

    std::vector<G4String> labels;
    labels.reserve(copyNumbers.size());
    for (const auto copyNumber : copyNumbers) {
        labels.push_back(GetSummaryLabel(copyNumber));
    }
    return labels;
}

G4String CylinderDetectorModule::GetSummaryLabel(G4int detectorID) const {
    return GetName() + "[" + std::to_string(detectorID) + "]";
}

void CylinderDetectorModule::ProcessEvent(const G4Event* event,
                                          G4AnalysisManager* analysisManager,
                                          G4DigiManager* digiManager,
                                          DetectorRuntimeState& runtimeState) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    ValidateAnalysisIdsOrThrow(state.signalNtupleId, state.calibrationNtupleId, GetName());
    auto* digitizer = DetectorModuleUtils::GetDigitizerOrThrow<CylinderDigitizer>(
        digiManager, "CylinderDigitizer", GetName(), "CylinderDetectorModule::GetDigitizerOrThrow");
    digitizer->Digitize();

    const CylinderDoseCalibrator calibrator(CylinderDoseCalibrator::BuildParameters(
        fGeometry->GetCylinderMaterial(),
        fGeometry->GetCylinderRadius(),
        fGeometry->GetCylinderHeight(),
        fGeometry->GetEnvelopeMaterial(),
        fGeometry->GetEnvelopeThickness(),
        fGeometry->GetCalibrationFactor(),
        fGeometry->GetCalibrationFactorError()));

    if (state.digitsCollectionId < 0) {
        state.digitsCollectionId =
            DetectorModuleUtils::GetDigiCollectionIdOrThrow(digiManager,
                                                            "CylinderDigitsCollection",
                                                            GetName(),
                                                            "CylinderDetectorModule::GetDigiCollectionIdOrThrow");
    }
    auto* digitsCollection = static_cast<const CylinderDigitsCollection*>(
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

void CylinderDetectorModule::PrintResults(const G4Run* /*run*/,
                                          const DetectorRuntimeState& runtimeState,
                                          const MD1::DetectorPrintContext& context) const {
    const auto& state = GetRuntimeStateOrThrow(runtimeState, GetName());
    MD1::PrintDosimetricDetectorResults(
        "Cylinder", GetSummaryLabels(), state.resultsAccumulable, context);
}
