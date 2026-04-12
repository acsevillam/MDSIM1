#include "geometry/detectors/model11/geometry/DetectorModel11.hh"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "G4Exception.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "geometry/detectors/model11/messenger/DetectorModel11Messenger.hh"

namespace {

G4String SanitizeIdentifier(const G4String& value) {
    G4String sanitized = value;
    std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), [](unsigned char ch) {
        return std::isalnum(ch) ? static_cast<char>(ch) : '_';
    });

    if (sanitized.empty()) {
        sanitized = "part";
    }
    return sanitized;
}

G4String TrimVolumeName(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

G4String JoinVolumeNames(const std::set<G4String>& names) {
    std::ostringstream stream;
    G4bool first = true;
    for (const auto& name : names) {
        if (!first) {
            stream << ", ";
        }
        stream << name;
        first = false;
    }
    return stream.str();
}

} // namespace

DetectorModel11::DetectorModel11()
    : fDetectorModel11Messenger(nullptr),
      fSensitiveDetector(nullptr) {
    geometryName = "DetectorModel11";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorModel11Messenger = new DetectorModel11Messenger(this);
}

DetectorModel11::~DetectorModel11() {
    fImportedGDMLCache.Clear();
    delete fDetectorModel11Messenger;
    fDetectorModel11Messenger = nullptr;
}

void DetectorModel11::DefineMaterials() {}

void DetectorModel11::DefineVolumes() {
    fAreVolumensDefined = true;
}

Model11DetectorConfig& DetectorModel11::EnsureDetectorConfig(G4int detectorID) {
    const auto it = fDetectorConfigs.find(detectorID);
    if (it != fDetectorConfigs.end()) {
        return it->second;
    }

    auto [insertedIt, inserted] = fDetectorConfigs.emplace(detectorID, fDefaultConfig);
    (void)inserted;
    return insertedIt->second;
}

void DetectorModel11::SetSplitAtInterface(G4int detectorID, G4bool splitAtInterface) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.splitAtInterface = splitAtInterface;
    fDefaultConfig = config;
}

void DetectorModel11::SetScintillationYield(G4int detectorID, G4double scintillationYieldPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.scintillationYield = scintillationYieldPerMeV / MeV;
    fDefaultConfig = config;
}

void DetectorModel11::SetBirksConstant(G4int detectorID, G4double birksConstantInMmPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.birksConstant = birksConstantInMmPerMeV * mm / MeV;
    fDefaultConfig = config;
}

void DetectorModel11::SetLightCollectionEfficiency(G4int detectorID, G4double lightCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.lightCollectionEfficiency = lightCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetDecayTime(G4int detectorID, G4double decayTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.decayTime = decayTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetTransportDelay(G4int detectorID, G4double transportDelay) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.transportDelay = transportDelay;
    fDefaultConfig = config;
}

void DetectorModel11::SetTimeJitter(G4int detectorID, G4double timeJitter) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.timeJitter = timeJitter;
    fDefaultConfig = config;
}

void DetectorModel11::SetResolutionScale(G4int detectorID, G4double resolutionScale) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.resolutionScale = resolutionScale;
    fDefaultConfig = config;
}

