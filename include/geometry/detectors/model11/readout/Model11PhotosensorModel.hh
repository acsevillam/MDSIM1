#ifndef MDSIM1_MODEL11_PHOTOSENSOR_MODEL_HH
#define MDSIM1_MODEL11_PHOTOSENSOR_MODEL_HH

#include <memory>

#include "globals.hh"
#include "G4SystemOfUnits.hh"

enum class Model11PhotosensorType {
    PMT = 0,
    SiPM = 1,
};

inline G4String Model11PhotosensorTypeToString(Model11PhotosensorType type) {
    switch (type) {
        case Model11PhotosensorType::PMT:
            return "PMT";
        case Model11PhotosensorType::SiPM:
            return "SiPM";
    }

    return "Unknown";
}

struct Model11PMTParameters {
    G4double quantumEfficiency = 0.25;
    G4double dynodeCollectionEfficiency = 0.90;
    G4double transitTime = 5.0 * ns;
    G4double transitTimeSpread = 0.30 * ns;
};

struct Model11SiPMParameters {
    G4double photoDetectionEfficiency = 0.45;
    G4double microcellCount = 20000.0;
    G4double excessNoiseFactor = 1.10;
    G4double avalancheTime = 2.0 * ns;
    G4double avalancheTimeSpread = 0.15 * ns;
};

struct Model11PhotosensorResponse {
    G4double detectedPhotoelectrons = 0.;
    G4double intrinsicTimeOffset = 0.;
};

class Model11PhotosensorModel {
public:
    virtual ~Model11PhotosensorModel() = default;

    virtual Model11PhotosensorType GetType() const = 0;
    virtual G4String GetTypeName() const = 0;
    virtual Model11PhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                        G4double weight,
                                                        G4double resolutionScale) const = 0;
};

class Model11PMTModel : public Model11PhotosensorModel {
public:
    explicit Model11PMTModel(const Model11PMTParameters& parameters);

    Model11PhotosensorType GetType() const override { return Model11PhotosensorType::PMT; }
    G4String GetTypeName() const override { return "PMT"; }
    Model11PhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                G4double weight,
                                                G4double resolutionScale) const override;

private:
    Model11PMTParameters fParameters;
};

class Model11SiPMModel : public Model11PhotosensorModel {
public:
    explicit Model11SiPMModel(const Model11SiPMParameters& parameters);

    Model11PhotosensorType GetType() const override { return Model11PhotosensorType::SiPM; }
    G4String GetTypeName() const override { return "SiPM"; }
    Model11PhotosensorResponse SampleResponse(G4double collectedPhotons,
                                                G4double weight,
                                                G4double resolutionScale) const override;

private:
    Model11SiPMParameters fParameters;
};

std::unique_ptr<Model11PhotosensorModel> MakeModel11PhotosensorModel(
    Model11PhotosensorType type,
    const Model11PMTParameters& pmtParameters,
    const Model11SiPMParameters& sipmParameters);

#endif // MDSIM1_MODEL11_PHOTOSENSOR_MODEL_HH
