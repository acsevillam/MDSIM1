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

#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4ScoringManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "geometry/base/DetectorRegistry.hh"
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1RunAction.hh"

using namespace std;

namespace MD1 {

namespace {

struct StatDisplayValues {
    G4double meanPerEvent = 0.;
    G4double rms = 0.;
    G4double mcError = 0.;
    G4double relativeMcErrorPercent = 0.;
};

StatDisplayValues BuildStatDisplayValues(const G4StatDouble& stat,
                                         G4int nofEvents,
                                         G4double scale = 1.) {
    StatDisplayValues values;
    if (nofEvents <= 0) {
        return values;
    }

    G4StatDouble scaledStat = stat;
    scaledStat.scale(scale);
    values.meanPerEvent = scaledStat.mean(nofEvents);
    values.rms = scaledStat.rms(nofEvents, nofEvents);
    if (nofEvents > 0) {
        values.mcError = values.rms / std::sqrt(static_cast<G4double>(nofEvents));
    }
    if (values.meanPerEvent != 0.) {
        values.relativeMcErrorPercent = 100. * values.mcError / std::abs(values.meanPerEvent);
    }
    return values;
}

void PrintDetectorSummary(const DetectorRunAccumulables& detectorAccumulables,
                          G4int nofEvents,
                          G4int simulatedMU,
                          G4double scaleFactorMU) {
    const G4int detectedEvents = detectorAccumulables.events.GetValue();
    const auto edepValues = BuildStatDisplayValues(detectorAccumulables.totalEdep.GetStat(), nofEvents);
    const auto chargeValues = BuildStatDisplayValues(detectorAccumulables.collectedCharge.GetStat(), nofEvents);
    const auto doseValues = BuildStatDisplayValues(detectorAccumulables.dose.GetStat(), nofEvents);
    const auto estimatedDoseToWaterValues = BuildStatDisplayValues(detectorAccumulables.estimatedDoseToWater.GetStat(), nofEvents);

    G4cout << G4endl
           << "---------------- Detector Summary: " << detectorAccumulables.name << " ----------------" << G4endl
           << "(1)  Events with signal: " << detectedEvents << G4endl
           << "(2)  Fraction with signal: " << G4double(detectedEvents) / G4double(nofEvents) << G4endl
           << "(3)  Mean deposited energy per event: "
           << G4BestUnit(edepValues.meanPerEvent, "Energy")
           << " err = " << G4BestUnit(edepValues.mcError, "Energy")
           << " (" << edepValues.relativeMcErrorPercent << " %)"
           << " rms = " << G4BestUnit(edepValues.rms, "Energy") << G4endl
           << "(4)  Mean dose per event in detector sensitive volume: "
           << G4BestUnit(doseValues.meanPerEvent, "Dose")
           << " err = " << G4BestUnit(doseValues.mcError, "Dose")
           << " (" << doseValues.relativeMcErrorPercent << " %)"
           << " rms = " << G4BestUnit(doseValues.rms, "Dose") << G4endl
           << "(5)  Mean collected charge per event: "
           << chargeValues.meanPerEvent / (1e-9*coulomb) << " nC"
           << " err = " << chargeValues.mcError / (1e-9*coulomb) << " nC"
           << " (" << chargeValues.relativeMcErrorPercent << " %)"
           << " rms = " << chargeValues.rms / (1e-9*coulomb) << " nC" << G4endl
           << "(6)  Calculated total dose in detector sensitive volume (" << simulatedMU << "UM): "
           << doseValues.meanPerEvent * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy"
           << " err = " << doseValues.mcError * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy"
           << " (" << doseValues.relativeMcErrorPercent << " %)"
           << " rms = " << doseValues.rms * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy" << G4endl
           << "(7)  Scaled charge (" << simulatedMU << "UM): "
           << chargeValues.meanPerEvent / (1e-9*coulomb) * scaleFactorMU * simulatedMU << " nC"
           << " err = " << chargeValues.mcError / (1e-9*coulomb) * scaleFactorMU * simulatedMU << " nC"
           << " (" << chargeValues.relativeMcErrorPercent << " %)"
           << " rms = " << chargeValues.rms / (1e-9*coulomb) * scaleFactorMU * simulatedMU << " nC" << G4endl
           << "(8)  Estimated total absorbed dose in water (" << simulatedMU << "UM): "
           << estimatedDoseToWaterValues.meanPerEvent * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy"
           << " err = " << estimatedDoseToWaterValues.mcError * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy"
           << " (" << estimatedDoseToWaterValues.relativeMcErrorPercent << " %)"
           << " rms = " << estimatedDoseToWaterValues.rms * scaleFactorMU * simulatedMU / (1e-2*gray) << " cGy" << G4endl
           << "------------------------------------------------------------------" << G4endl
           << "Notes: rms = G4StatDouble::rms(nofEvents, nofEvents);"
           << " err = rms / sqrt(N) (" << "100 * err / |mean| %" << "), with N = total events per run."
           << G4endl;
}

} // namespace

MD1RunAction::MD1RunAction()
    : G4UserRunAction(),
      fEDepEvents(0),
      fTotalEdep("TotalEdep"),
      fCollectedCharge("CollectedCharge"),
      fDose("Dose"),
      fEstimatedDoseToWater("EstimatedDoseToWater") {

    fRunActionMessenger = new MD1RunActionMessenger(this);

    auto* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(fEDepEvents);
    accumulableManager->Register(&fTotalEdep);
    accumulableManager->Register(&fCollectedCharge);
    accumulableManager->Register(&fDose);
    accumulableManager->Register(&fEstimatedDoseToWater);

    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        fDetectorAccumulables.emplace_back(detector->GetName());
        auto& accumulators = fDetectorAccumulables.back();
        accumulableManager->Register(accumulators.events);
        accumulableManager->Register(&accumulators.totalEdep);
        accumulableManager->Register(&accumulators.collectedCharge);
        accumulableManager->Register(&accumulators.dose);
        accumulableManager->Register(&accumulators.estimatedDoseToWater);
    }

    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetActivation(true);
    G4cout << "Using " << analysisManager->GetType() << G4endl;
    analysisManager->SetVerboseLevel(0);
    analysisManager->SetFileName("analysis/DetectorReadout.root");

