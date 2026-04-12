#ifndef MDSIM1_DETECTOR_MODEL11_HH
#define MDSIM1_DETECTOR_MODEL11_HH

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

#include "geometry/base/GenericGeometry.hh"
#include "geometry/detectors/model11/calibration/Model11DoseCalibrator.hh"
#include "geometry/detectors/model11/readout/Model11ReadoutParameters.hh"
#include "geometry/gdml/GDMLAssemblyCache.hh"

class DetectorModel11Messenger;
class G4VisAttributes;
class G4VPhysicalVolume;
class G4VSolid;

struct Model11DetectorConfig {
    G4bool splitAtInterface = false;
    Model11ReadoutParameters readoutParameters;
    Model11CalibrationParameters calibrationParameters;
    G4String importedGeometryGDMLPath;
    MD1::GDMLRootSelector importedGeometryRootSelector;
    MD1::GDMLReadOptions importedGeometryReadOptions;
    std::set<G4String> sensitiveVolumeNames;
};

class DetectorModel11 : public GenericGeometry {
public:
    DetectorModel11();
    ~DetectorModel11() override;

    void DefineMaterials() override;
    void DefineVolumes() override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume,
                     const G4ThreeVector& position,
                     G4RotationMatrix* rotation,
                     G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) override;

    void SetSplitAtInterface(G4int detectorID, G4bool splitAtInterface);
    void SetScintillationYield(G4int detectorID, G4double scintillationYieldPerMeV);
    void SetBirksConstant(G4int detectorID, G4double birksConstantInMmPerMeV);
    void SetLightCollectionEfficiency(G4int detectorID, G4double lightCollectionEfficiency);
    void SetDecayTime(G4int detectorID, G4double decayTime);
    void SetTransportDelay(G4int detectorID, G4double transportDelay);
    void SetTimeJitter(G4int detectorID, G4double timeJitter);
    void SetResolutionScale(G4int detectorID, G4double resolutionScale);
    void SetPhotosensorType(G4int detectorID, Model11PhotosensorType photosensorType);
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
    void SetImportedGeometryGDMLPath(G4int detectorID, const G4String& gdmlPath);
    void SetImportedGeometryRootLogicalName(G4int detectorID, const G4String& rootName);
    void SetImportedGeometryRootPhysicalName(G4int detectorID, const G4String& rootName);
    void SetImportedGeometryRootAssemblyName(G4int detectorID, const G4String& rootName);
    void SetImportedGeometryValidate(G4int detectorID, G4bool validate);
    void SetImportedGeometrySchema(G4int detectorID, const G4String& schemaPath);
    void AddSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName);
    void RemoveSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName);
    void ClearSensitiveVolumes(G4int detectorID);

    const Model11DetectorConfig& GetDetectorConfig(G4int detectorID) const;
    std::map<G4int, Model11ReadoutParameters> GetReadoutParametersByDetector() const;
    Model11CalibrationParameters GetCalibrationParameters(G4int detectorID) const;
    G4bool HasImportedGeometry(G4int detectorID) const;
    std::size_t GetImportedGeometryPartCount(G4int detectorID) const;
    G4String GetImportedGeometryRootVolumeName(G4int detectorID) const;
    std::vector<G4String> GetSensitiveVolumeNames(G4int detectorID) const;

    void AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector);

protected:
    void OnAfterPlacementRemoval(const G4int& copyNo) override;

private:
    struct PlacementOwnedResources {
        std::vector<G4LogicalVolume*> logicalVolumes;
        std::vector<G4VisAttributes*> visAttributes;
        std::vector<G4LogicalVolume*> sensitiveLogicalVolumes;
    };

    Model11DetectorConfig& EnsureDetectorConfig(G4int detectorID);
    G4bool ShouldBuildImportedGeometry(const Model11DetectorConfig& config) const;
    G4bool ShouldActivateSensitiveImportedVolumes(const std::set<G4String>& sensitiveVolumeNames) const;
    std::shared_ptr<const MD1::GDMLImportedAssembly> LoadImportedGDMLAssembly(
        const Model11DetectorConfig& config) const;
    std::set<G4String> ResolveSensitiveVolumeNames(const Model11DetectorConfig& config) const;
    G4LogicalVolume* CloneImportedSubtree(G4LogicalVolume* sourceLogicalVolume,
                                          const G4String& sourcePhysicalName,
                                          const MD1::GDMLImportedAssembly& importedAssembly,
                                          const std::set<G4String>& sensitiveVolumeNames,
                                          G4int copyNo,
                                          std::size_t& cloneSequence,
                                          PlacementOwnedResources& resources);
    G4bool IsSensitiveVolumeSelected(const std::set<G4String>& sensitiveVolumeNames,
                                     const G4String& logicalVolumeName,
                                     const G4String& physicalVolumeName) const;
    std::set<G4String> CollectReferencedImportedGDMLKeys() const;
    void PruneUnusedImportedGDMLAssemblies();
    void ReleasePlacementResources(const G4int& copyNo);
    G4VSensitiveDetector* GetCurrentSensitiveDetector() const { return fSensitiveDetector; }

    Model11DetectorConfig fDefaultConfig;
    std::map<G4int, Model11DetectorConfig> fDetectorConfigs;
    std::map<G4int, PlacementOwnedResources> fPlacementResources;
    std::map<G4int, G4String> fPlacementImportedGDMLKeys;
    mutable MD1::GDMLAssemblyCache fImportedGDMLCache;
    DetectorModel11Messenger* fDetectorModel11Messenger;
    G4VSensitiveDetector* fSensitiveDetector;
};

#endif // MDSIM1_DETECTOR_MODEL11_HH
