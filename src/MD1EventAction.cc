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
#include "G4EventManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4DigiManager.hh"

// MultiDetector Headers
#include "MD1EventAction.hh"
#include "MD1RunAction.hh"
#include "BB7SensitiveDetector.hh"
#include "BB7Hit.hh"
#include "BB7Digit.hh"
#include "BB7Digitizer.hh"

namespace MD1 {

MD1EventAction::MD1EventAction(MD1RunAction* runAction)
    : G4UserEventAction(), fRunAction(runAction), fHCID(-1), fDCID(-1) {
    auto* digitizer = new BB7Digitizer("BB7Digitizer");
    G4DigiManager::GetDMpointer()->AddNewModule(digitizer);
}

// Obtiene la colección de hits dada su ID
BB7HitsCollection* MD1EventAction::GetHitsCollection(G4int hcID, const G4Event* event) const {
    auto hitsCollection = (BB7HitsCollection*) (event->GetHCofThisEvent()->GetHC(hcID));
    if (!hitsCollection) {
        G4ExceptionDescription msg;
        msg << "Cannot access hitsCollection ID " << hcID;
        G4Exception("EventAction::GetHitsCollection()", "MyCode0003", FatalException, msg);
    }
    return hitsCollection;
}

// Obtiene la colección de hits dada su ID
BB7DigitsCollection* MD1EventAction::GetDigitsCollection(G4int dcID, G4DigiManager* digiManager) const{
    auto digitsCollection =(BB7DigitsCollection*) (digiManager->GetDigiCollection(dcID));
    if (!digitsCollection) {
        G4ExceptionDescription msg;
        msg << "Cannot access digitsCollection ID " << dcID;
        G4Exception("EventAction::GetDigitsCollection()", "MyCode0004", FatalException, msg);
    }
    return digitsCollection;
}

void MD1EventAction::BeginOfEventAction(const G4Event* event) {
    // Inicialización al comienzo del evento si es necesario

    G4int nEvents = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed() ;
	if(nEvents>10){
		G4int 	fraction 	= G4int(nEvents/100) ;
		if(event->GetEventID()%fraction == 0)
			G4cout<<"("<<(event->GetEventID()/(nEvents*1.0)*100)<<" %)"<<G4endl ;

	}else {
		G4int 	fraction 	= G4int(nEvents/1) ;
		if(event->GetEventID()%fraction == 0)
			G4cout<<"("<<(event->GetEventID()/(nEvents*1.0)*100)<<" %)"<<G4endl ;
	}
}

void MD1EventAction::EndOfEventAction(const G4Event* event) {

    // Rellenar ntuple, histogramas
    auto analysisManager = G4AnalysisManager::Instance();

    // Obtener el ID de la colección de hits
    if (fHCID == -1) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID("BB7HitsCollection");
    }
    
    // Obtener la colección de hits
    auto HC = GetHitsCollection(fHCID, event);

    G4double totalEdep1 = 0.;

    if (HC) {
        for (size_t i = 0; i < HC->entries(); ++i) {

            if((*HC)[i]->GetEdep() > 0.) {
                totalEdep1 += (*HC)[i]->GetEdep();
            }
        }
    }

    auto digiManager = G4DigiManager::GetDMpointer();
    auto *digitizer = (BB7Digitizer*) digiManager->FindDigitizerModule("BB7Digitizer");
    digitizer->Digitize();

    // Obtener el ID de la colección de dígitos
    if (fDCID == -1) {
        fDCID = G4DigiManager::GetDMpointer()->GetDigiCollectionID("BB7DigitsCollection");
    }


    // Obtener la colección de dígitos
    auto DC = GetDigitsCollection(fDCID, digiManager);

    G4double totalCharge = 0., totalEdep2 = 0., totalDose = 0.;

    G4int detectorID, sensorID, stripID;
    G4double eDep, collectedCharge, dose;

    if (DC != nullptr) {
        // Procesar la colección de dígitos
        for (size_t i = 0; i < DC->entries(); ++i) {
            BB7Digit* digit = (*DC)[i];
            // Acceder a los datos del dígito
            detectorID = digit->GetDetectorID();
            sensorID = digit->GetSensorID();
            stripID = digit->GetStripID();
            eDep = digit->GetEdep();
            collectedCharge = digit->GetCollectedCharge();
            dose = digit->GetDose();

            analysisManager->FillNtupleDColumn(0, 0, detectorID);
            analysisManager->FillNtupleDColumn(0, 1, sensorID);
            analysisManager->FillNtupleDColumn(0, 2, stripID);
            analysisManager->FillNtupleDColumn(0, 3, eDep);
            analysisManager->FillNtupleDColumn(0, 4, collectedCharge);
            analysisManager->FillNtupleDColumn(0, 5, dose);
            analysisManager->FillNtupleDColumn(0, 6, event->GetEventID());
            analysisManager->AddNtupleRow();

            for (size_t pStripID = 0; pStripID < 32; ++pStripID) {
                if(sensorID==0)
                    analysisManager->FillH2(0,stripID,pStripID,collectedCharge/32.);
                if(sensorID==1)
                    analysisManager->FillH2(0,pStripID,stripID,collectedCharge/32.);
            }

            totalEdep2 += eDep;
            totalCharge += collectedCharge;
            totalDose += dose;
        }
    }

    if (totalCharge > 0.) {
        fRunAction->AddTotalEdep(totalEdep2);
        fRunAction->AddTotalCollectedCharge(totalCharge);
        fRunAction->AddTotalDose(totalDose);
        fRunAction->CountEdepEvent();
    }
}

} // namespace MD1
