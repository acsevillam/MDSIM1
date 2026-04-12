#ifndef MDSIM1_SCINT_CUBE_PHOTOSENSOR_MODEL_HH
#define MDSIM1_SCINT_CUBE_PHOTOSENSOR_MODEL_HH

#include <memory>

#include "globals.hh"
#include "G4SystemOfUnits.hh"

enum class ScintCubePhotosensorType {
    PMT = 0,
    SiPM = 1,
};

inline G4String ScintCubePhotosensorTypeToString(ScintCubePhotosensorType type) {
    switch (type) {
        case ScintCubePhotosensorType::PMT:
            return "PMT";
        case ScintCubePhotosensorType::SiPM:
            return "SiPM";
    }

    return "Unknown";
}

struct ScintCubePMTParameters {
    G4double quantumEfficiency = 0.25;
    G4double dynodeCollectionEfficiency = 0.90;
    G4double transitTime = 5.0 * ns;
    G4double transitTimeSpread = 0.30 * ns;
};

struct ScintCubeSiPMParameters {
    G4double photoDetectionEfficiency = 0.45;
    G4double microcellCount = 20000.0;
    G4double excessNoiseFactor = 1.10;
    G4double avalancheTime = 2.0 * ns;
    G4double avalancheTimeSpread = 0.15 * ns;
};

struct ScintCubePhotosensorResponse {
    G4double detectedPhotoelectrons = 0.;
    G4double intrinsicTimeOffset = 0.;
};

class ScintCubePhotosensorModel {
public:
    virtual ~ScintCubePhotosensorModel() = default;

    virtual ScintCubePhotosensorType GetType() const = 0;
    virtual G4String GetTypeName() const = 0;
    virtual ScintCubePhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                        G4double weight,
                                                        G4double resolutionScale) const = 0;
};

class ScintCubePMTModel : public ScintCubePhotosensorModel {
public:
    explicit ScintCubePMTModel(const ScintCubePMTParameters& parameters);

    ScintCubePhotosensorType GetType() const override { return ScintCubePhotosensorType::PMT; }
    G4String GetTypeName() const override { return "PMT"; }
    ScintCubePhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                G4double weight,
                                                G4double resolutionScale) const override;

private:
    ScintCubePMTParameters fParameters;
};

class ScintCubeSiPMModel : public ScintCubePhotosensorModel {
public:
    explicit ScintCubeSiPMModel(const ScintCubeSiPMParameters& parameters);

    ScintCubePhotosensorType GetType() const override { return ScintCubePhotosensorType::SiPM; }
    G4String GetTypeName() const override { return "SiPM"; }
    ScintCubePhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                G4double weight,
                                                G4double resolutionScale) const override;

private:
    ScintCubeSiPMParameters fParameters;
};

std::unique_ptr<ScintCubePhotosensorModel> MakeScintCubePhotosensorModel(
    ScintCubePhotosensorType type,
    const ScintCubePMTParameters& pmtParameters,
    const ScintCubeSiPMParameters& sipmParameters);

#endif // MDSIM1_SCINT_CUBE_PHOTOSENSOR_MODEL_HH
