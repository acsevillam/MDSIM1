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

#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

#include "MD1EventAction.hh"
#include "MD1RunAction.hh"
#include "geometry/base/DetectorEventData.hh"
#include "geometry/base/DetectorRegistry.hh"

namespace MD1 {

MD1EventAction::MD1EventAction(MD1RunAction* runAction)
    : G4UserEventAction(), fRunAction(runAction) {
    auto* digiManager = G4DigiManager::GetDMpointer();
    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->RegisterDigitizers(digiManager);
    }
}

void MD1EventAction::BeginOfEventAction(const G4Event* event) {
    G4int nEvents = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEventToBeProcessed();
    if (nEvents > 10) {
        G4int fraction = G4int(nEvents / 100);
        if (fraction > 0 && event->GetEventID() % fraction == 0) {
            G4cout << "(" << (event->GetEventID() / (nEvents * 1.0) * 100) << " %)" << G4endl;
        }
    } else {
        G4int fraction = G4int(nEvents / 1);
        if (fraction > 0 && event->GetEventID() % fraction == 0) {
            G4cout << "(" << (event->GetEventID() / (nEvents * 1.0) * 100) << " %)" << G4endl;
        }
    }

    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        detector->BeginOfEvent(event);
    }
}

void MD1EventAction::EndOfEventAction(const G4Event* event) {
    auto* analysisManager = G4AnalysisManager::Instance();
    auto* digiManager = G4DigiManager::GetDMpointer();

    DetectorEventData totalData;
    for (auto* detector : DetectorRegistry::GetInstance()->GetActiveDetectors()) {
        const auto detectorData = detector->ProcessEvent(event, analysisManager, digiManager);
        totalData.totalEdep += detectorData.totalEdep;
        totalData.totalCollectedCharge += detectorData.totalCollectedCharge;
        totalData.totalDose += detectorData.totalDose;
        totalData.totalEstimatedDoseToWater += detectorData.totalEstimatedDoseToWater;
        totalData.hasSignal = totalData.hasSignal || detectorData.hasSignal;
        if (detectorData.hasSignal) {
            fRunAction->AddDetectorTotals(detectorData.detectorName,
                                          detectorData.totalEdep,
                                          detectorData.totalCollectedCharge,
                                          detectorData.totalDose,
                                          detectorData.totalEstimatedDoseToWater);
        }
    }

    if (totalData.hasSignal) {
        fRunAction->AddTotalEdep(totalData.totalEdep);
        fRunAction->AddTotalCollectedCharge(totalData.totalCollectedCharge);
        fRunAction->AddTotalDose(totalData.totalDose);
        fRunAction->AddTotalEstimatedDoseToWater(totalData.totalEstimatedDoseToWater);
        fRunAction->CountEdepEvent();
    }
}

} // namespace MD1
