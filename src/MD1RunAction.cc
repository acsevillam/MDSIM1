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

#include "geometry/base/DetectorRegistry.hh"
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1RunAction.hh"
#include "MD1RunActionMessenger.hh"

namespace MD1 {

MD1RunAction::MD1RunAction()
    : G4UserRunAction() {
    fRunActionMessenger = std::make_unique<MD1RunActionMessenger>(this);

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
    if (auto* sdManager = G4SDManager::GetSDMpointerIfExist(); sdManager != nullptr) {
        for (const auto& activeDetector : fActiveDetectors) {
            activeDetector.detector->RegisterSensitiveDetectors(sdManager);
        }
    }
    RegisterDetectorDigitizers(G4DigiManager::GetDMpointer());
    CreateNTuples();

    for (const auto& activeDetector : fActiveDetectors) {
        activeDetector.detector->PrepareForRun(*activeDetector.runtimeState, IsMaster());
    }

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
    for (const auto& activeDetector : fActiveDetectors) {
        activeDetector.detector->MergeRunResults(*activeDetector.runtimeState, IsMaster());
    }

    if (IsMaster()) {
        const MD1::DetectorPrintContext printContext{
            nofEvents,
            fSimulatedMU,
            fScaleFactorMU,
            fScaleFactorMUError,
        };

        for (const auto& activeDetector : fActiveDetectors) {
            activeDetector.detector->PrintResults(run, *activeDetector.runtimeState, printContext);
        }

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
    }
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
