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

// Geant4 Headers
#include "G4DigiManager.hh"

// MultiDetector Headers
#include "geometry/detectors/basic/core/BasicDosimeterReadout.hh"
#include "geometry/detectors/basic/cube/readout/CubeDigitizer.hh"

CubeDigitizer::CubeDigitizer(const G4String& name, const CubeReadoutParameters& readoutParameters)
    : BaseDigitizer(name, "cube"),
      fHitsCollection(nullptr),
      fDigitsCollection(nullptr),
      fDCID(-1),
      fReadoutParameters(readoutParameters) {
    collectionName.push_back("CubeDigitsCollection");
}

CubeDigitizer::~CubeDigitizer() = default;

void CubeDigitizer::Digitize() {
    fDigitsCollection = new CubeDigitsCollection(moduleName, collectionName[0]);
    auto* digitManager = G4DigiManager::GetDMpointer();
    const G4double sensitiveMass = MD1::GetBasicDosimeterSensitiveVolumeMassOrThrow(
        "DetectorCube", "CubeDigitizer::GetSensitiveVolumeMass", "CubeSensitiveVolumeNotFound");

    const G4int hcID = digitManager->GetHitsCollectionID("CubeHitsCollection");
    if (hcID < 0) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }
    fHitsCollection = static_cast<const CubeHitsCollection*>(digitManager->GetHitsCollection(hcID));

    if (fHitsCollection != nullptr) {
        for (size_t i = 0; i < fHitsCollection->GetSize(); ++i) {
            auto* hit = (*fHitsCollection)[i];
            const G4double edep = hit->GetEdep();
            const G4double weight = hit->GetWeight();

            if (edep > 0.) {
                const G4double collectedCharge =
                    MD1::ComputeBasicDosimeterCollectedCharge(edep, fReadoutParameters);
                const G4double weightedEdep = edep * weight;
                const G4double weightedCollectedCharge = collectedCharge * weight;
                auto newDigit = std::make_unique<CubeDigit>();
                newDigit->SetDetectorID(hit->GetDetectorID());
                newDigit->SetEdep(weightedEdep);
                newDigit->SetCollectedCharge(weightedCollectedCharge);
                newDigit->SetDose(weightedEdep / sensitiveMass);
                fDigitsCollection->insert(newDigit.release());
            }
        }
    }

    StoreDigiCollection(fDigitsCollection);
}
