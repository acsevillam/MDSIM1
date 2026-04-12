#include "geometry/detectors/scintCube/readout/ScintCubeDigitizer.hh"

#include <algorithm>
#include <cmath>

#include "G4DigiManager.hh"
#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace {

void ValidateReadoutParameters(const ScintCubeReadoutParameters& parameters,
                               G4int detectorID) {
    const G4String detectorLabel = "scintCube[" + std::to_string(detectorID) + "]";
    if (parameters.scintillationYield <= 0.) {
        G4Exception("ScintCubeDigitizer::ValidateReadoutParameters",
                    "ScintCubeInvalidScintillationYield",
                    FatalException,
                    ("Scintillation yield must be strictly positive for " + detectorLabel + ".").c_str());
    }
    if (parameters.birksConstant < 0.) {
        G4Exception("ScintCubeDigitizer::ValidateReadoutParameters",
                    "ScintCubeInvalidBirksConstant",
                    FatalException,
                    ("Birks constant must be non-negative for " + detectorLabel + ".").c_str());
    }
    if (parameters.lightCollectionEfficiency < 0. || parameters.lightCollectionEfficiency > 1.) {
        G4Exception("ScintCubeDigitizer::ValidateReadoutParameters",
                    "ScintCubeInvalidLightCollectionEfficiency",
                    FatalException,
                    ("Light collection efficiency must be inside [0, 1] for " + detectorLabel + ".").c_str());
    }
    if (parameters.decayTime < 0. ||
        parameters.transportDelay < 0. ||
        parameters.timeJitter < 0.) {
        G4Exception("ScintCubeDigitizer::ValidateReadoutParameters",
                    "ScintCubeInvalidTimingParameter",
                    FatalException,
                    ("Timing parameters must be non-negative for " + detectorLabel + ".").c_str());
    }
    if (parameters.resolutionScale < 0.) {
        G4Exception("ScintCubeDigitizer::ValidateReadoutParameters",
                    "ScintCubeInvalidResolutionScale",
                    FatalException,
                    ("Resolution scale must be non-negative for " + detectorLabel + ".").c_str());
    }
}

G4double ComputeVisibleEnergy(const ScintCubeReadoutParameters& parameters,
                              G4double edep,
                              G4double stepLength) {
    if (edep <= 0.) {
        return 0.;
    }
    if (parameters.birksConstant <= 0. || stepLength <= 0.) {
        return edep;
    }

    const G4double stoppingPower = edep / stepLength;
    return edep / (1. + parameters.birksConstant * stoppingPower);
}

} // namespace

ScintCubeDigitizer::ScintCubeDigitizer(
    const G4String& name,
    std::map<G4int, ScintCubeReadoutParameters> readoutParametersByDetector)
    : BaseDigitizer(name, "scintCube"),
      fHitsCollection(nullptr),
      fDigitsCollection(nullptr),
      fDCID(-1),
      fReadoutParametersByDetector(std::move(readoutParametersByDetector)) {
    collectionName.push_back("ScintCubeDigitsCollection");

    for (const auto& [detectorID, parameters] : fReadoutParametersByDetector) {
        ValidateReadoutParameters(parameters, detectorID);
        fPhotosensorModels[detectorID] = MakeScintCubePhotosensorModel(
            parameters.photosensorType,
            parameters.pmtParameters,
            parameters.sipmParameters);
    }
}

ScintCubeDigitizer::~ScintCubeDigitizer() = default;