void DetectorModel11::SetPhotosensorType(G4int detectorID, Model11PhotosensorType photosensorType) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.photosensorType = photosensorType;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTQuantumEfficiency(G4int detectorID, G4double quantumEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.quantumEfficiency = quantumEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTDynodeCollectionEfficiency(G4int detectorID,
                                                       G4double dynodeCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.dynodeCollectionEfficiency = dynodeCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTTransitTime(G4int detectorID, G4double transitTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTime = transitTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTTransitTimeSpread(G4int detectorID, G4double transitTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTimeSpread = transitTimeSpread;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMPDE(G4int detectorID, G4double photoDetectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.photoDetectionEfficiency = photoDetectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMMicrocellCount(G4int detectorID, G4double microcellCount) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.microcellCount = microcellCount;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMExcessNoiseFactor(G4int detectorID, G4double excessNoiseFactor) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.excessNoiseFactor = excessNoiseFactor;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMAvalancheTime(G4int detectorID, G4double avalancheTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTime = avalancheTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMAvalancheTimeSpread(G4int detectorID, G4double avalancheTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTimeSpread = avalancheTimeSpread;
    fDefaultConfig = config;
}

void DetectorModel11::SetDoseCalibrationFactor(G4int detectorID,
                                               G4double doseCalibrationFactorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactor = doseCalibrationFactorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

void DetectorModel11::SetDoseCalibrationFactorError(
    G4int detectorID,
    G4double doseCalibrationFactorErrorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactorError =
        doseCalibrationFactorErrorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

void DetectorModel11::SetImportedGeometryGDMLPath(G4int detectorID, const G4String& gdmlPath) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryGDMLPath = gdmlPath;
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootName = TrimVolumeName(rootName);
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::AddSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName) {
    const auto normalizedName = TrimVolumeName(logicalVolumeName);
    if (normalizedName.empty()) {
        G4Exception("DetectorModel11::AddSensitiveVolume",
                    "DetectorModel11InvalidSensitiveVolume",
                    FatalException,
                    "Sensitive volume name for model11 cannot be empty.");
        return;
    }

    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.insert(normalizedName);
    fDefaultConfig = config;
}

void DetectorModel11::RemoveSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.erase(TrimVolumeName(logicalVolumeName));
    fDefaultConfig = config;
}

void DetectorModel11::ClearSensitiveVolumes(G4int detectorID) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.clear();
    fDefaultConfig = config;
}

const Model11DetectorConfig& DetectorModel11::GetDetectorConfig(G4int detectorID) const {
    const auto it = fDetectorConfigs.find(detectorID);
    return (it != fDetectorConfigs.end()) ? it->second : fDefaultConfig;
}

std::map<G4int, Model11ReadoutParameters> DetectorModel11::GetReadoutParametersByDetector() const {
    std::map<G4int, Model11ReadoutParameters> parametersByDetector;
    for (const auto copyNo : GetPlacementCopyNumbers()) {
        parametersByDetector.emplace(copyNo, GetDetectorConfig(copyNo).readoutParameters);
    }
    return parametersByDetector;
}

Model11CalibrationParameters DetectorModel11::GetCalibrationParameters(G4int detectorID) const {
    return GetDetectorConfig(detectorID).calibrationParameters;
}

G4bool DetectorModel11::HasImportedGeometry(G4int detectorID) const {
    return ShouldBuildImportedGeometry(GetDetectorConfig(detectorID));
}

std::size_t DetectorModel11::GetImportedGeometryPartCount(G4int detectorID) const {
    const auto& config = GetDetectorConfig(detectorID);
    if (!ShouldBuildImportedGeometry(config)) {
        return 0;
    }

    return LoadImportedGDMLAssembly(config)->GetPartCount();
}

G4String DetectorModel11::GetImportedGeometryRootVolumeName(G4int detectorID) const {
    const auto& config = GetDetectorConfig(detectorID);
    if (!ShouldBuildImportedGeometry(config)) {
        return "none";
    }

    return LoadImportedGDMLAssembly(config)->GetRootVolumeName();
}

std::vector<G4String> DetectorModel11::GetSensitiveVolumeNames(G4int detectorID) const {
    const auto& sensitiveNames = GetDetectorConfig(detectorID).sensitiveVolumeNames;
    return std::vector<G4String>(sensitiveNames.begin(), sensitiveNames.end());
}

G4bool DetectorModel11::ShouldBuildImportedGeometry(const Model11DetectorConfig& config) const {
    return !config.importedGeometryGDMLPath.empty();
}

G4bool DetectorModel11::ShouldActivateSensitiveImportedVolumes(
    const Model11DetectorConfig& config) const {
    return !config.sensitiveVolumeNames.empty();
}

std::shared_ptr<const MD1::GDMLImportedAssembly> DetectorModel11::LoadImportedGDMLAssembly(
    const Model11DetectorConfig& config) const {
    if (!ShouldBuildImportedGeometry(config)) {
        return nullptr;
    }

    return fImportedGDMLCache.Load(config.importedGeometryGDMLPath, config.importedGeometryRootName);
}

G4bool DetectorModel11::IsSensitiveVolumeSelected(const Model11DetectorConfig& config,
                                                  const G4String& logicalVolumeName,
                                                  const G4String& physicalVolumeName) const {
    return config.sensitiveVolumeNames.find(logicalVolumeName) != config.sensitiveVolumeNames.end() ||
           config.sensitiveVolumeNames.find(physicalVolumeName) != config.sensitiveVolumeNames.end();
}

std::set<G4String> DetectorModel11::CollectReferencedImportedGDMLKeys() const {
    std::set<G4String> referencedKeys;

    for (const auto& [detectorID, config] : fDetectorConfigs) {
        (void)detectorID;
        if (ShouldBuildImportedGeometry(config)) {
            referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(config.importedGeometryGDMLPath,
                                                                        config.importedGeometryRootName));
        }
    }

    if (ShouldBuildImportedGeometry(fDefaultConfig)) {
        for (const auto& [copyNo, motherName] : detMotherVolumeNames) {
            (void)motherName;
            if (fDetectorConfigs.find(copyNo) == fDetectorConfigs.end()) {
                referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(
                    fDefaultConfig.importedGeometryGDMLPath, fDefaultConfig.importedGeometryRootName));
                break;
            }
        }
    }

    for (const auto& [copyNo, cacheKey] : fPlacementImportedGDMLKeys) {
        (void)copyNo;
        if (!cacheKey.empty()) {
            referencedKeys.insert(cacheKey);
        }
    }

    return referencedKeys;
}

void DetectorModel11::PruneUnusedImportedGDMLAssemblies() {
    fImportedGDMLCache.RetainOnly(CollectReferencedImportedGDMLKeys());
}

G4LogicalVolume* DetectorModel11::CloneImportedSubtree(
    const G4VPhysicalVolume* sourcePhysicalVolume,
    const Model11DetectorConfig& config,
    G4int copyNo,
    std::size_t& cloneSequence,
    PlacementOwnedResources& resources) {
    if (sourcePhysicalVolume == nullptr) {
        G4Exception("DetectorModel11::CloneImportedSubtree",
                    "DetectorModel11ImportedGeometryInvalidRoot",
                    FatalException,
                    "model11 cannot clone a null GDML physical volume.");
        return nullptr;
    }

    auto* sourceLogicalVolume = sourcePhysicalVolume->GetLogicalVolume();
    if (sourceLogicalVolume == nullptr) {
        G4Exception("DetectorModel11::CloneImportedSubtree",
                    "DetectorModel11ImportedGeometryInvalidLogical",
                    FatalException,
                    ("GDML physical volume " + sourcePhysicalVolume->GetName() +
                     " does not have an associated logical volume.")
                        .c_str());
        return nullptr;
    }

    const auto sourceLogicalName = sourceLogicalVolume->GetName();
    const auto sourcePhysicalName = sourcePhysicalVolume->GetName();
    const auto nodeId =
        SanitizeIdentifier(sourcePhysicalName.empty() ? sourceLogicalName : sourcePhysicalName);
    const auto cloneIndex = cloneSequence++;
    const auto logicalVolumeName =
        (IsSensitiveVolumeSelected(config, sourceLogicalName, sourcePhysicalName) ? "Model11SensitiveLV_"
                                                                                  : "Model11PassiveLV_") +
        nodeId + "_" + std::to_string(copyNo) + "_" + std::to_string(cloneIndex);

    auto* clonedLogicalVolume = new G4LogicalVolume(sourceLogicalVolume->GetSolid(),
                                                    sourceLogicalVolume->GetMaterial(),
                                                    logicalVolumeName);
    resources.logicalVolumes.push_back(clonedLogicalVolume);

    if (const auto* sourceVisAttributes = sourceLogicalVolume->GetVisAttributes();
        sourceVisAttributes != nullptr) {
        auto* clonedVisAttributes = new G4VisAttributes(*sourceVisAttributes);
        resources.visAttributes.push_back(clonedVisAttributes);
        clonedLogicalVolume->SetVisAttributes(clonedVisAttributes);
    }

    if (IsSensitiveVolumeSelected(config, sourceLogicalName, sourcePhysicalName)) {
        resources.sensitiveLogicalVolumes.push_back(clonedLogicalVolume);
        if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
            clonedLogicalVolume->SetSensitiveDetector(sensitiveDetector);
        }
    }

    for (G4int daughterIndex = 0; daughterIndex < sourceLogicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* sourceDaughterPhysical = sourceLogicalVolume->GetDaughter(daughterIndex);
        auto* clonedDaughterLogical =
            CloneImportedSubtree(sourceDaughterPhysical, config, copyNo, cloneSequence, resources);
        const auto daughterId = SanitizeIdentifier(sourceDaughterPhysical->GetName().empty()
                                                       ? sourceDaughterPhysical->GetLogicalVolume()->GetName()
                                                       : sourceDaughterPhysical->GetName());
        new G4PVPlacement(G4Transform3D(sourceDaughterPhysical->GetObjectRotationValue(),
                                        sourceDaughterPhysical->GetObjectTranslation()),
                          clonedDaughterLogical,
                          "ImportedModel11SubPhys_" + daughterId + "_" + std::to_string(copyNo) + "_" +
                              std::to_string(cloneSequence),
                          clonedLogicalVolume,
                          false,
                          copyNo,
                          true);
    }

    return clonedLogicalVolume;
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume,
                                  const G4ThreeVector& position,
                                  G4RotationMatrix* rotation,
                                  G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume,
                                  G4Transform3D* transformation,
                                  G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    const auto& config = GetDetectorConfig(copyNo);
    fPlacementImportedGDMLKeys.erase(copyNo);

    if (config.importedGeometryGDMLPath.empty()) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11ImportedGeometryGDMLMissing",
                    FatalException,
                    "model11 requires a GDML path because the full geometry is defined by the imported model.");
        return;
    }
    if (config.splitAtInterface) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11ImportedGeometrySplitUnsupported",
                    FatalException,
                    "split at interface is not supported for GDML-driven model11 geometry.");
        return;
    }

    const auto assembly = LoadImportedGDMLAssembly(config);
    if (assembly == nullptr || assembly->GetParts().empty()) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11ImportedGeometryEmpty",
                    FatalException,
                    "Imported geometry GDML did not provide any reusable parts for model11.");
        return;
    }

    std::set<G4String> availableVolumeNames;
    for (const auto& availableName : assembly->GetAvailableVolumeNames()) {
        availableVolumeNames.insert(availableName);
    }

    if (ShouldActivateSensitiveImportedVolumes(config)) {
        std::set<G4String> unresolvedVolumeNames;
        for (const auto& selectedName : config.sensitiveVolumeNames) {
            if (availableVolumeNames.find(selectedName) == availableVolumeNames.end()) {
                unresolvedVolumeNames.insert(selectedName);
            }
        }

        if (!unresolvedVolumeNames.empty()) {
            G4Exception("DetectorModel11::AddGeometry",
                        "DetectorModel11SensitiveVolumeNotFound",
                        FatalException,
                        ("The following model11 sensitive volumes were not found in GDML " +
                         assembly->GetSourcePath() + ": " + JoinVolumeNames(unresolvedVolumeNames) +
                         ". Available logical/physical names: " + JoinVolumeNames(availableVolumeNames))
                            .c_str());
            return;
        }
    }

    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    const G4ThreeVector centerRelativeToMother = finalTransform.getTranslation();
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    StoreRotation(copyNo, rotation);

    PlacementOwnedResources resources;
    std::size_t cloneSequence = 0;
    G4VPhysicalVolume* primaryPlacement = nullptr;
    std::size_t partIndex = 0;
    for (const auto& part : assembly->GetParts()) {
        auto* clonedRootLogical =
            CloneImportedSubtree(part.physicalVolume, config, copyNo, cloneSequence, resources);
        const auto rootId = SanitizeIdentifier(
            part.name.empty() ? assembly->GetRootVolumeName() : part.name);
        auto* placement = new G4PVPlacement(finalTransform * G4Transform3D(part.rotation, part.translation),
                                            clonedRootLogical,
                                            "ImportedModel11Phys_" + rootId + "_" + std::to_string(copyNo) + "_" +
                                                std::to_string(partIndex++),
                                            motherVolume,
                                            false,
                                            copyNo,
                                            true);

        if (placement == nullptr) {
            G4Exception("DetectorModel11::AddGeometry",
                        "DetectorModel11PlacementCreationFailed",
                        FatalException,
                        "Model11 failed to create any physical placement for the requested geometry.");
            return;
        }

        if (primaryPlacement == nullptr) {
            primaryPlacement = placement;
            SetPrimaryFrameVolume(copyNo, placement);
        } else {
            AddAuxiliaryFrameVolume(copyNo, placement);
        }
    }

    if (primaryPlacement == nullptr) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11PlacementCreationFailed",
                    FatalException,
                    "Model11 failed to create any physical placement for the requested geometry.");
        return;
    }

    fPlacementImportedGDMLKeys[copyNo] =
        MD1::GDMLAssemblyCache::BuildCacheKey(config.importedGeometryGDMLPath,
                                              config.importedGeometryRootName);
    fPlacementResources[copyNo] = std::move(resources);
    fAreVolumensAssembled = true;
    detPosition[copyNo] = centerRelativeToMother;
}

void DetectorModel11::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
    if (sensitiveDetector == nullptr) {
        return;
    }

    fSensitiveDetector = sensitiveDetector;
    for (auto& [copyNo, resources] : fPlacementResources) {
        (void)copyNo;
        for (auto* logicalVolume : resources.sensitiveLogicalVolumes) {
            if (logicalVolume != nullptr) {
                logicalVolume->SetSensitiveDetector(sensitiveDetector);
            }
        }
    }
}

void DetectorModel11::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleasePlacementResources(copyNo);
    fPlacementImportedGDMLKeys.erase(copyNo);
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::ReleasePlacementResources(const G4int& copyNo) {
    const auto resourcesIt = fPlacementResources.find(copyNo);
    if (resourcesIt == fPlacementResources.end()) {
        return;
    }

    auto& resources = resourcesIt->second;
    for (auto* logicalVolume : resources.logicalVolumes) {
        delete logicalVolume;
    }
    for (auto* visAttributes : resources.visAttributes) {
        delete visAttributes;
    }

    fPlacementResources.erase(resourcesIt);
}
