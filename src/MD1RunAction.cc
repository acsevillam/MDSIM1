/*
 *
 * Geant4 MultiDetector Simulation v1
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

#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>

#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4ScoringManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "analysis/RunSummaryUtils.hh"
#include "geometry/base/DetectorRegistry.hh"
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1RunAction.hh"
#include "MD1RunActionMessenger.hh"

namespace MD1 {

namespace {

const DetectorRunAccumulables* FindDetectorAccumulablesByName(
    const std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const G4String& detectorName) {
    const auto it = std::find_if(detectorAccumulables.begin(),
                                 detectorAccumulables.end(),
                                 [&](const auto& accumulators) {
                                     return accumulators.name == detectorName;
                                 });
    return (it != detectorAccumulables.end()) ? &(*it) : nullptr;
}

} // namespace

MD1RunAction::MD1RunAction()
    : G4UserRunAction(),
      fEDepEvents(0),
      fTotalEdep("TotalEdep"),
      fCollectedCharge("CollectedCharge"),
      fDose("Dose"),
      fEstimatedDoseToWater("EstimatedDoseToWater") {

    fRunActionMessenger = std::make_unique<MD1RunActionMessenger>(this);

    auto* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEDepEvents);
    accumulableManager->Register(&fTotalEdep);
    accumulableManager->Register(&fCollectedCharge);
    accumulableManager->Register(&fDose);
    accumulableManager->Register(&fEstimatedDoseToWater);

    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetActivation(true);
    analysisManager->SetVerboseLevel(0);
    analysisManager->SetFileName("analysis/DetectorReadout.root");
    CreateHistos();
}

MD1RunAction::~MD1RunAction() {
}

void MD1RunAction::BuildActiveDetectors() {
    fActiveDetectors.clear();
    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        auto runtimeEntryIt =
            std::find_if(fDetectorRuntimeEntries.begin(),
                         fDetectorRuntimeEntries.end(),
                         [&](const auto& runtimeEntry) { return runtimeEntry.detector == detector; });

        if (runtimeEntryIt == fDetectorRuntimeEntries.end()) {
            DetectorRuntimeEntry runtimeEntry;
            runtimeEntry.detector = detector;
            runtimeEntry.runtimeState = detector->CreateRuntimeState();
            fDetectorRuntimeEntries.push_back(std::move(runtimeEntry));
            runtimeEntryIt = std::prev(fDetectorRuntimeEntries.end());
        }

        ActiveDetectorRuntime activeDetector;
        activeDetector.detector = detector;
        activeDetector.runtimeState = runtimeEntryIt->runtimeState.get();
        fActiveDetectors.push_back(activeDetector);
    }
}

std::vector<G4String> MD1RunAction::GetActiveSummaryLabels() const {
    std::vector<G4String> summaryLabels;
    for (const auto& activeDetector : fActiveDetectors) {
        const auto detectorLabels = activeDetector.detector->GetSummaryLabels();
        summaryLabels.insert(summaryLabels.end(), detectorLabels.begin(), detectorLabels.end());
    }
    return summaryLabels;
}

void MD1RunAction::SetMU(G4int MU) {
    if (MU <= 0) {
        G4Exception("MD1RunAction::SetMU",
                    "InvalidMonitorUnits",
                    FatalException,
                    "Monitor units must be strictly positive.");
        return;
    }
    fSimulatedMU = MU;
}

void MD1RunAction::SetScaleFactorMU(G4double scaleFactorMU) {
    if (scaleFactorMU <= 0.) {
        G4Exception("MD1RunAction::SetScaleFactorMU",
                    "InvalidScaleFactorMU",
                    FatalException,
                    "Scale factor per MU must be strictly positive.");
        return;
    }

    fScaleFactorMU = scaleFactorMU;
}

void MD1RunAction::SetScaleFactorMUError(G4double scaleFactorMUError) {
    if (scaleFactorMUError < 0.) {
        G4Exception("MD1RunAction::SetScaleFactorMUError",
                    "InvalidScaleFactorMUError",
                    FatalException,
                    "Scale factor per MU uncertainty must be non-negative.");
        return;
    }

    fScaleFactorMUError = scaleFactorMUError;
}

void MD1RunAction::RegisterDetectorDigitizers(G4DigiManager* digiManager) {
    if (digiManager == nullptr) {
        return;
    }
    for (const auto& activeDetector : fActiveDetectors) {
        activeDetector.detector->RegisterDigitizers(digiManager);
    }
}

void MD1RunAction::BeginOfRunAction(const G4Run*) {
    BuildActiveDetectors();
    fDetectorAccumulables.PrepareForRun(IsMaster());
    if (auto* sdManager = G4SDManager::GetSDMpointerIfExist(); sdManager != nullptr) {
        for (const auto& activeDetector : fActiveDetectors) {
            activeDetector.detector->RegisterSensitiveDetectors(sdManager);
        }
    }
    RegisterDetectorDigitizers(G4DigiManager::GetDMpointer());
    CreateNTuples();

    auto* analysisManager = G4AnalysisManager::Instance();
    if (analysisManager->IsActive()) {
        analysisManager->OpenFile();
    }

    auto* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();
}

void MD1RunAction::EndOfRunAction(const G4Run* run) {
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;

    auto* analysisManager = G4AnalysisManager::Instance();
    if (analysisManager->IsActive()) {
        analysisManager->Write();
        analysisManager->CloseFile();
    }

    auto* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();
    fDetectorAccumulables.MergeWorkerResults(IsMaster());
    G4int eDepEvents = fEDepEvents.GetValue();
    const auto edepValues = RunSummary::BuildStatDisplayValues(fTotalEdep.GetStat(), nofEvents);
    const auto chargeValues = RunSummary::BuildStatDisplayValues(fCollectedCharge.GetStat(), nofEvents);
    const auto doseValues = RunSummary::BuildStatDisplayValues(fDose.GetStat(), nofEvents);
    const auto globalChargeScaledValues =
        RunSummary::BuildScaledStatDisplayValues(chargeValues,
                                                 fScaleFactorMU,
                                                 fScaleFactorMUError,
                                                 fSimulatedMU);
    const auto globalDoseScaledValues =
        RunSummary::BuildScaledStatDisplayValues(doseValues,
                                                 fScaleFactorMU,
                                                 fScaleFactorMUError,
                                                 fSimulatedMU);
    const auto estimatedDoseToWaterValues =
        RunSummary::BuildStatDisplayValues(fEstimatedDoseToWater.GetStat(), nofEvents);
    const auto detectorAccumulablesForSummary =
        fDetectorAccumulables.GetAccumulablesForSummary(IsMaster());
    const auto globalEstimatedDoseToWaterScaledValues =
        RunSummary::BuildScaledStatDisplayValues(
            estimatedDoseToWaterValues, fScaleFactorMU, fScaleFactorMUError, fSimulatedMU);
    const auto activeSummaryLabels = GetActiveSummaryLabels();
    const G4double globalEstimatedDoseToWaterCalibrationError =
        RunSummary::BuildCombinedCalibrationErrorPerEvent(detectorAccumulablesForSummary,
                                                          activeSummaryLabels,
                                                          nofEvents);
    const G4double globalEstimatedDoseToWaterScaledCalibrationError =
        globalEstimatedDoseToWaterCalibrationError * fScaleFactorMU * fSimulatedMU;
    const G4double globalEstimatedDoseToWaterTotalError =
        std::sqrt(globalEstimatedDoseToWaterScaledValues.mcError *
                      globalEstimatedDoseToWaterScaledValues.mcError +
                  globalEstimatedDoseToWaterScaledValues.muError *
                      globalEstimatedDoseToWaterScaledValues.muError +
                  globalEstimatedDoseToWaterScaledCalibrationError *
                      globalEstimatedDoseToWaterScaledCalibrationError);
    const G4double globalEstimatedDoseToWaterRelativeCalibrationErrorPercent =
        RunSummary::BuildRelativePercent(globalEstimatedDoseToWaterScaledCalibrationError,
                                         globalEstimatedDoseToWaterScaledValues.mean);
    const G4double globalEstimatedDoseToWaterRelativeTotalErrorPercent =
        RunSummary::BuildRelativePercent(globalEstimatedDoseToWaterTotalError,
                                         globalEstimatedDoseToWaterScaledValues.mean);

    if (IsMaster()) {
        G4cout << G4endl
               << "--------------------End of Global Run-----------------------" << G4endl
               << "(1)  Total events per run: " << nofEvents << G4endl
               << "(2)  Number of detected events per event: "
               << G4double(eDepEvents) / G4double(nofEvents) << G4endl
               << "     [Total number of detected events (2) / Total events per run (1)]" << G4endl
               << "(3)  Mean deposited energy per event in sensitive volume: "
               << G4BestUnit(edepValues.meanPerEvent, "Energy")
               << " mc_err = " << G4BestUnit(edepValues.mcError, "Energy")
               << " (" << edepValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(edepValues.rms, "Energy") << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(4)  Mean dose per event in detector sensitive volume: "
               << G4BestUnit(doseValues.meanPerEvent, "Dose")
               << " mc_err = " << G4BestUnit(doseValues.mcError, "Dose")
               << " (" << doseValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(doseValues.rms, "Dose") << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(5)  Mean collected charge per event in sensitive volume: "
               << chargeValues.meanPerEvent / (1e-9*coulomb) << " nC"
               << " mc_err = " << chargeValues.mcError / (1e-9*coulomb) << " nC"
               << " (" << chargeValues.relativeMcErrorPercent << " %)"
               << " rms = " << chargeValues.rms / (1e-9*coulomb) << " nC" << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(6)  Calculated total dose in detector sensitive volume ("<<fSimulatedMU<<"UM): "
               << globalDoseScaledValues.mean / (1e-2*gray) <<" cGy"
               << " mc_err = " << globalDoseScaledValues.mcError / (1e-2*gray) << " cGy"
               << " (" << globalDoseScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << globalDoseScaledValues.muError / (1e-2*gray) << " cGy"
               << " (" << globalDoseScaledValues.relativeMuErrorPercent << " %)"
               << " total_err = " << globalDoseScaledValues.totalError / (1e-2*gray) << " cGy"
               << " (" << globalDoseScaledValues.relativeTotalErrorPercent << " %)"
               << " rms = " << globalDoseScaledValues.rms / (1e-2*gray) << " cGy" << G4endl
               << "     [Mean dose per event in detector sensitive volume (4) * fScaleFactorMU * fSimulatedMU]" << G4endl
               << "(7)  Calculated total collected charge ("<<fSimulatedMU<<"UM): "
               << globalChargeScaledValues.mean / (1e-9*coulomb) << " nC"
               << " mc_err = " << globalChargeScaledValues.mcError / (1e-9*coulomb) << " nC"
               << " (" << globalChargeScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << globalChargeScaledValues.muError / (1e-9*coulomb) << " nC"
               << " (" << globalChargeScaledValues.relativeMuErrorPercent << " %)"
               << " total_err = " << globalChargeScaledValues.totalError / (1e-9*coulomb) << " nC"
               << " (" << globalChargeScaledValues.relativeTotalErrorPercent << " %)"
               << " rms = " << globalChargeScaledValues.rms / (1e-9*coulomb) << " nC" << G4endl
               << "     [Mean collected charge per event (5) * fScaleFactorMU * fSimulatedMU]" << G4endl
               << "(8)  Estimated total absorbed dose in water ("<<fSimulatedMU<<"UM): "
               << globalEstimatedDoseToWaterScaledValues.mean / (1e-2*gray) <<" cGy"
               << " mc_err = " << globalEstimatedDoseToWaterScaledValues.mcError / (1e-2*gray) << " cGy"
               << " (" << globalEstimatedDoseToWaterScaledValues.relativeMcErrorPercent << " %)"
               << " mu_err = " << globalEstimatedDoseToWaterScaledValues.muError / (1e-2*gray) << " cGy"
               << " (" << globalEstimatedDoseToWaterScaledValues.relativeMuErrorPercent << " %)"
               << " det_err = " << globalEstimatedDoseToWaterScaledCalibrationError / (1e-2*gray) << " cGy"
               << " (" << globalEstimatedDoseToWaterRelativeCalibrationErrorPercent << " %)"
               << " total_err = " << globalEstimatedDoseToWaterTotalError / (1e-2*gray) << " cGy"
               << " (" << globalEstimatedDoseToWaterRelativeTotalErrorPercent << " %)"
               << " rms = " << globalEstimatedDoseToWaterScaledValues.rms / (1e-2*gray) << " cGy" << G4endl
               << "     [Estimated absorbed dose in water per event from detector calibration * fScaleFactorMU * fSimulatedMU]"
               << G4endl
               << "------------------------------------------------------------" << G4endl;

        G4double scale_factor = fScaleFactorMU * fSimulatedMU;
        G4ScoringManager* scoringManager = G4ScoringManager::GetScoringManager();

        for (auto itr = 0; itr < scoringManager->GetNumberOfMesh(); ++itr){
            scoringManager->SetFactor(scale_factor);
            G4String meshName = scoringManager->GetMesh(itr)->GetWorldName();
            if(meshName.find("at") != std::string::npos){
                auto scoringMesh = scoringManager->FindMesh(meshName);
                auto scoreMap = scoringMesh->GetScoreMap();
                G4String psName = "dose";
                auto score = scoreMap[psName]->GetMap();
                G4cout << G4endl
                    << "------------------ Dose " << meshName << " ---------------------" << G4endl;
                for (auto itr = score->begin(); itr != score->end(); ++itr){
                    if(itr->second->n()>0){
                        G4StatDouble stat = *(itr->second);
                        stat.scale(scale_factor);
                        const auto meanDose = stat.mean(nofEvents);
                        const auto rms = stat.rms(nofEvents, nofEvents);
                        G4cout<<itr->first<<"\t"<< meanDose/(1e-2*gray) <<" cGy"
                              << " rms = " << rms/(1e-2*gray) << " cGy"
                              << " nEntries = " << itr->second->n() << G4endl;
                    }
                }
                G4cout << "------------------------------------------------------------" << G4endl;
            }
            scoringManager->DumpAllQuantitiesToFile(meshName,"analysis/"+meshName+".out");
        }

        for (const auto& summaryLabel : activeSummaryLabels) {
            const auto* detectorAccumulables =
                FindDetectorAccumulablesByName(detectorAccumulablesForSummary, summaryLabel);
            if (detectorAccumulables == nullptr) {
                const DetectorRunAccumulables emptyDetectorAccumulables(summaryLabel);
                RunSummary::PrintDetectorSummary(emptyDetectorAccumulables,
                                                nofEvents,
                                                fSimulatedMU,
                                                fScaleFactorMU,
                                                fScaleFactorMUError);
                continue;
            }
            RunSummary::PrintDetectorSummary(*detectorAccumulables,
                                            nofEvents,
                                            fSimulatedMU,
                                            fScaleFactorMU,
                                            fScaleFactorMUError);
        }
    }
}

void MD1RunAction::AddDetectorTotals(const G4String& detectorSummaryLabel,
                                     G4double edep,
                                     G4double collectedCharge,
                                     G4double dose,
                                     const CalibratedDoseToWaterData& estimatedDoseToWater) {
    auto& accumulators = fDetectorAccumulables.FindOrCreate(detectorSummaryLabel);

    accumulators.events += 1;
    accumulators.totalEdep.fill(edep);
    accumulators.collectedCharge.fill(collectedCharge);
    accumulators.dose.fill(dose);
    accumulators.estimatedDoseToWater.Fill(estimatedDoseToWater);
}

void MD1RunAction::CreateNTuples() {
    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetNtupleMerging(true);

    for (const auto& activeDetector : fActiveDetectors) {
        activeDetector.detector->CreateAnalysis(analysisManager, *activeDetector.runtimeState);
    }
}

void MD1RunAction::CreateHistos() {}

} // namespace MD1