    CreateNTuples();
    CreateHistos();
}

void MD1RunAction::BeginOfRunAction(const G4Run*) {
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

    G4int eDepEvents = fEDepEvents.GetValue();
    const auto edepValues = BuildStatDisplayValues(fTotalEdep.GetStat(), nofEvents);
    const auto chargeValues = BuildStatDisplayValues(fCollectedCharge.GetStat(), nofEvents);
    const auto doseValues = BuildStatDisplayValues(fDose.GetStat(), nofEvents);
    const auto estimatedDoseToWaterValues = BuildStatDisplayValues(fEstimatedDoseToWater.GetStat(), nofEvents);

    if (IsMaster()) {
        G4cout << G4endl
               << "--------------------End of Global Run-----------------------" << G4endl
               << "(1)  Total events per run: " << nofEvents << G4endl
               << "(2)  Number of detected events per event: "
               << G4double(eDepEvents) / G4double(nofEvents) << G4endl
               << "     [Total number of detected events (2) / Total events per run (1)]" << G4endl
               << "(3)  Mean deposited energy per event in sensitive volume: "
               << G4BestUnit(edepValues.meanPerEvent, "Energy")
               << " err = " << G4BestUnit(edepValues.mcError, "Energy")
               << " (" << edepValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(edepValues.rms, "Energy") << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(4)  Mean dose per event in detector sensitive volume: "
               << G4BestUnit(doseValues.meanPerEvent, "Dose")
               << " err = " << G4BestUnit(doseValues.mcError, "Dose")
               << " (" << doseValues.relativeMcErrorPercent << " %)"
               << " rms = " << G4BestUnit(doseValues.rms, "Dose") << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(5)  Mean collected charge per event in sensitive volume: "
               << chargeValues.meanPerEvent / (1e-9*coulomb) << " nC"
               << " err = " << chargeValues.mcError / (1e-9*coulomb) << " nC"
               << " (" << chargeValues.relativeMcErrorPercent << " %)"
               << " rms = " << chargeValues.rms / (1e-9*coulomb) << " nC" << G4endl
               << "     [Statistics computed with G4StatDouble normalized to the full run]" << G4endl
               << "(6)  Calculated total dose in detector sensitive volume ("<<fSimulatedMU<<"UM): "
               << doseValues.meanPerEvent * fScaleFactorMU * fSimulatedMU / (1e-2*gray) <<" cGy"
               << " err = " << doseValues.mcError * fScaleFactorMU * fSimulatedMU / (1e-2*gray) << " cGy"
               << " (" << doseValues.relativeMcErrorPercent << " %)"
               << " rms = " << doseValues.rms * fScaleFactorMU * fSimulatedMU / (1e-2*gray) << " cGy" << G4endl
               << "     [Mean dose per event in detector sensitive volume (4) * fScaleFactorMU * fSimulatedMU]" << G4endl
               << "(7)  Calculated total collected charge ("<<fSimulatedMU<<"UM): "
               << chargeValues.meanPerEvent / (1e-9*coulomb) * fScaleFactorMU * fSimulatedMU << " nC"
               << " err = " << chargeValues.mcError / (1e-9*coulomb) * fScaleFactorMU * fSimulatedMU << " nC"
               << " (" << chargeValues.relativeMcErrorPercent << " %)"
               << " rms = " << chargeValues.rms / (1e-9*coulomb) * fScaleFactorMU * fSimulatedMU << " nC" << G4endl
               << "     [Mean collected charge per event (5) * fScaleFactorMU * fSimulatedMU]" << G4endl
               << "(8)  Estimated total absorbed dose in water ("<<fSimulatedMU<<"UM): "
               << estimatedDoseToWaterValues.meanPerEvent * fScaleFactorMU * fSimulatedMU / (1e-2*gray) <<" cGy"
               << " err = " << estimatedDoseToWaterValues.mcError * fScaleFactorMU * fSimulatedMU / (1e-2*gray) << " cGy"
               << " (" << estimatedDoseToWaterValues.relativeMcErrorPercent << " %)"
               << " rms = " << estimatedDoseToWaterValues.rms * fScaleFactorMU * fSimulatedMU / (1e-2*gray) << " cGy" << G4endl
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

        for (const auto& detectorAccumulables : fDetectorAccumulables) {
            PrintDetectorSummary(detectorAccumulables, nofEvents, fSimulatedMU, fScaleFactorMU);
        }
    }
}

void MD1RunAction::AddDetectorTotals(const G4String& detectorName,
                                     G4double edep,
                                     G4double collectedCharge,
                                     G4double dose,
                                     G4double estimatedDoseToWater) {
    for (auto& accumulators : fDetectorAccumulables) {
        if (accumulators.name == detectorName) {
            accumulators.events += 1;
            accumulators.totalEdep.Fill(edep);
            accumulators.collectedCharge.Fill(collectedCharge);
            accumulators.dose.Fill(dose);
            accumulators.estimatedDoseToWater.Fill(estimatedDoseToWater);
            return;
        }
    }
}

void MD1RunAction::CreateNTuples() {
    auto* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetNtupleMerging(true);

    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->CreateAnalysis(analysisManager);
    }
}

void MD1RunAction::CreateHistos() {}

} // namespace MD1
