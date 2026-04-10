/*
 *
 * Geant4 MultiDetector Simulation
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

// Geant4 Headers
#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"

// MultiDetector Headers
#include "geometry/detectors/cube/geometry/DetectorCube.hh"

namespace {

G4bool StartsWith(const G4String& value, const G4String& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

} // namespace

DetectorCube::DetectorCube(G4double cubeSide,
                           const G4String& materialName,
                           G4double envelopeThickness,
                           const G4String& envelopeMaterialName,
                           G4bool splitAtInterface,
                           G4double calibrationFactor,
                           G4double calibrationFactorError)
    : fCubeSide(cubeSide),
      fMaterialName(materialName),
      fEnvelopeThickness(envelopeThickness),
      fEnvelopeMaterialName(envelopeMaterialName),
      fSplitAtInterface(splitAtInterface),
      fCalibrationFactor(calibrationFactor),
      fCalibrationFactorError(calibrationFactorError),
      fDetectorCubeMessenger(nullptr) {
    geometryName = "DetectorCube";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorCubeMessenger = new DetectorCubeMessenger(this);
}

DetectorCube::~DetectorCube() {
    delete fDetectorCubeMessenger;
    fDetectorCubeMessenger = nullptr;
}

void DetectorCube::DefineMaterials() {
    if (detMat.find(fMaterialName) != detMat.end()) {
        if (fEnvelopeThickness <= 0. || detMat.find(fEnvelopeMaterialName) != detMat.end()) {
            return;
        }
    }

    auto* nistManager = G4NistManager::Instance();
    auto* material = nistManager->FindOrBuildMaterial(fMaterialName, false);
    if (material == nullptr) {
        G4Exception("DetectorCube::DefineMaterials",
                    "DetectorCubeInvalidMaterial",
                    FatalException,
                    ("Material " + fMaterialName + " was not found in the NIST database.").c_str());
        return;
    }

    detMat[fMaterialName] = material;

    if (fEnvelopeThickness > 0. && detMat.find(fEnvelopeMaterialName) == detMat.end()) {
        auto* envelopeMaterial = nistManager->FindOrBuildMaterial(fEnvelopeMaterialName, false);
        if (envelopeMaterial == nullptr) {
            G4Exception("DetectorCube::DefineMaterials",
                        "DetectorCubeInvalidEnvelopeMaterial",
                        FatalException,
                        ("Envelope material " + fEnvelopeMaterialName + " was not found in the NIST database.").c_str());
            return;
        }

        detMat[fEnvelopeMaterialName] = envelopeMaterial;
    }
}

void DetectorCube::DefineVolumes() {
    if (fAreVolumensDefined) {
        return;
    }

    DefineMaterials();

    auto* geoDetectorCube = new G4Box("DetectorCube", fCubeSide / 2., fCubeSide / 2., fCubeSide / 2.);
    detGeo["DetectorCube"] = geoDetectorCube;

    auto* logDetectorCube = new G4LogicalVolume(geoDetectorCube, detMat[fMaterialName], "DetectorCube");
    auto* visDetectorCube = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
    visDetectorCube->SetVisibility(true);
    visDetectorCube->SetForceSolid(true);
    logDetectorCube->SetVisAttributes(visDetectorCube);
    detLog["DetectorCube"] = logDetectorCube;

    if (fEnvelopeThickness > 0.) {
        const G4double envelopeSide = fCubeSide + 2. * fEnvelopeThickness;
        auto* geoDetectorCubeEnvelope =
            new G4Box("DetectorCubeEnvelope", envelopeSide / 2., envelopeSide / 2., envelopeSide / 2.);
        detGeo["DetectorCubeEnvelope"] = geoDetectorCubeEnvelope;

        auto* logDetectorCubeEnvelope =
            new G4LogicalVolume(geoDetectorCubeEnvelope, detMat[fEnvelopeMaterialName], "DetectorCubeEnvelope");
        auto* visDetectorCubeEnvelope = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        visDetectorCubeEnvelope->SetVisibility(true);
        logDetectorCubeEnvelope->SetVisAttributes(visDetectorCubeEnvelope);
        detLog["DetectorCubeEnvelope"] = logDetectorCubeEnvelope;

        new G4PVPlacement(nullptr,
                          G4ThreeVector(),
                          logDetectorCube,
                          "DetectorCube_core_phys",
                          logDetectorCubeEnvelope,
                          false,
                          0,
                          true);
    }

    fAreVolumensDefined = true;
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume,
                               const G4ThreeVector& position,
                               G4RotationMatrix* rotation,
                               G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    const G4ThreeVector centerRelativeToMother = finalTransform.getTranslation();
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    const G4double outerHalfSize = GetOuterHalfSizeZ();

    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end()) {
        delete rotationIt->second;
        rotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);

    if (fSplitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterBox") {
        ValidateSplitPlacementSupport(motherVolume, centerRelativeToMother, outerHalfSize, copyNo);

        auto* waterPhysicalVolume = GetWaterPhysicalVolume();
        const G4ThreeVector waterWorldTranslation = waterPhysicalVolume->GetTranslation();
        auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
        auto* worldMother = G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false);
        if (worldMother == nullptr) {
            G4Exception("DetectorCube::AddGeometry",
                        "DetectorCubeWorldLogicalVolumeNotFound",
                        FatalException,
                        "Could not resolve world_log while placing the cube at the water-air interface.");
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
            auto* placementLogical = (fEnvelopeThickness > 0.) ? detLog["DetectorCubeEnvelope"] : detLog["DetectorCube"];
            auto* placement = new G4PVPlacement(nullptr,
                                                G4ThreeVector(waterWorldTranslation.x() + centerRelativeToMother.x(),
                                                              waterWorldTranslation.y() + centerRelativeToMother.y(),
                                                              waterWorldTranslation.z() + centerRelativeToMother.z()),
                                                placementLogical,
                                                "DetectorCube_air_phys",
                                                worldMother,
                                                false,
                                                copyNo,
                                                true);
            SetPrimaryFrameVolume(copyNo, placement);
            fAreVolumensAssembled = true;
            detPosition[copyNo] = centerRelativeToMother;
            return;
        }

        if (outerMaxZ <= interfaceRelativeZ) {
            // Entire detector is inside the water phantom.
        } else {
            const G4double waterOuterThickness = interfaceRelativeZ - outerMinZ;
            const G4double airOuterThickness = outerMaxZ - interfaceRelativeZ;
            const G4double sensitiveHalfSize = fCubeSide / 2.;
            const G4double sensitiveMinZ = centerRelativeToMother.z() - sensitiveHalfSize;
            const G4double sensitiveMaxZ = centerRelativeToMother.z() + sensitiveHalfSize;
            const G4double waterSensitiveThickness =
                std::max(0., std::min(interfaceRelativeZ, sensitiveMaxZ) - sensitiveMinZ);
            const G4double airSensitiveThickness =
                std::max(0., sensitiveMaxZ - std::max(interfaceRelativeZ, sensitiveMinZ));

            const G4String suffix = "_" + std::to_string(copyNo);
            auto splitPlacement = BuildSplitPlacementVolumes(suffix,
                                                             waterOuterThickness,
                                                             airOuterThickness,
                                                             waterSensitiveThickness,
                                                             airSensitiveThickness);
            PlaceSplitPlacement(splitPlacement,
                                motherVolume,
                                worldMother,
                                suffix,
                                centerRelativeToMother,
                                waterWorldTranslation,
                                interfaceRelativeZ,
                                copyNo);
            fSplitPlacementResources[copyNo] = std::move(splitPlacement.ownedResources);
            fAreVolumensAssembled = true;
            detPosition[copyNo] = centerRelativeToMother;
            return;
        }
    }

    fAreVolumensAssembled = true;
    auto* placementLogical = (fEnvelopeThickness > 0.) ? detLog["DetectorCubeEnvelope"] : detLog["DetectorCube"];
    auto* placement = new G4PVPlacement(finalTransform,
                                        placementLogical,
                                        "DetectorCube_phys",
                                        motherVolume,
                                        false,
                                        copyNo,
                                        true);
    SetPrimaryFrameVolume(copyNo, placement);
    detPosition[copyNo] = centerRelativeToMother;
}

void DetectorCube::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
    if (sensitiveDetector == nullptr) {
        return;
    }

    auto* logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
    for (auto* logicalVolume : *logicalVolumeStore) {
        if (logicalVolume != nullptr && IsSensitiveLogicalVolumeName(logicalVolume->GetName())) {
            logicalVolume->SetSensitiveDetector(sensitiveDetector);
        }
    }
}

G4bool DetectorCube::IsSensitiveLogicalVolumeName(const G4String& logicalVolumeName) const {
    return logicalVolumeName == "DetectorCube" ||
           StartsWith(logicalVolumeName, "DetectorCube_water") ||
           StartsWith(logicalVolumeName, "DetectorCube_air");
}

G4VSensitiveDetector* DetectorCube::GetCurrentSensitiveDetector() const {
    const auto it = detLog.find("DetectorCube");
    if (it != detLog.end() && it->second != nullptr) {
        return it->second->GetSensitiveDetector();
    }

    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("DetectorCube", false);
    return (logicalVolume != nullptr) ? logicalVolume->GetSensitiveDetector() : nullptr;
}

G4bool DetectorCube::RequiresPlacementRebuild(const G4int& copyNo) const {
    if (!fSplitAtInterface) {
        return false;
    }

    auto motherIt = detMotherVolumeNames.find(copyNo);
    return motherIt != detMotherVolumeNames.end() && motherIt->second == "WaterBox";
}

DetectorCube::SplitPlacementParts DetectorCube::BuildSplitPlacementVolumes(
    const G4String& suffix,
    G4double waterOuterThickness,
    G4double airOuterThickness,
    G4double waterSensitiveThickness,
    G4double airSensitiveThickness) {
    SplitPlacementParts parts;

    const G4double outerSide = fCubeSide + 2. * fEnvelopeThickness;
    const auto buildSensitiveLogical = [&](const G4String& name, G4double thickness) -> G4LogicalVolume* {
        if (thickness <= 0.) {
            return nullptr;
        }

        auto* solid = new G4Box(name, fCubeSide / 2., fCubeSide / 2., thickness / 2.);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[fMaterialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
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

    if (fEnvelopeThickness <= 0.) {
        parts.waterSensitive = buildSensitiveLogical("DetectorCube_water" + suffix, waterSensitiveThickness);
        parts.airSensitive = buildSensitiveLogical("DetectorCube_air" + suffix, airSensitiveThickness);
        return parts;
    }

    const auto buildEnvelopeLogical = [&](const G4String& name, G4double thickness) -> G4LogicalVolume* {
        if (thickness <= 0.) {
            return nullptr;
        }

        auto* solid = new G4Box(name, outerSide / 2., outerSide / 2., thickness / 2.);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[fEnvelopeMaterialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        parts.ownedResources.visAttributes.push_back(vis);
        vis->SetVisibility(true);
        logical->SetVisAttributes(vis);
        return logical;
    };

    parts.waterEnvelope = buildEnvelopeLogical("DetectorCubeEnvelope_water" + suffix, waterOuterThickness);
    parts.airEnvelope = buildEnvelopeLogical("DetectorCubeEnvelope_air" + suffix, airOuterThickness);
    parts.waterSensitive = buildSensitiveLogical("DetectorCube_water" + suffix, waterSensitiveThickness);
    parts.airSensitive = buildSensitiveLogical("DetectorCube_air" + suffix, airSensitiveThickness);

    return parts;
}

void DetectorCube::PlaceSplitPlacement(SplitPlacementParts& parts,
                                       G4LogicalVolume* waterMother,
                                       G4LogicalVolume* worldMother,
                                       const G4String& suffix,
                                       const G4ThreeVector& centerRelativeToWater,
                                       const G4ThreeVector& waterWorldTranslation,
                                       G4double interfaceRelativeZ,
                                       G4int copyNo) {
    const G4double outerHalfSize = GetOuterHalfSizeZ();
    const G4double cubeHalfSize = fCubeSide / 2.;
    const G4double outerMinZ = centerRelativeToWater.z() - outerHalfSize;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerHalfSize;
    const G4double waterOuterCenterZ = 0.5 * (outerMinZ + interfaceRelativeZ);
    const G4double airOuterCenterZ = 0.5 * (interfaceRelativeZ + outerMaxZ);
    const G4double cubeMinZ = centerRelativeToWater.z() - cubeHalfSize;
    const G4double cubeMaxZ = centerRelativeToWater.z() + cubeHalfSize;

    auto placeSensitiveIntoEnvelope = [&](G4LogicalVolume* sensitiveLogical,
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

    if (fEnvelopeThickness > 0.) {
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
                                            "DetectorCube_water_phys" + suffix,
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
                                            "DetectorCube_air_phys" + suffix,
                                            worldMother,
                                            false,
                                            copyNo,
                                            true);
        AddAuxiliaryFrameVolume(copyNo, placement);
    }
}

G4double DetectorCube::GetOuterHalfSizeZ() const {
    return (fCubeSide + 2. * fEnvelopeThickness) / 2.;
}

G4bool DetectorCube::HasNonIdentityRotation(const G4int& copyNo) const {
    auto rotationIt = detRotMat.find(copyNo);
    return rotationIt != detRotMat.end() &&
           rotationIt->second != nullptr &&
           !rotationIt->second->isIdentity();
}

void DetectorCube::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleaseSplitPlacementResources(copyNo);
}

void DetectorCube::ReleaseSplitPlacementResources(const G4int& copyNo) {
    auto resourcesIt = fSplitPlacementResources.find(copyNo);
    if (resourcesIt == fSplitPlacementResources.end()) {
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

    fSplitPlacementResources.erase(resourcesIt);
}

void DetectorCube::ValidateSplitPlacementSupport(G4LogicalVolume* motherVolume,
                                                 const G4ThreeVector& centerRelativeToWater,
                                                 G4double outerHalfSize,
                                                 G4int copyNo) const {
    if (motherVolume == nullptr || motherVolume->GetName() != "WaterBox") {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitMotherNotSupported",
                    FatalException,
                    "split at interface is only supported when the cube is added to WaterBox.");
        return;
    }

    if (HasNonIdentityRotation(copyNo)) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitRotationNotSupported",
                    FatalException,
                    "split at interface does not support rotated cube placements.");
        return;
    }

    auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
    if (waterBoxSolid == nullptr) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitMotherShapeNotSupported",
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
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface only supports crossing the top face of WaterBox.");
        return;
    }

    if (outerMinX < waterMinX || outerMaxX > waterMaxX ||
        outerMinY < waterMinY || outerMaxY > waterMaxY) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface does not support crossing the lateral faces of WaterBox.");
        return;
    }

    if (outerMinZ < interfaceZ && outerMaxZ > interfaceZ) {
        return;
    }

    G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                "DetectorCubeSplitPlacementNotSupported",
                FatalException,
                "split at interface encountered an unsupported cube placement relative to WaterBox.");
}

G4VPhysicalVolume* DetectorCube::GetWaterPhysicalVolume() const {
    auto* waterPhysicalVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
    if (waterPhysicalVolume == nullptr) {
        G4Exception("DetectorCube::GetWaterPhysicalVolume",
                    "DetectorCubeWaterBoxPhysicalVolumeNotFound",
                    FatalException,
                    "Could not resolve the WaterBox physical volume while splitting the cube at the interface.");
        return nullptr;
    }
    return waterPhysicalVolume;
}
