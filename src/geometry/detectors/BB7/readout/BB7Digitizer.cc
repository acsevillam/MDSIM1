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
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4Exception.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/readout/BB7Digitizer.hh"

namespace {

G4double GetSensitiveVolumeMass(const G4String& logicalVolumeName) {
    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(logicalVolumeName, false);
    if (logicalVolume == nullptr) {
        G4Exception("BB7Digitizer::GetSensitiveVolumeMass",
                    "BB7SensitiveVolumeNotFound",
                    FatalException,
                    ("Logical volume " + logicalVolumeName + " was not found.").c_str());
        return 0.;
    }
    return logicalVolume->GetMass(true, false);
}

} // namespace

BB7Digitizer::BB7Digitizer(const G4String& name, const BB7ReadoutParameters& readoutParameters)
    : BaseDigitizer(name, "BB7"),
      fHitsCollection(nullptr),
      fDigitsCollection(nullptr),
      fDCID(-1),
      fReadoutParameters(readoutParameters) {
    constexpr auto DIGIT_COLLECTION_NAME{"BB7DigitsCollection"};
    collectionName.push_back(DIGIT_COLLECTION_NAME);
}

BB7Digitizer::~BB7Digitizer() = default;

void BB7Digitizer::Digitize() {

    fDigitsCollection = new BB7DigitsCollection(moduleName, collectionName[0]);
    auto* digitManager = G4DigiManager::GetDMpointer();
    const G4double sensitiveMass = GetSensitiveVolumeMass("SdCube");

    G4int hcID = digitManager->GetHitsCollectionID("BB7HitsCollection");
    if (hcID < 0) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }
    fHitsCollection = static_cast<const BB7HitsCollection*>(digitManager->GetHitsCollection(hcID));

    if (fHitsCollection) {
        for (size_t i = 0; i < fHitsCollection->GetSize(); ++i) {
            auto* hit = (*fHitsCollection)[i];
            G4int detectorID = hit->GetDetectorID();
            G4int sensorID = hit->GetSensorID();
            G4int stripID = hit->GetStripID();
            G4double edep = hit->GetEdep();
            const G4double weight = hit->GetWeight();

            if (edep > 0.) {
                const G4double collectedCharge =
                    static_cast<G4int>(edep / fReadoutParameters.meanEnergyPerIon) *
                    fReadoutParameters.elementaryCharge;
                const G4double weightedEdep = edep * weight;
                const G4double weightedCollectedCharge = collectedCharge * weight;
                auto newDigit = std::make_unique<BB7Digit>();
                newDigit->SetDetectorID(detectorID);
                newDigit->SetSensorID(sensorID);
                newDigit->SetStripID(stripID);
                newDigit->SetEdep(weightedEdep);
                newDigit->SetCollectedCharge(weightedCollectedCharge);
                newDigit->SetDose(weightedEdep / sensitiveMass);
                // newDigit->Print();
                fDigitsCollection->insert(newDigit.release());
            }
        }
    }

    StoreDigiCollection(fDigitsCollection);
}
