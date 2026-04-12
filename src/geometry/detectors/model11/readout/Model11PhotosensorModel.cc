#include "geometry/detectors/model11/readout/Model11PhotosensorModel.hh"

#include <algorithm>
#include <cmath>

#include "G4Exception.hh"
#include "G4Poisson.hh"
#include "Randomize.hh"

namespace {

G4double ClampNonNegative(G4double value) {
    return std::max(0.0, value);
}

void ValidateEfficiency(const char* scope,
                        const G4String& name,
                        G4double value) {
    if (value < 0. || value > 1.) {
        G4Exception(scope,
                    "Model11InvalidEfficiency",
                    FatalException,
                    (name + " must be inside [0, 1].").c_str());
    }
}

void ValidateNonNegativeTime(const char* scope,
                             const G4String& name,
                             G4double value) {
    if (value < 0.) {
        G4Exception(scope,
                    "Model11InvalidTimeParameter",
                    FatalException,
                    (name + " must be non-negative.").c_str());
    }
}

G4double SamplePhotoelectronCount(G4double meanValue,
                                  G4double weight,
                                  G4double sigmaScale) {
    if (meanValue <= 0.) {
        return 0.;
    }

    if (weight != 1.) {
        return meanValue;
    }

    if (meanValue < 50.) {
        return static_cast<G4double>(G4Poisson(meanValue));
    }

    const G4double sigma = std::max(0.0, sigmaScale) * std::sqrt(meanValue);
    if (sigma <= 0.) {
        return meanValue;
    }

    return ClampNonNegative(G4RandGauss::shoot(meanValue, sigma));
}

G4double SampleTimeOffset(G4double nominalTime,
                          G4double timeSpread,
                          G4double weight) {
    if (nominalTime < 0.) {
        return 0.;
    }

    if (weight != 1. || timeSpread <= 0.) {
        return nominalTime;
    }

    return ClampNonNegative(G4RandGauss::shoot(nominalTime, timeSpread));
}

} // namespace

Model11PMTModel::Model11PMTModel(const Model11PMTParameters& parameters)
    : fParameters(parameters) {
    ValidateEfficiency("Model11PMTModel::Model11PMTModel",
                       "PMT quantum efficiency",
                       fParameters.quantumEfficiency);
    ValidateEfficiency("Model11PMTModel::Model11PMTModel",
                       "PMT dynode collection efficiency",
                       fParameters.dynodeCollectionEfficiency);
    ValidateNonNegativeTime("Model11PMTModel::Model11PMTModel",
                            "PMT transit time",
                            fParameters.transitTime);
    ValidateNonNegativeTime("Model11PMTModel::Model11PMTModel",
                            "PMT transit time spread",
                            fParameters.transitTimeSpread);
}

Model11PhotosensorResponse Model11PMTModel::SampleResponse(G4double collectedPhotons,
                                                               G4double weight,
                                                               G4double resolutionScale) const {
    Model11PhotosensorResponse response;
    const G4double meanPhotoelectrons =
        std::max(0.0,
                 collectedPhotons *
                 fParameters.quantumEfficiency *
                 fParameters.dynodeCollectionEfficiency);
    response.detectedPhotoelectrons =
        SamplePhotoelectronCount(meanPhotoelectrons, weight, resolutionScale);
    response.intrinsicTimeOffset =
        SampleTimeOffset(fParameters.transitTime, fParameters.transitTimeSpread, weight);
    return response;
}

Model11SiPMModel::Model11SiPMModel(const Model11SiPMParameters& parameters)
    : fParameters(parameters) {
    ValidateEfficiency("Model11SiPMModel::Model11SiPMModel",
                       "SiPM photo-detection efficiency",
                       fParameters.photoDetectionEfficiency);
    if (fParameters.microcellCount <= 0.) {
        G4Exception("Model11SiPMModel::Model11SiPMModel",
                    "Model11InvalidMicrocellCount",
                    FatalException,
                    "SiPM microcell count must be strictly positive.");
    }
    if (fParameters.excessNoiseFactor < 1.) {
        G4Exception("Model11SiPMModel::Model11SiPMModel",
                    "Model11InvalidExcessNoiseFactor",
                    FatalException,
                    "SiPM excess noise factor must be greater or equal to 1.");
    }
    ValidateNonNegativeTime("Model11SiPMModel::Model11SiPMModel",
                            "SiPM avalanche time",
                            fParameters.avalancheTime);
    ValidateNonNegativeTime("Model11SiPMModel::Model11SiPMModel",
                            "SiPM avalanche time spread",
                            fParameters.avalancheTimeSpread);
}

Model11PhotosensorResponse Model11SiPMModel::SampleResponse(G4double collectedPhotons,
                                                                G4double weight,
                                                                G4double resolutionScale) const {
    Model11PhotosensorResponse response;
    const G4double occupancyArgument =
        std::max(0.0,
                 collectedPhotons *
                 fParameters.photoDetectionEfficiency /
                 fParameters.microcellCount);
    const G4double meanPhotoelectrons =
        std::max(0.0,
                 fParameters.microcellCount * (1. - std::exp(-occupancyArgument)));
    response.detectedPhotoelectrons =
        SamplePhotoelectronCount(meanPhotoelectrons,
                                 weight,
                                 resolutionScale * std::sqrt(fParameters.excessNoiseFactor));
    response.intrinsicTimeOffset =
        SampleTimeOffset(fParameters.avalancheTime, fParameters.avalancheTimeSpread, weight);
    return response;
}

std::unique_ptr<Model11PhotosensorModel> MakeModel11PhotosensorModel(
    Model11PhotosensorType type,
    const Model11PMTParameters& pmtParameters,
    const Model11SiPMParameters& sipmParameters) {
    switch (type) {
        case Model11PhotosensorType::PMT:
            return std::make_unique<Model11PMTModel>(pmtParameters);
        case Model11PhotosensorType::SiPM:
            return std::make_unique<Model11SiPMModel>(sipmParameters);
    }

    G4Exception("MakeModel11PhotosensorModel",
                "Model11InvalidPhotosensorType",
                FatalException,
                "Unsupported model11 photosensor type.");
    return nullptr;
}
