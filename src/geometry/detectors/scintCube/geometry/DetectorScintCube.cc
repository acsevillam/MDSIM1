#include "geometry/detectors/scintCube/geometry/DetectorScintCube.hh"

#include <algorithm>

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "geometry/detectors/scintCube/messenger/DetectorScintCubeMessenger.hh"

DetectorScintCube::DetectorScintCube()
    : fDetectorScintCubeMessenger(nullptr),
      fSensitiveDetector(nullptr) {
    geometryName = "DetectorScintCube";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorScintCubeMessenger = new DetectorScintCubeMessenger(this);
}

DetectorScintCube::~DetectorScintCube() {
    delete fDetectorScintCubeMessenger;
    fDetectorScintCubeMessenger = nullptr;
}

void DetectorScintCube::DefineMaterials() {
    EnsureMaterials(fDefaultConfig);
}

void DetectorScintCube::DefineVolumes() {
    fAreVolumensDefined = true;
}

ScintCubeDetectorConfig& DetectorScintCube::EnsureDetectorConfig(G4int detectorID) {
    const auto it = fDetectorConfigs.find(detectorID);
    if (it != fDetectorConfigs.end()) {
        return it->second;
    }

    auto [insertedIt, inserted] = fDetectorConfigs.emplace(detectorID, fDefaultConfig);
    (void)inserted;
    return insertedIt->second;
}

void DetectorScintCube::EnsureMaterials(const ScintCubeDetectorConfig& config) {
    auto* nistManager = G4NistManager::Instance();
    if (detMat.find(config.materialName) == detMat.end()) {
        auto* material = nistManager->FindOrBuildMaterial(config.materialName, false);
        if (material == nullptr) {
            G4Exception("DetectorScintCube::EnsureMaterials",
                        "DetectorScintCubeInvalidMaterial",
                        FatalException,
                        ("Material " + config.materialName +
                         " was not found in the NIST database.").c_str());
            return;
        }
        detMat[config.materialName] = material;
    }

    if (config.envelopeThickness > 0. &&
        detMat.find(config.envelopeMaterialName) == detMat.end()) {
        auto* material = nistManager->FindOrBuildMaterial(config.envelopeMaterialName, false);
        if (material == nullptr) {
            G4Exception("DetectorScintCube::EnsureMaterials",
                        "DetectorScintCubeInvalidEnvelopeMaterial",
                        FatalException,
                        ("Envelope material " + config.envelopeMaterialName +
                         " was not found in the NIST database.").c_str());
            return;
        }
        detMat[config.envelopeMaterialName] = material;
    }
}

void DetectorScintCube::SetCubeSide(G4int detectorID, G4double cubeSide) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.cubeSide = cubeSide;
    fDefaultConfig = config;
}

void DetectorScintCube::SetCubeMaterial(G4int detectorID, const G4String& materialName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.materialName = materialName;
    fDefaultConfig = config;
}

void DetectorScintCube::SetEnvelopeThickness(G4int detectorID, G4double envelopeThickness) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.envelopeThickness = envelopeThickness;
    fDefaultConfig = config;
}

void DetectorScintCube::SetEnvelopeMaterial(G4int detectorID, const G4String& materialName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.envelopeMaterialName = materialName;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSplitAtInterface(G4int detectorID, G4bool splitAtInterface) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.splitAtInterface = splitAtInterface;
    fDefaultConfig = config;
}

void DetectorScintCube::SetScintillationYield(G4int detectorID, G4double scintillationYieldPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.scintillationYield = scintillationYieldPerMeV / MeV;
    fDefaultConfig = config;
}

void DetectorScintCube::SetBirksConstant(G4int detectorID, G4double birksConstantInMmPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.birksConstant = birksConstantInMmPerMeV * mm / MeV;
    fDefaultConfig = config;
}

void DetectorScintCube::SetLightCollectionEfficiency(G4int detectorID, G4double lightCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.lightCollectionEfficiency = lightCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorScintCube::SetDecayTime(G4int detectorID, G4double decayTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.decayTime = decayTime;
    fDefaultConfig = config;
}

void DetectorScintCube::SetTransportDelay(G4int detectorID, G4double transportDelay) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.transportDelay = transportDelay;
    fDefaultConfig = config;
}

