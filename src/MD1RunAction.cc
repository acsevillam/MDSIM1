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

// Geant4 Headers
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4ScoringManager.hh"

// MultiDetector Headers
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1RunAction.hh"

using namespace std;

namespace MD1 {

MD1RunAction::MD1RunAction()
    : G4UserRunAction(),
      fEDepEvents(0),
      fTotalEdep(0.),
      fTotalEdep2(0.),
      fCollectedCharge(0.),
      fCollectedCharge2(0.),
      fDose(0.),
      fDose2(0.){

    fRunActionMessenger = new MD1RunActionMessenger(this) ;

    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    // Register accumulable to the accumulable manager
    accumulableManager->Register(fEDepEvents);
    accumulableManager->Register(fTotalEdep);
    accumulableManager->Register(fTotalEdep2);
    accumulableManager->Register(fCollectedCharge);
    accumulableManager->Register(fCollectedCharge2);
    accumulableManager->Register(fDose);
    accumulableManager->Register(fDose2);

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetActivation(true);
    G4cout << "Using " << analysisManager->GetType() << G4endl;
    analysisManager->SetVerboseLevel(0);
    analysisManager->SetFileName("analysis/BB7Readout.root");

    // Create Histograms and N-Tuples
    CreateNTuples();
    CreateHistos();
}

void MD1RunAction::BeginOfRunAction(const G4Run*) {

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

	if(analysisManager->IsActive()){
		// Open an output file
		analysisManager->OpenFile();
	}

    // Reset accumulables to their initial values
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();
}

void MD1RunAction::EndOfRunAction(const G4Run* run) {
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;

	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

	if(analysisManager->IsActive()){
		// Save histograms and ntuples
		analysisManager->Write();
		analysisManager->CloseFile();
	}

    // Merge accumulables
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    // Compute energy deposited event
    G4int eDepEvents = fEDepEvents.GetValue();

    // Total energy deposit in a run and its variance
    G4double TotalEdep = fTotalEdep.GetValue();
    G4double TotalEdep2 = fTotalEdep2.GetValue();
    G4double rmsTotalEdep = 0.;

    if (eDepEvents > 0) {
        rmsTotalEdep = TotalEdep2 - TotalEdep * TotalEdep / eDepEvents;
        if (rmsTotalEdep > 0.) {
            rmsTotalEdep = std::sqrt(rmsTotalEdep);
        }
    }

    // Total collected charge in a run and its variance
    G4double CollectedCharge = fCollectedCharge.GetValue();
    G4double CollectedCharge2 = fCollectedCharge2.GetValue();
    G4double rmsCollectedCharge = 0.;

    if (eDepEvents > 0) {
        rmsCollectedCharge = CollectedCharge2 - CollectedCharge * CollectedCharge / eDepEvents;
        if (rmsCollectedCharge > 0.) {
            rmsCollectedCharge = std::sqrt(rmsCollectedCharge);
        }
    }

    // Total collected charge in a run and its variance
    G4double Dose = fDose.GetValue();
    G4double Dose2 = fDose2.GetValue();
    G4double rmsDose = 0.;

    if (eDepEvents > 0) {
        rmsDose = Dose2 - Dose * Dose / eDepEvents;
        if (rmsDose > 0.) {
            rmsDose = std::sqrt(rmsDose);
        }
    }

    // Print results
    if (IsMaster()) {
        G4cout << G4endl
               << "--------------------End of Global Run-----------------------" << G4endl
               << "(1)  Total events per run: " << nofEvents << G4endl
               << "(2)  Number of detected events per event: "
               << G4double(eDepEvents) / G4double(nofEvents) << G4endl
               << "     [Total number of detected events (2) / Total events per run (1)]" << G4endl
               << "(3)  Total deposited energy per event in sensitive volume: "
               << G4BestUnit(TotalEdep / nofEvents, "Energy") << " rms = "
               << G4BestUnit(rmsTotalEdep / nofEvents, "Energy") << G4endl
               << "     [Total deposited energy in sensitive volume / Total events per run (1)]" << G4endl
               << "(4)  Total collected charge per event in sensitive volume: "
               << CollectedCharge / nofEvents / (10e-9*coulomb) << " nC rms = " << rmsCollectedCharge / nofEvents / (10e-9*coulomb) << " nC" << G4endl
               << "     [Total collected charge in sensitive volume / Total events per run (1)]" << G4endl
               << "(5)  Total dose per event in sensitive volume: "
               << G4BestUnit(Dose / nofEvents, "Dose")  << " rms = " << G4BestUnit(rmsDose / nofEvents, "Dose") << G4endl
               << "     [Total collected charge in sensitive volume / Total events per run (1)]" << G4endl
               << "(6)  Calculated total collected charge ("<<fSimulatedMU<<"UM): "
               << CollectedCharge / nofEvents  / (10e-9*coulomb) * fScaleFactorMU * fSimulatedMU << "nC" << " rms = " << rmsCollectedCharge / nofEvents / (10e-9*coulomb) * fScaleFactorMU * fSimulatedMU << " nC" << G4endl
               << "     [Total collected charge per event in sensitive volume (4) * fScaleFactorMU * fSimulatedMU]" << G4endl
               << "(7)  Calculated total dose ("<<fSimulatedMU<<"UM): "
               << Dose / nofEvents * fScaleFactorMU * fSimulatedMU / (10e-2*gray) <<" cGy"<< " rms = " << rmsDose / nofEvents * fScaleFactorMU * fSimulatedMU / (10e-2*gray) << " cGy" << G4endl
               << "     [Total dose per event in sensitive volume (4) * fScaleFactorMU * fSimulatedMU]"
               << G4endl
               << "------------------------------------------------------------" << G4endl;

            G4double scale_factor = (1./nofEvents)*fScaleFactorMU*fSimulatedMU;
            G4ScoringManager* scoringManager = G4ScoringManager::GetScoringManager();
        
            G4double sum_wx, sum_wx2, nEntries, wx_rms;

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
                            //G4cout<<itr->first<<"\t"<< itr->second->sum_wx()/(1e-2*gray) <<" cGy"<< " total(val^2) = " << itr->second->sum_wx2()/(1e-2*gray)/(1e-2*gray) <<" cGy^2" << " nEntries = " << itr->second->n() <<" (without scaling to MU)"<< G4endl;
                            sum_wx = itr->second->sum_wx();
                            sum_wx2 = itr->second->sum_wx2();
                            nEntries = itr->second->n();
                            wx_rms = std::sqrt(sum_wx2 - sum_wx*sum_wx/nEntries);
                            G4cout<<itr->first<<"\t"<< sum_wx*scale_factor/(1e-2*gray) <<" cGy"<< " rms = " << wx_rms*scale_factor*scale_factor/(1e-2*gray)<<" cGy" << " nEntries = " << itr->second->n() << G4endl;
                        }
                    }
                    G4cout << "------------------------------------------------------------" << G4endl;  
                }
                scoringManager->DumpAllQuantitiesToFile(meshName,"analysis/"+meshName+".out");
            }

    }
}

void MD1RunAction::CreateNTuples() {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetNtupleMerging(true);

    // Creating ntuples
    analysisManager->SetFirstNtupleId(0);

    // id = 0
    analysisManager->CreateNtuple("BB7Hits", "Variables related to BB7 detector hits");
    //analysisManager->SetNtupleActivation(false);
    analysisManager->CreateNtupleDColumn("DetectorID");
    analysisManager->CreateNtupleDColumn("SensorID");
    analysisManager->CreateNtupleDColumn("StripID");
    analysisManager->CreateNtupleDColumn("Edep[eV]");
    analysisManager->CreateNtupleDColumn("Charge[coulomb]");
    analysisManager->CreateNtupleDColumn("Dose[Gy]");
    analysisManager->CreateNtupleDColumn("EventID");
    analysisManager->FinishNtuple();

}

void MD1RunAction::CreateHistos() {
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

    // Creating histos
    analysisManager->SetFirstHistoId(0);

    // id = 0
    analysisManager->CreateH2("CollectedChargeMap",
                              "2D collected charge map reconstruction", 
                              32, 0, 32, 32, 0, 32);
    analysisManager->SetH2Activation(true);

}

} // namespace MD1
