#include "geometry/detectors/scintCube/readout/ScintCubePhotosensorModel.hh"

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
                    "ScintCubeInvalidEfficiency",
                    FatalException,
                    (name + " must be inside [0, 1].").c_str());
    }
}

void ValidateNonNegativeTime(const char* scope,
                             const G4String& name,
                             G4double value) {
    if (value < 0.) {
        G4Exception(scope,
                    "ScintCubeInvalidTimeParameter",
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

ScintCubePMTModel::ScintCubePMTModel(const ScintCubePMTParameters& parameters)
    : fParameters(parameters) {
    ValidateEfficiency("ScintCubePMTModel::ScintCubePMTModel",
                       "PMT quantum efficiency",
                       fParameters.quantumEfficiency);
    ValidateEfficiency("ScintCubePMTModel::ScintCubePMTModel",
                       "PMT dynode collection efficiency",
                       fParameters.dynodeCollectionEfficiency);
    ValidateNonNegativeTime("ScintCubePMTModel::ScintCubePMTModel",
                            "PMT transit time",
                            fParameters.transitTime);
    ValidateNonNegativeTime("ScintCubePMTModel::ScintCubePMTModel",
                            "PMT transit time spread",
                            fParameters.transitTimeSpread);
}

ScintCubePhotosensorResponse ScintCubePMTModel::SampleResponse(G4double collectedPhotons,
                                                               G4double weight,
                                                               G4double resolutionScale) const {
    ScintCubePhotosensorResponse response;
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

ScintCubeSiPMModel::ScintCubeSiPMModel(const ScintCubeSiPMParameters& parameters)
    : fParameters(parameters) {
    ValidateEfficiency("ScintCubeSiPMModel::ScintCubeSiPMModel",
                       "SiPM photo-detection efficiency",
                       fParameters.photoDetectionEfficiency);
    if (fParameters.microcellCount <= 0.) {
        G4Exception("ScintCubeSiPMModel::ScintCubeSiPMModel",
                    "ScintCubeInvalidMicrocellCount",
                    FatalException,
                    "SiPM microcell count must be strictly positive.");
    }
    if (fParameters.excessNoiseFactor < 1.) {
        G4Exception("ScintCubeSiPMModel::ScintCubeSiPMModel",
                    "ScintCubeInvalidExcessNoiseFactor",
                    FatalException,
                    "SiPM excess noise factor must be greater or equal to 1.");
    }
    ValidateNonNegativeTime("ScintCubeSiPMModel::ScintCubeSiPMModel",
                            "SiPM avalanche time",
                            fParameters.avalancheTime);
    ValidateNonNegativeTime("ScintCubeSiPMModel::ScintCubeSiPMModel",
                            "SiPM avalanche time spread",
                            fParameters.avalancheTimeSpread);
}

ScintCubePhotosensorResponse ScintCubeSiPMModel::SampleResponse(G4double collectedPhotons,
                                                                G4double weight,
                                                                G4double resolutionScale) const {
    ScintCubePhotosensorResponse response;
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

std::unique_ptr<ScintCubePhotosensorModel> MakeScintCubePhotosensorModel(
    ScintCubePhotosensorType type,
    const ScintCubePMTParameters& pmtParameters,
    const ScintCubeSiPMParameters& sipmParameters) {
    switch (type) {
        case ScintCubePhotosensorType::PMT:
            return std::make_unique<ScintCubePMTModel>(pmtParameters);
        case ScintCubePhotosensorType::SiPM:
            return std::make_unique<ScintCubeSiPMModel>(sipmParameters);
    }

    G4Exception("MakeScintCubePhotosensorModel",
                "ScintCubeInvalidPhotosensorType",
                FatalException,
                "Unsupported scintCube photosensor type.");
    return nullptr;
}