void ScintCubeDigitizer::Digitize() {
    fDigitsCollection = new ScintCubeDigitsCollection(moduleName, collectionName[0]);
    auto* digitManager = G4DigiManager::GetDMpointer();

    const G4int hcID = digitManager->GetHitsCollectionID("ScintCubeHitsCollection");
    if (hcID < 0) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }

    fHitsCollection =
        static_cast<const ScintCubeHitsCollection*>(digitManager->GetHitsCollection(hcID));
    if (fHitsCollection == nullptr) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }

    struct EventSignal {
        ScintCubePhotosensorType photosensorType = ScintCubePhotosensorType::PMT;
        G4double edep = 0.;
        G4double visibleEnergy = 0.;
        G4double producedPhotons = 0.;
        G4double detectedPhotoelectrons = 0.;
        G4double timeNumerator = 0.;
        G4double timeJitter = 0.;
        G4bool applyTimeJitter = false;
    };

    std::map<G4int, EventSignal> eventSignalsByDetector;

    for (size_t i = 0; i < fHitsCollection->GetSize(); ++i) {
        auto* hit = (*fHitsCollection)[i];
        const auto parametersIt = fReadoutParametersByDetector.find(hit->GetDetectorID());
        if (parametersIt == fReadoutParametersByDetector.end()) {
            G4Exception("ScintCubeDigitizer::Digitize",
                        "ScintCubeMissingReadoutParameters",
                        FatalException,
                        ("No readout parameters found for scintCube detectorID " +
                         std::to_string(hit->GetDetectorID()) + ".").c_str());
            continue;
        }

        const auto& parameters = parametersIt->second;
        const auto modelIt = fPhotosensorModels.find(hit->GetDetectorID());
        if (modelIt == fPhotosensorModels.end() || modelIt->second == nullptr) {
            G4Exception("ScintCubeDigitizer::Digitize",
                        "ScintCubeMissingPhotosensorModel",
                        FatalException,
                        ("No photosensor model found for scintCube detectorID " +
                         std::to_string(hit->GetDetectorID()) + ".").c_str());
            continue;
        }

        const G4double weight = hit->GetWeight();
        const G4double weightedEdep = hit->GetEdep() * weight;
        const G4double visibleEnergy =
            ComputeVisibleEnergy(parameters, hit->GetEdep(), hit->GetStepLength()) * weight;
        const G4double producedPhotons = visibleEnergy * parameters.scintillationYield;
        const G4double collectedPhotons =
            producedPhotons * parameters.lightCollectionEfficiency;
        const auto response =
            modelIt->second->SampleResponse(collectedPhotons, weight, parameters.resolutionScale);

        auto& eventSignal = eventSignalsByDetector[hit->GetDetectorID()];
        eventSignal.photosensorType = parameters.photosensorType;
        eventSignal.edep += weightedEdep;
        eventSignal.visibleEnergy += visibleEnergy;
        eventSignal.producedPhotons += producedPhotons;
        eventSignal.detectedPhotoelectrons += response.detectedPhotoelectrons;
        eventSignal.timeJitter = parameters.timeJitter;
        if (weight == 1. && parameters.timeJitter > 0.) {
            eventSignal.applyTimeJitter = true;
        }

        if (response.detectedPhotoelectrons > 0.) {
            const G4double hitSignalTime =
                hit->GetGlobalTime() +
                parameters.decayTime +
                parameters.transportDelay +
                response.intrinsicTimeOffset;
            eventSignal.timeNumerator += response.detectedPhotoelectrons * hitSignalTime;
        }
    }

    for (const auto& [detectorID, eventSignal] : eventSignalsByDetector) {
        if (eventSignal.edep <= 0.) {
            continue;
        }

        auto newDigit = std::make_unique<ScintCubeDigit>();
        newDigit->SetDetectorID(detectorID);
        newDigit->SetPhotosensorType(eventSignal.photosensorType);
        newDigit->SetEdep(eventSignal.edep);
        newDigit->SetVisibleEnergy(eventSignal.visibleEnergy);
        newDigit->SetProducedPhotons(eventSignal.producedPhotons);
        newDigit->SetDetectedPhotoelectrons(eventSignal.detectedPhotoelectrons);

        G4double meanSignalTime = 0.;
        if (eventSignal.detectedPhotoelectrons > 0.) {
            meanSignalTime = eventSignal.timeNumerator / eventSignal.detectedPhotoelectrons;
            if (eventSignal.applyTimeJitter && eventSignal.timeJitter > 0.) {
                meanSignalTime =
                    std::max(0.0, G4RandGauss::shoot(meanSignalTime, eventSignal.timeJitter));
            }
        }
        newDigit->SetMeanSignalTime(meanSignalTime);
        fDigitsCollection->insert(newDigit.release());
    }

    StoreDigiCollection(fDigitsCollection);
}
