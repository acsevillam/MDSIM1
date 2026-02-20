/*
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
#include "G4DigiManager.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "BB7Digitizer.hh"

BB7Digitizer::BB7Digitizer(const G4String& name)
    : G4VDigitizerModule(name), fHitsCollection(nullptr), fDigitsCollection(nullptr), fDCID(-1) {
    constexpr auto DIGIT_COLLECTION_NAME{"BB7DigitsCollection"};
    collectionName.push_back(DIGIT_COLLECTION_NAME);
}

BB7Digitizer::~BB7Digitizer() = default;

void BB7Digitizer::Digitize() {

    fDigitsCollection = new BB7DigitsCollection(moduleName, collectionName[0]);
    auto* digitManager = G4DigiManager::GetDMpointer();

    G4int hcID = digitManager->GetHitsCollectionID("BB7HitsCollection");
    fHitsCollection = (BB7HitsCollection*)(digitManager->GetHitsCollection(hcID));

    if (fHitsCollection) {
        for (size_t i = 0; i < fHitsCollection->GetSize(); ++i) {
            auto* hit = (*fHitsCollection)[i];
            G4int detectorID = hit->GetDetectorID();
            G4int sensorID = hit->GetSensorID();
            G4int stripID = hit->GetStripID();
            G4double edep = hit->GetEdep();

            if (edep > 0.) {
                G4double collectedCharge = int(edep / fMeanEnergyPerIon) * feCharge;
                auto newDigit = std::make_unique<BB7Digit>();
                newDigit->SetDetectorID(detectorID);
                newDigit->SetSensorID(sensorID);
                newDigit->SetStripID(stripID);
                newDigit->SetEdep(edep / eV);
                newDigit->SetCollectedCharge(collectedCharge / coulomb);
                newDigit->SetDose(collectedCharge * fCallibrationFactor / gray);
                // newDigit->Print();
                fDigitsCollection->insert(newDigit.release());
            }
        }
    }

    StoreDigiCollection(fDigitsCollection);
}
