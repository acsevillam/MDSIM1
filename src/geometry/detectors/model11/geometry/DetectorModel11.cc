#include "geometry/detectors/model11/geometry/DetectorModel11.hh"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

#include "G4Colour.hh"
#include "G4Exception.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "geometry/detectors/model11/messenger/DetectorModel11Messenger.hh"
#include "geometry/gdml/GeometryAuxiliaryRegistry.hh"

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

G4GDMLAuxListType FlattenAuxiliaryList(const G4GDMLAuxListType& auxiliaries) {
    G4GDMLAuxListType flatCopy;
    flatCopy.reserve(auxiliaries.size());
    for (const auto& aux : auxiliaries) {
        flatCopy.push_back(G4GDMLAuxStructType{aux.type, aux.value, aux.unit, nullptr});
    }
    return flatCopy;
}

G4bool TryParseHexColorComponent(const std::string& hexPair, G4double& component) {
    if (hexPair.size() != 2) {
        return false;
    }

    char* end = nullptr;
    const long parsed = std::strtol(hexPair.c_str(), &end, 16);
    if (end == nullptr || *end != '\0' || parsed < 0 || parsed > 255) {
        return false;
    }

    component = static_cast<G4double>(parsed) / 255.0;
    return true;
}

G4bool TryCreateVisAttributesFromColorAux(const G4GDMLAuxListType& auxiliaries,
                                          G4VisAttributes*& visAttributes) {
    const auto colorIt = std::find_if(auxiliaries.begin(), auxiliaries.end(), [](const auto& aux) {
        return aux.type == "Color";
    });
    if (colorIt == auxiliaries.end()) {
        return false;
    }

    G4double red = 0.0;
    G4double green = 0.0;
    G4double blue = 0.0;
    G4double alpha = 1.0;
    const auto colorValue = colorIt->value;

    if (colorValue.size() == 7 || colorValue.size() == 9) {
        if (colorValue[0] != '#') {
            return false;
        }

        const std::string rgb = colorValue.substr(1);
        if (!TryParseHexColorComponent(rgb.substr(0, 2), red) ||
            !TryParseHexColorComponent(rgb.substr(2, 2), green) ||
            !TryParseHexColorComponent(rgb.substr(4, 2), blue)) {
            return false;
        }
        if (rgb.size() == 8 && !TryParseHexColorComponent(rgb.substr(6, 2), alpha)) {
            return false;
        }
    } else {
        G4String normalizedColor = colorValue;
        std::transform(normalizedColor.begin(),
                       normalizedColor.end(),
                       normalizedColor.begin(),
                       [](unsigned char ch) { return std::tolower(ch); });

        if (normalizedColor == "red") {
            red = 1.0;
        } else if (normalizedColor == "green") {
            green = 1.0;
        } else if (normalizedColor == "blue") {
            blue = 1.0;
        } else if (normalizedColor == "yellow") {
            red = 1.0;
            green = 1.0;
        } else if (normalizedColor == "cyan") {
            green = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "magenta") {
            red = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "white") {
            red = 1.0;
            green = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "black") {
            red = 0.0;
            green = 0.0;
            blue = 0.0;
        } else if (normalizedColor == "gray" || normalizedColor == "grey") {
            red = 0.5;
            green = 0.5;
            blue = 0.5;
        } else {
            return false;
        }
    }

    visAttributes = new G4VisAttributes(G4Colour(red, green, blue, alpha));
    return true;
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

void DetectorModel11::SetImportedGeometryRootLogicalName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Logical, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootPhysicalName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Physical, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootAssemblyName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Assembly, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryValidate(G4int detectorID, G4bool validate) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryReadOptions.validate = validate;
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometrySchema(G4int detectorID, const G4String& schemaPath) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryReadOptions.schemaPath = TrimVolumeName(schemaPath);
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
    const auto sensitiveNames = ResolveSensitiveVolumeNames(GetDetectorConfig(detectorID));
    return std::vector<G4String>(sensitiveNames.begin(), sensitiveNames.end());
}

G4bool DetectorModel11::ShouldBuildImportedGeometry(const Model11DetectorConfig& config) const {
    return !config.importedGeometryGDMLPath.empty();
}

G4bool DetectorModel11::ShouldActivateSensitiveImportedVolumes(
    const std::set<G4String>& sensitiveVolumeNames) const {
    return !sensitiveVolumeNames.empty();
}

std::shared_ptr<const MD1::GDMLImportedAssembly> DetectorModel11::LoadImportedGDMLAssembly(
    const Model11DetectorConfig& config) const {
    if (!ShouldBuildImportedGeometry(config)) {
        return nullptr;
    }

    return fImportedGDMLCache.Load(config.importedGeometryGDMLPath,
                                   config.importedGeometryRootSelector,
                                   config.importedGeometryReadOptions);
}

std::set<G4String> DetectorModel11::ResolveSensitiveVolumeNames(
    const Model11DetectorConfig& config) const {
    if (!config.sensitiveVolumeNames.empty()) {
        return config.sensitiveVolumeNames;
    }

    if (!ShouldBuildImportedGeometry(config)) {
        return {};
    }

    const auto assembly = LoadImportedGDMLAssembly(config);
    if (assembly == nullptr) {
        return {};
    }

    const auto auxSensitiveNames = assembly->GetAuxSensitiveVolumeNames();
    return std::set<G4String>(auxSensitiveNames.begin(), auxSensitiveNames.end());
}

G4bool DetectorModel11::IsSensitiveVolumeSelected(const std::set<G4String>& sensitiveVolumeNames,
                                                  const G4String& logicalVolumeName,
                                                  const G4String& physicalVolumeName) const {
    return sensitiveVolumeNames.find(logicalVolumeName) != sensitiveVolumeNames.end() ||
           sensitiveVolumeNames.find(physicalVolumeName) != sensitiveVolumeNames.end();
}

