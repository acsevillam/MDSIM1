#ifndef MDSIM1_DETECTOR_SCINT_CUBE_HH
#define MDSIM1_DETECTOR_SCINT_CUBE_HH

#include <map>
#include <vector>

#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

#include "geometry/base/GenericGeometry.hh"
#include "geometry/detectors/scintCube/calibration/ScintCubeDoseCalibrator.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeReadoutParameters.hh"

class DetectorScintCubeMessenger;
class G4VisAttributes;
class G4VSolid;

struct ScintCubeDetectorConfig {
    G4double cubeSide = 5.0 * mm;
    G4String materialName = "G4_WATER";
    G4double envelopeThickness = 0.;
    G4String envelopeMaterialName = "G4_AIR";
    G4bool splitAtInterface = false;
    ScintCubeReadoutParameters readoutParameters;
    ScintCubeCalibrationParameters calibrationParameters;
};

class DetectorScintCube : public GenericGeometry {
public:
    DetectorScintCube();
    ~DetectorScintCube() override;

    void DefineMaterials() override;
    void DefineVolumes() override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume,
                     const G4ThreeVector& position,
                     G4RotationMatrix* rotation,
                     G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) override;

    void SetCubeSide(G4int detectorID, G4double cubeSide);
    void SetCubeMaterial(G4int detectorID, const G4String& materialName);
    void SetEnvelopeThickness(G4int detectorID, G4double envelopeThickness);
    void SetEnvelopeMaterial(G4int detectorID, const G4String& materialName);
    void SetSplitAtInterface(G4int detectorID, G4bool splitAtInterface);
    void SetScintillationYield(G4int detectorID, G4double scintillationYieldPerMeV);
    void SetBirksConstant(G4int detectorID, G4double birksConstantInMmPerMeV);
    void SetLightCollectionEfficiency(G4int detectorID, G4double lightCollectionEfficiency);
    void SetDecayTime(G4int detectorID, G4double decayTime);
    void SetTransportDelay(G4int detectorID, G4double transportDelay);
    void SetTimeJitter(G4int detectorID, G4double timeJitter);
    void SetResolutionScale(G4int detectorID, G4double resolutionScale);
    void SetPhotosensorType(G4int detectorID, ScintCubePhotosensorType photosensorType);
    void SetPMTQuantumEfficiency(G4int detectorID, G4double quantumEfficiency);
    void SetPMTDynodeCollectionEfficiency(G4int detectorID, G4double dynodeCollectionEfficiency);
    void SetPMTTransitTime(G4int detectorID, G4double transitTime);
    void SetPMTTransitTimeSpread(G4int detectorID, G4double transitTimeSpread);
    void SetSiPMPDE(G4int detectorID, G4double photoDetectionEfficiency);
    void SetSiPMMicrocellCount(G4int detectorID, G4double microcellCount);
    void SetSiPMExcessNoiseFactor(G4int detectorID, G4double excessNoiseFactor);
    void SetSiPMAvalancheTime(G4int detectorID, G4double avalancheTime);
    void SetSiPMAvalancheTimeSpread(G4int detectorID, G4double avalancheTimeSpread);
    void SetDoseCalibrationFactor(G4int detectorID, G4double doseCalibrationFactorInGyPerPhotoelectron);
    void SetDoseCalibrationFactorError(G4int detectorID, G4double doseCalibrationFactorErrorInGyPerPhotoelectron);

    const ScintCubeDetectorConfig& GetDetectorConfig(G4int detectorID) const;
    std::map<G4int, ScintCubeReadoutParameters> GetReadoutParametersByDetector() const;
    ScintCubeCalibrationParameters GetCalibrationParameters(G4int detectorID) const;

    void AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector);

protected:
    G4bool RequiresPlacementRebuild(const G4int& copyNo) const override;
    void OnAfterPlacementRemoval(const G4int& copyNo) override;

private:
    struct PlacementOwnedResources {
        std::vector<G4VPhysicalVolume*> nestedPhysicalVolumes;
        std::vector<G4LogicalVolume*> logicalVolumes;
        std::vector<G4VSolid*> solids;
        std::vector<G4VisAttributes*> visAttributes;
        std::vector<G4LogicalVolume*> sensitiveLogicalVolumes;
    };

    struct StandardPlacementParts {
        G4LogicalVolume* sensitive = nullptr;
        G4LogicalVolume* envelope = nullptr;
        PlacementOwnedResources ownedResources;
    };

    struct SplitPlacementParts {
        G4LogicalVolume* waterSensitive = nullptr;
        G4LogicalVolume* airSensitive = nullptr;
        G4LogicalVolume* waterEnvelope = nullptr;
        G4LogicalVolume* airEnvelope = nullptr;
        PlacementOwnedResources ownedResources;
    };

    ScintCubeDetectorConfig& EnsureDetectorConfig(G4int detectorID);
    void EnsureMaterials(const ScintCubeDetectorConfig& config);
    StandardPlacementParts BuildStandardPlacementVolumes(const ScintCubeDetectorConfig& config,
                                                         const G4String& suffix);
    SplitPlacementParts BuildSplitPlacementVolumes(const ScintCubeDetectorConfig& config,
                                                   const G4String& suffix,
                                                   G4double waterOuterThickness,
                                                   G4double airOuterThickness,
                                                   G4double waterSensitiveThickness,
                                                   G4double airSensitiveThickness);
    void PlaceSplitPlacement(SplitPlacementParts& parts,
                             const ScintCubeDetectorConfig& config,
                             G4LogicalVolume* waterMother,
                             G4LogicalVolume* worldMother,
                             const G4String& suffix,
                             const G4ThreeVector& centerRelativeToWater,
                             const G4ThreeVector& waterWorldTranslation,
                             G4double interfaceRelativeZ,
                             G4int copyNo);
    G4double GetOuterHalfSizeZ(const ScintCubeDetectorConfig& config) const;
    G4bool HasNonIdentityRotation(const G4int& copyNo) const;
    void ReleasePlacementResources(const G4int& copyNo);
    void ValidateSplitPlacementSupport(const ScintCubeDetectorConfig& config,
                                       G4LogicalVolume* motherVolume,
                                       const G4ThreeVector& centerRelativeToWater,
                                       G4double outerHalfSize,
                                       G4int copyNo) const;
    G4VPhysicalVolume* GetWaterPhysicalVolume() const;
    G4VSensitiveDetector* GetCurrentSensitiveDetector() const { return fSensitiveDetector; }

    ScintCubeDetectorConfig fDefaultConfig;
    std::map<G4int, ScintCubeDetectorConfig> fDetectorConfigs;
    std::map<G4int, PlacementOwnedResources> fPlacementResources;
    DetectorScintCubeMessenger* fDetectorScintCubeMessenger;
    G4VSensitiveDetector* fSensitiveDetector;
};

#endif // MDSIM1_DETECTOR_SCINT_CUBE_HH
