#include "geometry/detectors/BB7/BB7DetectorModule.hh"

#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"
#include "geometry/detectors/BB7/readout/BB7Digit.hh"
#include "geometry/detectors/BB7/readout/BB7Digitizer.hh"
#include "geometry/detectors/BB7/readout/BB7SensitiveDetector.hh"

const G4double BB7DetectorModule::kStaticCalibrationFactor = 1.0 * gray / coulomb;

BB7DetectorModule::BB7DetectorModule()
    : fEnabled(false), fNtupleId(-1), fChargeMapId(-1), fGeometry(std::make_unique<DetectorDualBB7>()) {}

BB7DetectorModule::~BB7DetectorModule() = default;

G4bool BB7DetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void BB7DetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void BB7DetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* bb7SD = new BB7SensitiveDetector("BB7SD");
    sdManager->AddNewDetector(bb7SD);

    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("SdCube", false);
    if (logicalVolume != nullptr) {
        logicalVolume->SetSensitiveDetector(bb7SD);
    }
}

void BB7DetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }
    digiManager->AddNewModule(new BB7Digitizer("BB7Digitizer", kStaticCalibrationFactor));
}

void BB7DetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager) {
    fNtupleId = analysisManager->CreateNtuple("BB7Hits", "Variables related to BB7 detector hits");
    analysisManager->CreateNtupleDColumn(fNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(fNtupleId, "SensorID");
    analysisManager->CreateNtupleDColumn(fNtupleId, "StripID");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "EventID");
    analysisManager->FinishNtuple(fNtupleId);

    fChargeMapId = analysisManager->CreateH2("CollectedChargeMap",
                                             "2D collected charge map reconstruction",
                                             32, 0, 32, 32, 0, 32);
    analysisManager->SetH2Activation(fChargeMapId, true);
}

DetectorEventData BB7DetectorModule::ProcessEvent(const G4Event* event,
                                                  G4AnalysisManager* analysisManager,
                                                  G4DigiManager* digiManager) {
    DetectorEventData eventData;
    eventData.detectorName = GetName();
    if (!fEnabled || !HasPlacedGeometry()) {
        return eventData;
    }

    auto* digitizer = static_cast<BB7Digitizer*>(digiManager->FindDigitizerModule("BB7Digitizer"));
    if (digitizer == nullptr) {
        return eventData;
    }
    digitizer->Digitize();

    const auto collectionId = digiManager->GetDigiCollectionID("BB7DigitsCollection");
    auto* digitsCollection = static_cast<const BB7DigitsCollection*>(digiManager->GetDigiCollection(collectionId));
    if (digitsCollection == nullptr) {
        return eventData;
    }

    for (size_t i = 0; i < digitsCollection->entries(); ++i) {
        auto* digit = (*digitsCollection)[i];
        analysisManager->FillNtupleDColumn(fNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(fNtupleId, 1, digit->GetSensorID());
        analysisManager->FillNtupleDColumn(fNtupleId, 2, digit->GetStripID());
        analysisManager->FillNtupleDColumn(fNtupleId, 3, digit->GetEdep());
        analysisManager->FillNtupleDColumn(fNtupleId, 4, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(fNtupleId, 5, digit->GetDose());
        analysisManager->FillNtupleDColumn(fNtupleId, 6, digit->GetEstimatedDoseToWater());
        analysisManager->FillNtupleDColumn(fNtupleId, 7, event->GetEventID());
        analysisManager->AddNtupleRow(fNtupleId);

        for (size_t pStripID = 0; pStripID < 32; ++pStripID) {
            if (digit->GetSensorID() == 0) {
                analysisManager->FillH2(fChargeMapId, digit->GetStripID(), pStripID, digit->GetCollectedCharge() / 32.);
            }
            if (digit->GetSensorID() == 1) {
                analysisManager->FillH2(fChargeMapId, pStripID, digit->GetStripID(), digit->GetCollectedCharge() / 32.);
            }
        }

        eventData.totalEdep += digit->GetEdep();
        eventData.totalCollectedCharge += digit->GetCollectedCharge();
        eventData.totalDose += digit->GetDose();
        eventData.totalEstimatedDoseToWater += digit->GetEstimatedDoseToWater();
        eventData.hasSignal = true;
    }

    return eventData;
}