void DetectorScintCube::SetTimeJitter(G4int detectorID, G4double timeJitter) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.timeJitter = timeJitter;
    fDefaultConfig = config;
}

void DetectorScintCube::SetResolutionScale(G4int detectorID, G4double resolutionScale) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.resolutionScale = resolutionScale;
    fDefaultConfig = config;
}

void DetectorScintCube::SetPhotosensorType(G4int detectorID,
                                           ScintCubePhotosensorType photosensorType) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.photosensorType = photosensorType;
    fDefaultConfig = config;
}

void DetectorScintCube::SetPMTQuantumEfficiency(G4int detectorID, G4double quantumEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.quantumEfficiency = quantumEfficiency;
    fDefaultConfig = config;
}

void DetectorScintCube::SetPMTDynodeCollectionEfficiency(G4int detectorID,
                                                         G4double dynodeCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.dynodeCollectionEfficiency = dynodeCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorScintCube::SetPMTTransitTime(G4int detectorID, G4double transitTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTime = transitTime;
    fDefaultConfig = config;
}

void DetectorScintCube::SetPMTTransitTimeSpread(G4int detectorID, G4double transitTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTimeSpread = transitTimeSpread;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSiPMPDE(G4int detectorID, G4double photoDetectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.photoDetectionEfficiency = photoDetectionEfficiency;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSiPMMicrocellCount(G4int detectorID, G4double microcellCount) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.microcellCount = microcellCount;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSiPMExcessNoiseFactor(G4int detectorID, G4double excessNoiseFactor) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.excessNoiseFactor = excessNoiseFactor;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSiPMAvalancheTime(G4int detectorID, G4double avalancheTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTime = avalancheTime;
    fDefaultConfig = config;
}

void DetectorScintCube::SetSiPMAvalancheTimeSpread(G4int detectorID, G4double avalancheTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTimeSpread = avalancheTimeSpread;
    fDefaultConfig = config;
}

void DetectorScintCube::SetDoseCalibrationFactor(G4int detectorID,
                                                 G4double doseCalibrationFactorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactor = doseCalibrationFactorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

void DetectorScintCube::SetDoseCalibrationFactorError(
    G4int detectorID,
    G4double doseCalibrationFactorErrorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactorError =
        doseCalibrationFactorErrorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

const ScintCubeDetectorConfig& DetectorScintCube::GetDetectorConfig(G4int detectorID) const {
    const auto it = fDetectorConfigs.find(detectorID);
    return (it != fDetectorConfigs.end()) ? it->second : fDefaultConfig;
}

std::map<G4int, ScintCubeReadoutParameters> DetectorScintCube::GetReadoutParametersByDetector() const {
    std::map<G4int, ScintCubeReadoutParameters> parametersByDetector;
    for (const auto copyNo : GetPlacementCopyNumbers()) {
        parametersByDetector.emplace(copyNo, GetDetectorConfig(copyNo).readoutParameters);
    }
    return parametersByDetector;
}

ScintCubeCalibrationParameters DetectorScintCube::GetCalibrationParameters(G4int detectorID) const {
    return GetDetectorConfig(detectorID).calibrationParameters;
}

DetectorScintCube::StandardPlacementParts DetectorScintCube::BuildStandardPlacementVolumes(
    const ScintCubeDetectorConfig& config,
    const G4String& suffix) {
    StandardPlacementParts parts;

    auto* sensitiveSolid =
        new G4Box("DetectorScintCube" + suffix, config.cubeSide / 2., config.cubeSide / 2., config.cubeSide / 2.);
    parts.ownedResources.solids.push_back(sensitiveSolid);
    parts.sensitive = new G4LogicalVolume(
        sensitiveSolid, detMat[config.materialName], "DetectorScintCube" + suffix);
    parts.ownedResources.logicalVolumes.push_back(parts.sensitive);
    parts.ownedResources.sensitiveLogicalVolumes.push_back(parts.sensitive);
    auto* sensitiveVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
    parts.ownedResources.visAttributes.push_back(sensitiveVis);
    sensitiveVis->SetVisibility(true);
    sensitiveVis->SetForceSolid(true);
    parts.sensitive->SetVisAttributes(sensitiveVis);
    if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
        parts.sensitive->SetSensitiveDetector(sensitiveDetector);
    }

    if (config.envelopeThickness <= 0.) {
        return parts;
    }

    const G4double envelopeSide = config.cubeSide + 2. * config.envelopeThickness;
    auto* envelopeSolid = new G4Box(
        "DetectorScintCubeEnvelope" + suffix,
        envelopeSide / 2.,
        envelopeSide / 2.,
        envelopeSide / 2.);
    parts.ownedResources.solids.push_back(envelopeSolid);
    parts.envelope = new G4LogicalVolume(
        envelopeSolid,
        detMat[config.envelopeMaterialName],
        "DetectorScintCubeEnvelope" + suffix);
    parts.ownedResources.logicalVolumes.push_back(parts.envelope);
    auto* envelopeVis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
    parts.ownedResources.visAttributes.push_back(envelopeVis);
    envelopeVis->SetVisibility(true);
    parts.envelope->SetVisAttributes(envelopeVis);

    auto* nestedPlacement = new G4PVPlacement(nullptr,
                                              G4ThreeVector(),
                                              parts.sensitive,
                                              "DetectorScintCube_core_phys" + suffix,
                                              parts.envelope,
                                              false,
                                              0,
                                              true);
    parts.ownedResources.nestedPhysicalVolumes.push_back(nestedPlacement);
    return parts;
}

DetectorScintCube::SplitPlacementParts DetectorScintCube::BuildSplitPlacementVolumes(
    const ScintCubeDetectorConfig& config,
    const G4String& suffix,
    G4double waterOuterThickness,
    G4double airOuterThickness,
    G4double waterSensitiveThickness,
    G4double airSensitiveThickness) {
    SplitPlacementParts parts;

    const auto buildSensitiveLogical =
        [&](const G4String& name, G4double thickness) -> G4LogicalVolume* {
        if (thickness <= 0.) {
            return nullptr;
        }

        auto* solid = new G4Box(name, config.cubeSide / 2., config.cubeSide / 2., thickness / 2.);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[config.materialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
        parts.ownedResources.sensitiveLogicalVolumes.push_back(logical);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
        parts.ownedResources.visAttributes.push_back(vis);
        vis->SetVisibility(true);
        vis->SetForceSolid(true);
        logical->SetVisAttributes(vis);
        if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
            logical->SetSensitiveDetector(sensitiveDetector);
        }
        return logical;
    };

    const auto buildEnvelopeLogical =
        [&](const G4String& name, G4double thickness) -> G4LogicalVolume* {
        if (thickness <= 0.) {
            return nullptr;
        }

        const G4double outerSide = config.cubeSide + 2. * config.envelopeThickness;
        auto* solid = new G4Box(name, outerSide / 2., outerSide / 2., thickness / 2.);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[config.envelopeMaterialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        parts.ownedResources.visAttributes.push_back(vis);
        vis->SetVisibility(true);
        logical->SetVisAttributes(vis);
        return logical;
    };

    if (config.envelopeThickness > 0.) {
        parts.waterEnvelope =
            buildEnvelopeLogical("DetectorScintCubeEnvelope_water" + suffix, waterOuterThickness);
        parts.airEnvelope =
            buildEnvelopeLogical("DetectorScintCubeEnvelope_air" + suffix, airOuterThickness);
    }
    parts.waterSensitive =
        buildSensitiveLogical("DetectorScintCube_water" + suffix, waterSensitiveThickness);
    parts.airSensitive =
        buildSensitiveLogical("DetectorScintCube_air" + suffix, airSensitiveThickness);
    return parts;
}

void DetectorScintCube::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorScintCube::AddGeometry(G4LogicalVolume* motherVolume,
                                    const G4ThreeVector& position,
                                    G4RotationMatrix* rotation,
                                    G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorScintCube::AddGeometry(G4LogicalVolume* motherVolume,
                                    G4Transform3D* transformation,
                                    G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    auto& config = EnsureDetectorConfig(copyNo);
    if (config.cubeSide <= 0.) {
        G4Exception("DetectorScintCube::AddGeometry",
                    "DetectorScintCubeInvalidCubeSide",
                    FatalException,
                    "ScintCube side length must be strictly positive.");
        return;
    }
    if (config.envelopeThickness < 0.) {
        G4Exception("DetectorScintCube::AddGeometry",
                    "DetectorScintCubeInvalidEnvelopeThickness",
                    FatalException,
                    "ScintCube envelope thickness must be non-negative.");
        return;
    }

    EnsureMaterials(config);

    const G4String suffix = "_" + std::to_string(copyNo);
    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    const G4ThreeVector centerRelativeToMother = finalTransform.getTranslation();
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    const G4double outerHalfSize = GetOuterHalfSizeZ(config);

    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end()) {
        delete rotationIt->second;
        rotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);

    if (config.splitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterTube") {
        G4Exception("DetectorScintCube::AddGeometry",
                    "DetectorScintCubeSplitWaterTubeUnsupported",
                    FatalException,
                    "split at interface is not supported when scintCube is added to WaterTube.");
        return;
    }

    if (config.splitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterBox") {
        ValidateSplitPlacementSupport(config, motherVolume, centerRelativeToMother, outerHalfSize, copyNo);

        auto* waterPhysicalVolume = GetWaterPhysicalVolume();
        const G4ThreeVector waterWorldTranslation = waterPhysicalVolume->GetTranslation();
        auto* worldMother = G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false);
        auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
        if (worldMother == nullptr || waterBoxSolid == nullptr) {
            G4Exception("DetectorScintCube::AddGeometry",
                        "DetectorScintCubeSplitPlacementUnavailable",
                        FatalException,
                        "Could not resolve world_log or WaterBox solid while placing scintCube.");
            return;
        }

        const G4double waterMinX = -waterBoxSolid->GetXHalfLength();
        const G4double waterMaxX = waterBoxSolid->GetXHalfLength();
        const G4double waterMinY = -waterBoxSolid->GetYHalfLength();
        const G4double waterMaxY = waterBoxSolid->GetYHalfLength();
        const G4double waterMinZ = -waterBoxSolid->GetZHalfLength();
        const G4double interfaceRelativeZ = waterBoxSolid->GetZHalfLength();
        const G4double outerMinX = centerRelativeToMother.x() - outerHalfSize;
        const G4double outerMaxX = centerRelativeToMother.x() + outerHalfSize;
        const G4double outerMinY = centerRelativeToMother.y() - outerHalfSize;
        const G4double outerMaxY = centerRelativeToMother.y() + outerHalfSize;
        const G4double outerMinZ = centerRelativeToMother.z() - outerHalfSize;
        const G4double outerMaxZ = centerRelativeToMother.z() + outerHalfSize;
        const G4bool fullyOutsideWater =
            outerMaxX <= waterMinX || outerMinX >= waterMaxX ||
            outerMaxY <= waterMinY || outerMinY >= waterMaxY ||
            outerMaxZ <= waterMinZ || outerMinZ >= interfaceRelativeZ;

        if (fullyOutsideWater) {
            auto parts = BuildStandardPlacementVolumes(config, suffix);
            auto* logical = (parts.envelope != nullptr) ? parts.envelope : parts.sensitive;
            auto* placement = new G4PVPlacement(
                nullptr,
                G4ThreeVector(waterWorldTranslation.x() + centerRelativeToMother.x(),
                              waterWorldTranslation.y() + centerRelativeToMother.y(),
                              waterWorldTranslation.z() + centerRelativeToMother.z()),
                logical,
                "DetectorScintCube_air_phys" + suffix,
                worldMother,
                false,
                copyNo,
                true);
            SetPrimaryFrameVolume(copyNo, placement);
            fPlacementResources[copyNo] = std::move(parts.ownedResources);
            detPosition[copyNo] = centerRelativeToMother;
            fAreVolumensAssembled = true;
            return;
        }

        if (outerMaxZ > interfaceRelativeZ) {
            const G4double waterOuterThickness = interfaceRelativeZ - outerMinZ;
            const G4double airOuterThickness = outerMaxZ - interfaceRelativeZ;
            const G4double sensitiveHalfSize = config.cubeSide / 2.;
            const G4double sensitiveMinZ = centerRelativeToMother.z() - sensitiveHalfSize;
            const G4double sensitiveMaxZ = centerRelativeToMother.z() + sensitiveHalfSize;
            const G4double waterSensitiveThickness =
                std::max(0., std::min(interfaceRelativeZ, sensitiveMaxZ) - sensitiveMinZ);
            const G4double airSensitiveThickness =
                std::max(0., sensitiveMaxZ - std::max(interfaceRelativeZ, sensitiveMinZ));

            auto splitPlacement = BuildSplitPlacementVolumes(config,
                                                             suffix,
                                                             waterOuterThickness,
                                                             airOuterThickness,
                                                             waterSensitiveThickness,
                                                             airSensitiveThickness);
            PlaceSplitPlacement(splitPlacement,
                                config,
                                motherVolume,
                                worldMother,
                                suffix,
                                centerRelativeToMother,
                                waterWorldTranslation,
                                interfaceRelativeZ,
                                copyNo);
            fPlacementResources[copyNo] = std::move(splitPlacement.ownedResources);
            detPosition[copyNo] = centerRelativeToMother;
            fAreVolumensAssembled = true;
            return;
        }
    }

    auto parts = BuildStandardPlacementVolumes(config, suffix);
    auto* logical = (parts.envelope != nullptr) ? parts.envelope : parts.sensitive;
    auto* placement = new G4PVPlacement(finalTransform,
                                        logical,
                                        "DetectorScintCube_phys" + suffix,
                                        motherVolume,
                                        false,
                                        copyNo,
                                        true);
    SetPrimaryFrameVolume(copyNo, placement);
    fPlacementResources[copyNo] = std::move(parts.ownedResources);
    detPosition[copyNo] = centerRelativeToMother;
    fAreVolumensAssembled = true;
}

void DetectorScintCube::PlaceSplitPlacement(SplitPlacementParts& parts,
                                            const ScintCubeDetectorConfig& config,
                                            G4LogicalVolume* waterMother,
                                            G4LogicalVolume* worldMother,
                                            const G4String& suffix,
                                            const G4ThreeVector& centerRelativeToWater,
                                            const G4ThreeVector& waterWorldTranslation,
                                            G4double interfaceRelativeZ,
                                            G4int copyNo) {
    const G4double outerHalfSize = GetOuterHalfSizeZ(config);
    const G4double cubeHalfSize = config.cubeSide / 2.;
    const G4double outerMinZ = centerRelativeToWater.z() - outerHalfSize;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerHalfSize;
    const G4double waterOuterCenterZ = 0.5 * (outerMinZ + interfaceRelativeZ);
    const G4double airOuterCenterZ = 0.5 * (interfaceRelativeZ + outerMaxZ);
    const G4double cubeMinZ = centerRelativeToWater.z() - cubeHalfSize;
    const G4double cubeMaxZ = centerRelativeToWater.z() + cubeHalfSize;

    const auto placeSensitiveIntoEnvelope =
        [&](G4LogicalVolume* sensitiveLogical,
            G4LogicalVolume* envelopeLogical,
            G4double sensitiveCenterZ,
            G4double envelopeCenterZ) {
        if (sensitiveLogical == nullptr || envelopeLogical == nullptr) {
            return;
        }
        auto* nestedPlacement = new G4PVPlacement(nullptr,
                                                  G4ThreeVector(0., 0., sensitiveCenterZ - envelopeCenterZ),
                                                  sensitiveLogical,
                                                  sensitiveLogical->GetName() + "_core_phys",
                                                  envelopeLogical,
                                                  false,
                                                  copyNo,
                                                  true);
        parts.ownedResources.nestedPhysicalVolumes.push_back(nestedPlacement);
    };

    if (config.envelopeThickness > 0.) {
        const G4double waterSensitiveCenterZ =
            0.5 * (cubeMinZ + std::min(interfaceRelativeZ, cubeMaxZ));
        const G4double airSensitiveCenterZ =
            0.5 * (std::max(interfaceRelativeZ, cubeMinZ) + cubeMaxZ);
        placeSensitiveIntoEnvelope(parts.waterSensitive,
                                   parts.waterEnvelope,
                                   waterSensitiveCenterZ,
                                   waterOuterCenterZ);
        placeSensitiveIntoEnvelope(parts.airSensitive,
                                   parts.airEnvelope,
                                   airSensitiveCenterZ,
                                   airOuterCenterZ);
    }

    if (parts.waterEnvelope != nullptr || parts.waterSensitive != nullptr) {
        auto* logical = (parts.waterEnvelope != nullptr) ? parts.waterEnvelope : parts.waterSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            G4ThreeVector(centerRelativeToWater.x(),
                                                          centerRelativeToWater.y(),
                                                          waterOuterCenterZ),
                                            logical,
                                            "DetectorScintCube_water_phys" + suffix,
                                            waterMother,
                                            false,
                                            copyNo,
                                            true);
        SetPrimaryFrameVolume(copyNo, placement);
    }

    if (parts.airEnvelope != nullptr || parts.airSensitive != nullptr) {
        auto* logical = (parts.airEnvelope != nullptr) ? parts.airEnvelope : parts.airSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            G4ThreeVector(waterWorldTranslation.x() + centerRelativeToWater.x(),
                                                          waterWorldTranslation.y() + centerRelativeToWater.y(),
                                                          waterWorldTranslation.z() + airOuterCenterZ),
                                            logical,
                                            "DetectorScintCube_air_phys" + suffix,
                                            worldMother,
                                            false,
                                            copyNo,
                                            true);
        AddAuxiliaryFrameVolume(copyNo, placement);
    }
}

void DetectorScintCube::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
    fSensitiveDetector = sensitiveDetector;
    if (sensitiveDetector == nullptr) {
        return;
    }

    for (auto& [copyNo, resources] : fPlacementResources) {
        (void)copyNo;
        for (auto* logicalVolume : resources.sensitiveLogicalVolumes) {
            if (logicalVolume != nullptr) {
                logicalVolume->SetSensitiveDetector(sensitiveDetector);
            }
        }
    }
}

G4double DetectorScintCube::GetOuterHalfSizeZ(const ScintCubeDetectorConfig& config) const {
    return (config.cubeSide + 2. * config.envelopeThickness) / 2.;
}

G4bool DetectorScintCube::HasNonIdentityRotation(const G4int& copyNo) const {
    const auto rotationIt = detRotMat.find(copyNo);
    return rotationIt != detRotMat.end() &&
           rotationIt->second != nullptr &&
           !rotationIt->second->isIdentity();
}

G4bool DetectorScintCube::RequiresPlacementRebuild(const G4int& copyNo) const {
    const auto& config = GetDetectorConfig(copyNo);
    if (!config.splitAtInterface) {
        return false;
    }

    const auto motherIt = detMotherVolumeNames.find(copyNo);
    return motherIt != detMotherVolumeNames.end() && motherIt->second == "WaterBox";
}

void DetectorScintCube::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleasePlacementResources(copyNo);
}

