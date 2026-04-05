#include "geometry/detectors/cube/CubeDetectorModule.hh"

#include "G4AnalysisManager.hh"
#include "G4DigiManager.hh"
#include "G4Event.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "geometry/detectors/cube/geometry/DetectorCube.hh"
#include "geometry/detectors/cube/readout/CubeDigit.hh"
#include "geometry/detectors/cube/readout/CubeDigitizer.hh"
#include "geometry/detectors/cube/readout/CubeReadoutModel.hh"
#include "geometry/detectors/cube/readout/CubeSensitiveDetector.hh"

CubeDetectorModule::CubeDetectorModule()
    : fEnabled(false), fNtupleId(-1), fGeometry(std::make_unique<DetectorCube>(2.0 * mm, "G4_Si")) {}

CubeDetectorModule::~CubeDetectorModule() = default;

G4bool CubeDetectorModule::HasPlacedGeometry() const {
    return fGeometry->HasPlacementRequests();
}

void CubeDetectorModule::ConstructGeometry(G4LogicalVolume* motherVolume) {
    if (!fEnabled || motherVolume == nullptr) {
        return;
    }
    if (!HasPlacedGeometry()) {
        return;
    }

    if (!CubeReadoutModel::HasCalibrationFactor(fGeometry->GetCubeMaterial(),
                                                fGeometry->GetCubeSide(),
                                                fGeometry->GetCalibrationFactor())) {
        G4Exception("CubeDetectorModule::ConstructGeometry",
                    "CubeCalibrationMissing",
                    FatalException,
                    ("Cube detector is enabled but no calibration factor was found for material " +
                     fGeometry->GetCubeMaterial() + " and the configured cube side.").c_str());
        return;
    }

    fGeometry->AssembleRequestedGeometries();
}

void CubeDetectorModule::RegisterSensitiveDetectors(G4SDManager* sdManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    auto* cubeSD = new CubeSensitiveDetector("CubeSD");
    sdManager->AddNewDetector(cubeSD);

    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("DetectorCube", false);
    if (logicalVolume != nullptr) {
        logicalVolume->SetSensitiveDetector(cubeSD);
    }
}

void CubeDetectorModule::RegisterDigitizers(G4DigiManager* digiManager) {
    if (!fEnabled || !HasPlacedGeometry()) {
        return;
    }

    const auto readoutParameters = CubeReadoutModel::Build(
        fGeometry->GetCubeMaterial(),
        fGeometry->GetCubeSide(),
        fGeometry->GetCalibrationFactor());
    digiManager->AddNewModule(new CubeDigitizer("CubeDigitizer", readoutParameters));
}

void CubeDetectorModule::CreateAnalysis(G4AnalysisManager* analysisManager) {
    fNtupleId = analysisManager->CreateNtuple("CubeHits", "Variables related to cube detector hits");
    analysisManager->CreateNtupleDColumn(fNtupleId, "DetectorID");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Edep[eV]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Charge[coulomb]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "Dose[Gy]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "EstimatedDoseToWater[Gy]");
    analysisManager->CreateNtupleDColumn(fNtupleId, "EventID");
    analysisManager->FinishNtuple(fNtupleId);
}

DetectorEventData CubeDetectorModule::ProcessEvent(const G4Event* event,
                                                   G4AnalysisManager* analysisManager,
                                                   G4DigiManager* digiManager) {
    DetectorEventData eventData;
    eventData.detectorName = GetName();
    if (!fEnabled || !HasPlacedGeometry()) {
        return eventData;
    }

    auto* digitizer = static_cast<CubeDigitizer*>(digiManager->FindDigitizerModule("CubeDigitizer"));
    if (digitizer == nullptr) {
        return eventData;
    }
    digitizer->Digitize();

    const auto collectionId = digiManager->GetDigiCollectionID("CubeDigitsCollection");
    auto* digitsCollection = static_cast<const CubeDigitsCollection*>(digiManager->GetDigiCollection(collectionId));
    if (digitsCollection == nullptr) {
        return eventData;
    }

    for (size_t i = 0; i < digitsCollection->entries(); ++i) {
        auto* digit = (*digitsCollection)[i];
        analysisManager->FillNtupleDColumn(fNtupleId, 0, digit->GetDetectorID());
        analysisManager->FillNtupleDColumn(fNtupleId, 1, digit->GetEdep());
        analysisManager->FillNtupleDColumn(fNtupleId, 2, digit->GetCollectedCharge());
        analysisManager->FillNtupleDColumn(fNtupleId, 3, digit->GetDose());
        analysisManager->FillNtupleDColumn(fNtupleId, 4, digit->GetEstimatedDoseToWater());
        analysisManager->FillNtupleDColumn(fNtupleId, 5, event->GetEventID());
        analysisManager->AddNtupleRow(fNtupleId);

        eventData.totalEdep += digit->GetEdep();
        eventData.totalCollectedCharge += digit->GetCollectedCharge();
        eventData.totalDose += digit->GetDose();
        eventData.totalEstimatedDoseToWater += digit->GetEstimatedDoseToWater();
        eventData.hasSignal = true;
    }

    return eventData;
}
