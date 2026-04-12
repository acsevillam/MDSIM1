#include "geometry/detectors/model11/readout/Model11Digitizer.hh"

#include <algorithm>
#include <cmath>

#include "G4DigiManager.hh"
#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace {

void ValidateReadoutParameters(const Model11ReadoutParameters& parameters,
                               G4int detectorID) {
    const G4String detectorLabel = "model11[" + std::to_string(detectorID) + "]";
    if (parameters.scintillationYield <= 0.) {
        G4Exception("Model11Digitizer::ValidateReadoutParameters",
                    "Model11InvalidScintillationYield",
                    FatalException,
                    ("Scintillation yield must be strictly positive for " + detectorLabel + ".").c_str());
    }
    if (parameters.birksConstant < 0.) {
        G4Exception("Model11Digitizer::ValidateReadoutParameters",
                    "Model11InvalidBirksConstant",
                    FatalException,
                    ("Birks constant must be non-negative for " + detectorLabel + ".").c_str());
    }
    if (parameters.lightCollectionEfficiency < 0. || parameters.lightCollectionEfficiency > 1.) {
        G4Exception("Model11Digitizer::ValidateReadoutParameters",
                    "Model11InvalidLightCollectionEfficiency",
                    FatalException,
                    ("Light collection efficiency must be inside [0, 1] for " + detectorLabel + ".").c_str());
    }
    if (parameters.decayTime < 0. ||
        parameters.transportDelay < 0. ||
        parameters.timeJitter < 0.) {
        G4Exception("Model11Digitizer::ValidateReadoutParameters",
                    "Model11InvalidTimingParameter",
                    FatalException,
                    ("Timing parameters must be non-negative for " + detectorLabel + ".").c_str());
    }
    if (parameters.resolutionScale < 0.) {
        G4Exception("Model11Digitizer::ValidateReadoutParameters",
                    "Model11InvalidResolutionScale",
                    FatalException,
                    ("Resolution scale must be non-negative for " + detectorLabel + ".").c_str());
    }
}

G4double ComputeVisibleEnergy(const Model11ReadoutParameters& parameters,
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

Model11Digitizer::Model11Digitizer(
    const G4String& name,
    std::map<G4int, Model11ReadoutParameters> readoutParametersByDetector)
    : BaseDigitizer(name, "model11"),
      fHitsCollection(nullptr),
      fDigitsCollection(nullptr),
      fDCID(-1),
      fReadoutParametersByDetector(std::move(readoutParametersByDetector)) {
    collectionName.push_back("Model11DigitsCollection");

    for (const auto& [detectorID, parameters] : fReadoutParametersByDetector) {
        ValidateReadoutParameters(parameters, detectorID);
        fPhotosensorModels[detectorID] = MakeModel11PhotosensorModel(
            parameters.photosensorType,
            parameters.pmtParameters,
            parameters.sipmParameters);
    }
}

Model11Digitizer::~Model11Digitizer() = default;

void Model11Digitizer::Digitize() {
    fDigitsCollection = new Model11DigitsCollection(moduleName, collectionName[0]);
    auto* digitManager = G4DigiManager::GetDMpointer();

    const G4int hcID = digitManager->GetHitsCollectionID("Model11HitsCollection");
    if (hcID < 0) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }

    fHitsCollection =
        static_cast<const Model11HitsCollection*>(digitManager->GetHitsCollection(hcID));
    if (fHitsCollection == nullptr) {
        StoreDigiCollection(fDigitsCollection);
        return;
    }

    struct EventSignal {
        Model11PhotosensorType photosensorType = Model11PhotosensorType::PMT;
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
            G4Exception("Model11Digitizer::Digitize",
                        "Model11MissingReadoutParameters",
                        FatalException,
                        ("No readout parameters found for model11 detectorID " +
                         std::to_string(hit->GetDetectorID()) + ".").c_str());
            continue;
        }

        const auto& parameters = parametersIt->second;
        const auto modelIt = fPhotosensorModels.find(hit->GetDetectorID());
        if (modelIt == fPhotosensorModels.end() || modelIt->second == nullptr) {
            G4Exception("Model11Digitizer::Digitize",
                        "Model11MissingPhotosensorModel",
                        FatalException,
                        ("No photosensor model found for model11 detectorID " +
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

        auto newDigit = std::make_unique<Model11Digit>();
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