void DetectorScintCube::ReleasePlacementResources(const G4int& copyNo) {
    const auto resourcesIt = fPlacementResources.find(copyNo);
    if (resourcesIt == fPlacementResources.end()) {
        return;
    }

    auto& resources = resourcesIt->second;
    for (auto* nestedPlacement : resources.nestedPhysicalVolumes) {
        delete nestedPlacement;
    }
    for (auto* logicalVolume : resources.logicalVolumes) {
        delete logicalVolume;
    }
    for (auto* solid : resources.solids) {
        delete solid;
    }
    for (auto* visAttributes : resources.visAttributes) {
        delete visAttributes;
    }

    fPlacementResources.erase(resourcesIt);
}

void DetectorScintCube::ValidateSplitPlacementSupport(const ScintCubeDetectorConfig& config,
                                                      G4LogicalVolume* motherVolume,
                                                      const G4ThreeVector& centerRelativeToWater,
                                                      G4double outerHalfSize,
                                                      G4int copyNo) const {
    if (motherVolume == nullptr || motherVolume->GetName() != "WaterBox") {
        G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                    "DetectorScintCubeSplitMotherNotSupported",
                    FatalException,
                    "split at interface is only supported when scintCube is added to WaterBox.");
        return;
    }

    if (HasNonIdentityRotation(copyNo)) {
        G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                    "DetectorScintCubeSplitRotationNotSupported",
                    FatalException,
                    "split at interface does not support rotated scintCube placements.");
        return;
    }

    auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
    if (waterBoxSolid == nullptr) {
        G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                    "DetectorScintCubeSplitMotherShapeNotSupported",
                    FatalException,
                    "split at interface requires WaterBox to be a G4Box.");
        return;
    }

    const G4double interfaceZ = waterBoxSolid->GetZHalfLength();
    const G4double waterMinX = -waterBoxSolid->GetXHalfLength();
    const G4double waterMaxX = waterBoxSolid->GetXHalfLength();
    const G4double waterMinY = -waterBoxSolid->GetYHalfLength();
    const G4double waterMaxY = waterBoxSolid->GetYHalfLength();
    const G4double waterMinZ = -waterBoxSolid->GetZHalfLength();
    const G4double outerMinX = centerRelativeToWater.x() - outerHalfSize;
    const G4double outerMaxX = centerRelativeToWater.x() + outerHalfSize;
    const G4double outerMinY = centerRelativeToWater.y() - outerHalfSize;
    const G4double outerMaxY = centerRelativeToWater.y() + outerHalfSize;
    const G4double outerMinZ = centerRelativeToWater.z() - outerHalfSize;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerHalfSize;
    const G4bool fullyInsideWater =
        outerMinX >= waterMinX && outerMaxX <= waterMaxX &&
        outerMinY >= waterMinY && outerMaxY <= waterMaxY &&
        outerMinZ >= waterMinZ && outerMaxZ <= interfaceZ;
    const G4bool fullyOutsideWater =
        outerMaxX <= waterMinX || outerMinX >= waterMaxX ||
        outerMaxY <= waterMinY || outerMinY >= waterMaxY ||
        outerMaxZ <= waterMinZ || outerMinZ >= interfaceZ;

    if (fullyInsideWater || fullyOutsideWater) {
        return;
    }

    if (outerMinZ < waterMinZ) {
        G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                    "DetectorScintCubeSplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface only supports crossing the top face of WaterBox.");
        return;
    }

    if (outerMinX < waterMinX || outerMaxX > waterMaxX ||
        outerMinY < waterMinY || outerMaxY > waterMaxY) {
        G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                    "DetectorScintCubeSplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface does not support crossing the lateral faces of WaterBox.");
        return;
    }

    if (outerMinZ < interfaceZ && outerMaxZ > interfaceZ) {
        return;
    }

    G4Exception("DetectorScintCube::ValidateSplitPlacementSupport",
                "DetectorScintCubeSplitPlacementNotSupported",
                FatalException,
                "split at interface encountered an unsupported scintCube placement relative to WaterBox.");
}

G4VPhysicalVolume* DetectorScintCube::GetWaterPhysicalVolume() const {
    auto* waterPhysicalVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
    if (waterPhysicalVolume == nullptr) {
        G4Exception("DetectorScintCube::GetWaterPhysicalVolume",
                    "DetectorScintCubeWaterBoxPhysicalVolumeNotFound",
                    FatalException,
                    "Could not resolve the WaterBox physical volume while splitting scintCube.");
        return nullptr;
    }
    return waterPhysicalVolume;
}