std::set<G4String> DetectorModel11::CollectReferencedImportedGDMLKeys() const {
    std::set<G4String> referencedKeys;

    for (const auto& [detectorID, config] : fDetectorConfigs) {
        (void)detectorID;
        if (ShouldBuildImportedGeometry(config)) {
            referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(
                config.importedGeometryGDMLPath,
                config.importedGeometryRootSelector,
                config.importedGeometryReadOptions));
        }
    }

    if (ShouldBuildImportedGeometry(fDefaultConfig)) {
        for (const auto& [copyNo, motherName] : detMotherVolumeNames) {
            (void)motherName;
            if (fDetectorConfigs.find(copyNo) == fDetectorConfigs.end()) {
                referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(
                    fDefaultConfig.importedGeometryGDMLPath,
                    fDefaultConfig.importedGeometryRootSelector,
                    fDefaultConfig.importedGeometryReadOptions));
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
    G4LogicalVolume* sourceLogicalVolume,
    const G4String& sourcePhysicalName,
    const MD1::GDMLImportedAssembly& importedAssembly,
    const std::set<G4String>& sensitiveVolumeNames,
    G4int copyNo,
    std::size_t& cloneSequence,
    PlacementOwnedResources& resources) {
    if (sourceLogicalVolume == nullptr) {
        G4Exception("DetectorModel11::CloneImportedSubtree",
                    "DetectorModel11ImportedGeometryInvalidRoot",
                    FatalException,
                    "model11 cannot clone a null GDML logical volume.");
        return nullptr;
    }

    const auto sourceLogicalName = sourceLogicalVolume->GetName();
    const auto nodeId =
        SanitizeIdentifier(sourcePhysicalName.empty() ? sourceLogicalName : sourcePhysicalName);
    const auto cloneIndex = cloneSequence++;
    const auto isSensitive =
        IsSensitiveVolumeSelected(sensitiveVolumeNames, sourceLogicalName, sourcePhysicalName);
    const auto logicalVolumeName =
        (isSensitive ? "Model11SensitiveLV_" : "Model11PassiveLV_") + nodeId + "_" +
        std::to_string(copyNo) + "_" + std::to_string(cloneIndex);

    auto* clonedLogicalVolume = new G4LogicalVolume(sourceLogicalVolume->GetSolid(),
                                                    sourceLogicalVolume->GetMaterial(),
                                                    logicalVolumeName);
    resources.logicalVolumes.push_back(clonedLogicalVolume);

    if (const auto* sourceVisAttributes = sourceLogicalVolume->GetVisAttributes();
        sourceVisAttributes != nullptr) {
        auto* clonedVisAttributes = new G4VisAttributes(*sourceVisAttributes);
        resources.visAttributes.push_back(clonedVisAttributes);
        clonedLogicalVolume->SetVisAttributes(clonedVisAttributes);
    } else if (const auto* sourceAuxiliaries = importedAssembly.GetAuxiliaryInfo(sourceLogicalVolume);
               sourceAuxiliaries != nullptr) {
        G4VisAttributes* clonedVisAttributes = nullptr;
        if (TryCreateVisAttributesFromColorAux(*sourceAuxiliaries, clonedVisAttributes)) {
            resources.visAttributes.push_back(clonedVisAttributes);
            clonedLogicalVolume->SetVisAttributes(clonedVisAttributes);
        }
    }

    if (const auto* sourceAuxiliaries = importedAssembly.GetAuxiliaryInfo(sourceLogicalVolume);
        sourceAuxiliaries != nullptr) {
        MD1::GeometryAuxiliaryRegistry::GetInstance()->Register(
            clonedLogicalVolume, FlattenAuxiliaryList(*sourceAuxiliaries));
    }

    if (isSensitive) {
        resources.sensitiveLogicalVolumes.push_back(clonedLogicalVolume);
        if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
            clonedLogicalVolume->SetSensitiveDetector(sensitiveDetector);
        }
    }

    for (G4int daughterIndex = 0; daughterIndex < sourceLogicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* sourceDaughterPhysical = sourceLogicalVolume->GetDaughter(daughterIndex);
        auto* clonedDaughterLogical =
            CloneImportedSubtree(sourceDaughterPhysical->GetLogicalVolume(),
                                 sourceDaughterPhysical->GetName(),
                                 importedAssembly,
                                 sensitiveVolumeNames,
                                 copyNo,
                                 cloneSequence,
                                 resources);
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

    const auto effectiveSensitiveVolumeNames = ResolveSensitiveVolumeNames(config);
    if (ShouldActivateSensitiveImportedVolumes(effectiveSensitiveVolumeNames)) {
        std::set<G4String> unresolvedVolumeNames;
        for (const auto& selectedName : effectiveSensitiveVolumeNames) {
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
        auto* clonedRootLogical = CloneImportedSubtree(part.logicalVolume,
                                                       (part.physicalVolume != nullptr)
                                                           ? part.physicalVolume->GetName()
                                                           : part.name,
                                                       *assembly,
                                                       effectiveSensitiveVolumeNames,
                                                       copyNo,
                                                       cloneSequence,
                                                       resources);
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
                                              config.importedGeometryRootSelector,
                                              config.importedGeometryReadOptions);
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
        MD1::GeometryAuxiliaryRegistry::GetInstance()->Unregister(logicalVolume);
        delete logicalVolume;
    }
    for (auto* visAttributes : resources.visAttributes) {
        delete visAttributes;
    }

    fPlacementResources.erase(resourcesIt);
}
