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

#include "geometry/detectors/basic/cylinder/geometry/DetectorCylinder.hh"

#include <algorithm>

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"

namespace {

G4bool StartsWith(const G4String& value, const G4String& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

} // namespace

DetectorCylinder::DetectorCylinder(G4double cylinderRadius,
                                   G4double cylinderHeight,
                                   const G4String& materialName,
                                   G4double envelopeThickness,
                                   const G4String& envelopeMaterialName,
                                   G4bool splitAtInterface,
                                   G4double calibrationFactor,
                                   G4double calibrationFactorError)
    : fCylinderRadius(cylinderRadius),
      fCylinderHeight(cylinderHeight),
      fMaterialName(materialName),
      fEnvelopeThickness(envelopeThickness),
      fEnvelopeMaterialName(envelopeMaterialName),
      fSplitAtInterface(splitAtInterface),
      fCalibrationFactor(calibrationFactor),
      fCalibrationFactorError(calibrationFactorError),
      fDetectorCylinderMessenger(nullptr) {
    geometryName = "DetectorCylinder";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorCylinderMessenger = new DetectorCylinderMessenger(this);
}

DetectorCylinder::~DetectorCylinder() {
    delete fDetectorCylinderMessenger;
    fDetectorCylinderMessenger = nullptr;
}

void DetectorCylinder::DefineMaterials() {
    if (detMat.find(fMaterialName) != detMat.end()) {
        if (fEnvelopeThickness <= 0. || detMat.find(fEnvelopeMaterialName) != detMat.end()) {
            return;
        }
    }

    auto* nistManager = G4NistManager::Instance();
    auto* material = nistManager->FindOrBuildMaterial(fMaterialName, false);
    if (material == nullptr) {
        G4Exception("DetectorCylinder::DefineMaterials",
                    "DetectorCylinderInvalidMaterial",
                    FatalException,
                    ("Material " + fMaterialName + " was not found in the NIST database.").c_str());
        return;
    }

    detMat[fMaterialName] = material;

    if (fEnvelopeThickness > 0. && detMat.find(fEnvelopeMaterialName) == detMat.end()) {
        auto* envelopeMaterial = nistManager->FindOrBuildMaterial(fEnvelopeMaterialName, false);
        if (envelopeMaterial == nullptr) {
            G4Exception("DetectorCylinder::DefineMaterials",
                        "DetectorCylinderInvalidEnvelopeMaterial",
                        FatalException,
                        ("Envelope material " + fEnvelopeMaterialName +
                         " was not found in the NIST database.")
                            .c_str());
            return;
        }

        detMat[fEnvelopeMaterialName] = envelopeMaterial;
    }
}

void DetectorCylinder::DefineVolumes() {
    if (fAreVolumensDefined) {
        return;
    }

    DefineMaterials();

    auto* geoDetectorCylinder =
        new G4Tubs("DetectorCylinder", 0., fCylinderRadius, fCylinderHeight / 2., 0., 360. * deg);
    detGeo["DetectorCylinder"] = geoDetectorCylinder;

    auto* logDetectorCylinder =
        new G4LogicalVolume(geoDetectorCylinder, detMat[fMaterialName], "DetectorCylinder");
    auto* visDetectorCylinder = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
    visDetectorCylinder->SetVisibility(true);
    visDetectorCylinder->SetForceSolid(true);
    logDetectorCylinder->SetVisAttributes(visDetectorCylinder);
    detLog["DetectorCylinder"] = logDetectorCylinder;

    if (fEnvelopeThickness > 0.) {
        const G4double outerRadius = GetOuterRadius();
        const G4double outerHalfHeight = GetOuterHalfHeight();
        auto* geoDetectorCylinderEnvelope = new G4Tubs(
            "DetectorCylinderEnvelope", 0., outerRadius, outerHalfHeight, 0., 360. * deg);
        detGeo["DetectorCylinderEnvelope"] = geoDetectorCylinderEnvelope;

        auto* logDetectorCylinderEnvelope = new G4LogicalVolume(
            geoDetectorCylinderEnvelope, detMat[fEnvelopeMaterialName], "DetectorCylinderEnvelope");
        auto* visDetectorCylinderEnvelope = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        visDetectorCylinderEnvelope->SetVisibility(true);
        logDetectorCylinderEnvelope->SetVisAttributes(visDetectorCylinderEnvelope);
        detLog["DetectorCylinderEnvelope"] = logDetectorCylinderEnvelope;

        new G4PVPlacement(nullptr,
                          G4ThreeVector(),
                          logDetectorCylinder,
                          "DetectorCylinder_core_phys",
                          logDetectorCylinderEnvelope,
                          false,
                          0,
                          true);
    }

    fAreVolumensDefined = true;
}

void DetectorCylinder::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorCylinder::AddGeometry(G4LogicalVolume* motherVolume,
                                   const G4ThreeVector& position,
                                   G4RotationMatrix* rotation,
                                   G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorCylinder::AddGeometry(G4LogicalVolume* motherVolume,
                                   G4Transform3D* transformation,
                                   G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    const G4ThreeVector centerRelativeToMother = finalTransform.getTranslation();
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    const G4double outerRadius = GetOuterRadius();
    const G4double outerHalfHeight = GetOuterHalfHeight();

    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end()) {
        delete rotationIt->second;
        rotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);

    if (fSplitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterTube") {
        G4Exception("DetectorCylinder::AddGeometry",
                    "DetectorCylinderSplitWaterTubeUnsupported",
                    FatalException,
                    "split at interface is not supported when the cylinder is added to WaterTube.");
        return;
    }

    if (fSplitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterBox") {
        ValidateSplitPlacementSupport(
            motherVolume, centerRelativeToMother, outerRadius, outerHalfHeight, copyNo);

        auto* waterPhysicalVolume = GetWaterPhysicalVolume();
        const G4ThreeVector waterWorldTranslation = waterPhysicalVolume->GetTranslation();
        auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
        auto* worldMother = G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false);
        if (worldMother == nullptr) {
            G4Exception("DetectorCylinder::AddGeometry",
                        "DetectorCylinderWorldLogicalVolumeNotFound",
                        FatalException,
                        "Could not resolve world_log while placing the cylinder at the water-air interface.");
            return;
        }

        const G4double waterMinX = -waterBoxSolid->GetXHalfLength();
        const G4double waterMaxX = waterBoxSolid->GetXHalfLength();
        const G4double waterMinY = -waterBoxSolid->GetYHalfLength();
        const G4double waterMaxY = waterBoxSolid->GetYHalfLength();
        const G4double waterMinZ = -waterBoxSolid->GetZHalfLength();
        const G4double interfaceRelativeZ = waterBoxSolid->GetZHalfLength();
        const G4double outerMinX = centerRelativeToMother.x() - outerRadius;
        const G4double outerMaxX = centerRelativeToMother.x() + outerRadius;
        const G4double outerMinY = centerRelativeToMother.y() - outerRadius;
        const G4double outerMaxY = centerRelativeToMother.y() + outerRadius;
        const G4double outerMinZ = centerRelativeToMother.z() - outerHalfHeight;
        const G4double outerMaxZ = centerRelativeToMother.z() + outerHalfHeight;
        const G4bool fullyOutsideWater =
            outerMaxX <= waterMinX || outerMinX >= waterMaxX || outerMaxY <= waterMinY ||
            outerMinY >= waterMaxY || outerMaxZ <= waterMinZ || outerMinZ >= interfaceRelativeZ;

        if (fullyOutsideWater) {
            auto* placementLogical =
                (fEnvelopeThickness > 0.) ? detLog["DetectorCylinderEnvelope"] : detLog["DetectorCylinder"];
            auto* placement = new G4PVPlacement(nullptr,
                                                G4ThreeVector(waterWorldTranslation.x() +
                                                                  centerRelativeToMother.x(),
                                                              waterWorldTranslation.y() +
                                                                  centerRelativeToMother.y(),
                                                              waterWorldTranslation.z() +
                                                                  centerRelativeToMother.z()),
                                                placementLogical,
                                                "DetectorCylinder_air_phys",
                                                worldMother,
                                                false,
                                                copyNo,
                                                true);
            SetPrimaryFrameVolume(copyNo, placement);
            fAreVolumensAssembled = true;
            detPosition[copyNo] = centerRelativeToMother;
            return;
        }

        if (outerMaxZ > interfaceRelativeZ) {
            const G4double waterOuterHeight = interfaceRelativeZ - outerMinZ;
            const G4double airOuterHeight = outerMaxZ - interfaceRelativeZ;
            const G4double sensitiveHalfHeight = fCylinderHeight / 2.;
            const G4double sensitiveMinZ = centerRelativeToMother.z() - sensitiveHalfHeight;
            const G4double sensitiveMaxZ = centerRelativeToMother.z() + sensitiveHalfHeight;
            const G4double waterSensitiveHeight =
                std::max(0., std::min(interfaceRelativeZ, sensitiveMaxZ) - sensitiveMinZ);
            const G4double airSensitiveHeight =
                std::max(0., sensitiveMaxZ - std::max(interfaceRelativeZ, sensitiveMinZ));

            const G4String suffix = "_" + std::to_string(copyNo);
            auto splitPlacement = BuildSplitPlacementVolumes(
                suffix, waterOuterHeight, airOuterHeight, waterSensitiveHeight, airSensitiveHeight);
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
    auto* placementLogical =
        (fEnvelopeThickness > 0.) ? detLog["DetectorCylinderEnvelope"] : detLog["DetectorCylinder"];
    auto* placement = new G4PVPlacement(finalTransform,
                                        placementLogical,
                                        "DetectorCylinder_phys",
                                        motherVolume,
                                        false,
                                        copyNo,
                                        true);
    SetPrimaryFrameVolume(copyNo, placement);
    detPosition[copyNo] = centerRelativeToMother;
}

void DetectorCylinder::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
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

G4bool DetectorCylinder::IsSensitiveLogicalVolumeName(const G4String& logicalVolumeName) const {
    return logicalVolumeName == "DetectorCylinder" ||
           StartsWith(logicalVolumeName, "DetectorCylinder_water") ||
           StartsWith(logicalVolumeName, "DetectorCylinder_air");
}

G4VSensitiveDetector* DetectorCylinder::GetCurrentSensitiveDetector() const {
    const auto it = detLog.find("DetectorCylinder");
    if (it != detLog.end() && it->second != nullptr) {
        return it->second->GetSensitiveDetector();
    }

    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("DetectorCylinder", false);
    return (logicalVolume != nullptr) ? logicalVolume->GetSensitiveDetector() : nullptr;
}

G4bool DetectorCylinder::RequiresPlacementRebuild(const G4int& copyNo) const {
    if (!fSplitAtInterface) {
        return false;
    }

    auto motherIt = detMotherVolumeNames.find(copyNo);
    return motherIt != detMotherVolumeNames.end() && motherIt->second == "WaterBox";
}

DetectorCylinder::SplitPlacementParts DetectorCylinder::BuildSplitPlacementVolumes(
    const G4String& suffix,
    G4double waterOuterHeight,
    G4double airOuterHeight,
    G4double waterSensitiveHeight,
    G4double airSensitiveHeight) {
    SplitPlacementParts parts;

    const auto buildSensitiveLogical = [&](const G4String& name, G4double height) -> G4LogicalVolume* {
        if (height <= 0.) {
            return nullptr;
        }

        auto* solid = new G4Tubs(name, 0., fCylinderRadius, height / 2., 0., 360. * deg);
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
        parts.waterSensitive = buildSensitiveLogical("DetectorCylinder_water" + suffix, waterSensitiveHeight);
        parts.airSensitive = buildSensitiveLogical("DetectorCylinder_air" + suffix, airSensitiveHeight);
        return parts;
    }

    const G4double outerRadius = GetOuterRadius();
    const auto buildEnvelopeLogical = [&](const G4String& name, G4double height) -> G4LogicalVolume* {
        if (height <= 0.) {
            return nullptr;
        }

        auto* solid = new G4Tubs(name, 0., outerRadius, height / 2., 0., 360. * deg);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[fEnvelopeMaterialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        parts.ownedResources.visAttributes.push_back(vis);
        vis->SetVisibility(true);
        logical->SetVisAttributes(vis);
        return logical;
    };

    parts.waterEnvelope =
        buildEnvelopeLogical("DetectorCylinderEnvelope_water" + suffix, waterOuterHeight);
    parts.airEnvelope = buildEnvelopeLogical("DetectorCylinderEnvelope_air" + suffix, airOuterHeight);
    parts.waterSensitive = buildSensitiveLogical("DetectorCylinder_water" + suffix, waterSensitiveHeight);
    parts.airSensitive = buildSensitiveLogical("DetectorCylinder_air" + suffix, airSensitiveHeight);

    return parts;
}

void DetectorCylinder::PlaceSplitPlacement(SplitPlacementParts& parts,
                                           G4LogicalVolume* waterMother,
                                           G4LogicalVolume* worldMother,
                                           const G4String& suffix,
                                           const G4ThreeVector& centerRelativeToWater,
                                           const G4ThreeVector& waterWorldTranslation,
                                           G4double interfaceRelativeZ,
                                           G4int copyNo) {
    const G4double outerHalfHeight = GetOuterHalfHeight();
    const G4double sensitiveHalfHeight = fCylinderHeight / 2.;
    const G4double outerMinZ = centerRelativeToWater.z() - outerHalfHeight;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerHalfHeight;
    const G4double waterOuterCenterZ = 0.5 * (outerMinZ + interfaceRelativeZ);
    const G4double airOuterCenterZ = 0.5 * (interfaceRelativeZ + outerMaxZ);
    const G4double sensitiveMinZ = centerRelativeToWater.z() - sensitiveHalfHeight;
    const G4double sensitiveMaxZ = centerRelativeToWater.z() + sensitiveHalfHeight;

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
            0.5 * (sensitiveMinZ + std::min(interfaceRelativeZ, sensitiveMaxZ));
        const G4double airSensitiveCenterZ =
            0.5 * (std::max(interfaceRelativeZ, sensitiveMinZ) + sensitiveMaxZ);
        placeSensitiveIntoEnvelope(
            parts.waterSensitive, parts.waterEnvelope, waterSensitiveCenterZ, waterOuterCenterZ);
        placeSensitiveIntoEnvelope(
            parts.airSensitive, parts.airEnvelope, airSensitiveCenterZ, airOuterCenterZ);
    }

    if (parts.waterEnvelope != nullptr || parts.waterSensitive != nullptr) {
        auto* logical = (parts.waterEnvelope != nullptr) ? parts.waterEnvelope : parts.waterSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            G4ThreeVector(centerRelativeToWater.x(),
                                                          centerRelativeToWater.y(),
                                                          waterOuterCenterZ),
                                            logical,
                                            "DetectorCylinder_water_phys" + suffix,
                                            waterMother,
                                            false,
                                            copyNo,
                                            true);
        SetPrimaryFrameVolume(copyNo, placement);
    }

    if (parts.airEnvelope != nullptr || parts.airSensitive != nullptr) {
        auto* logical = (parts.airEnvelope != nullptr) ? parts.airEnvelope : parts.airSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            G4ThreeVector(waterWorldTranslation.x() +
                                                              centerRelativeToWater.x(),
                                                          waterWorldTranslation.y() +
                                                              centerRelativeToWater.y(),
                                                          waterWorldTranslation.z() + airOuterCenterZ),
                                            logical,
                                            "DetectorCylinder_air_phys" + suffix,
                                            worldMother,
                                            false,
                                            copyNo,
                                            true);
        AddAuxiliaryFrameVolume(copyNo, placement);
    }
}

G4double DetectorCylinder::GetOuterRadius() const {
    return fCylinderRadius + fEnvelopeThickness;
}

G4double DetectorCylinder::GetOuterHalfHeight() const {
    return (fCylinderHeight / 2.) + fEnvelopeThickness;
}

G4bool DetectorCylinder::HasNonIdentityRotation(const G4int& copyNo) const {
    auto rotationIt = detRotMat.find(copyNo);
    return rotationIt != detRotMat.end() && rotationIt->second != nullptr &&
           !rotationIt->second->isIdentity();
}

void DetectorCylinder::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleaseSplitPlacementResources(copyNo);
}

void DetectorCylinder::ReleaseSplitPlacementResources(const G4int& copyNo) {
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

void DetectorCylinder::ValidateSplitPlacementSupport(G4LogicalVolume* motherVolume,
                                                     const G4ThreeVector& centerRelativeToWater,
                                                     G4double outerRadius,
                                                     G4double outerHalfHeight,
                                                     G4int copyNo) const {
    if (motherVolume == nullptr || motherVolume->GetName() != "WaterBox") {
        G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                    "DetectorCylinderSplitMotherNotSupported",
                    FatalException,
                    "split at interface is only supported when the cylinder is added to WaterBox.");
        return;
    }

    if (HasNonIdentityRotation(copyNo)) {
        G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                    "DetectorCylinderSplitRotationNotSupported",
                    FatalException,
                    "split at interface does not support rotated cylinder placements.");
        return;
    }

    auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
    if (waterBoxSolid == nullptr) {
        G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                    "DetectorCylinderSplitMotherShapeNotSupported",
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
    const G4double outerMinX = centerRelativeToWater.x() - outerRadius;
    const G4double outerMaxX = centerRelativeToWater.x() + outerRadius;
    const G4double outerMinY = centerRelativeToWater.y() - outerRadius;
    const G4double outerMaxY = centerRelativeToWater.y() + outerRadius;
    const G4double outerMinZ = centerRelativeToWater.z() - outerHalfHeight;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerHalfHeight;
    const G4bool fullyInsideWater =
        outerMinX >= waterMinX && outerMaxX <= waterMaxX && outerMinY >= waterMinY &&
        outerMaxY <= waterMaxY && outerMinZ >= waterMinZ && outerMaxZ <= interfaceZ;
    const G4bool fullyOutsideWater =
        outerMaxX <= waterMinX || outerMinX >= waterMaxX || outerMaxY <= waterMinY ||
        outerMinY >= waterMaxY || outerMaxZ <= waterMinZ || outerMinZ >= interfaceZ;

    if (fullyInsideWater || fullyOutsideWater) {
        return;
    }

    if (outerMinZ < waterMinZ) {
        G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                    "DetectorCylinderSplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface only supports crossing the top face of WaterBox.");
        return;
    }

    if (outerMinX < waterMinX || outerMaxX > waterMaxX || outerMinY < waterMinY ||
        outerMaxY > waterMaxY) {
        G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                    "DetectorCylinderSplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface does not support crossing the lateral faces of WaterBox.");
        return;
    }

    if (outerMinZ < interfaceZ && outerMaxZ > interfaceZ) {
        return;
    }

    G4Exception("DetectorCylinder::ValidateSplitPlacementSupport",
                "DetectorCylinderSplitPlacementNotSupported",
                FatalException,
                "split at interface encountered an unsupported cylinder placement relative to WaterBox.");
}

G4VPhysicalVolume* DetectorCylinder::GetWaterPhysicalVolume() const {
    auto* waterPhysicalVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
    if (waterPhysicalVolume == nullptr) {
        G4Exception("DetectorCylinder::GetWaterPhysicalVolume",
                    "DetectorCylinderWaterBoxPhysicalVolumeNotFound",
                    FatalException,
                    "Could not resolve the WaterBox physical volume while splitting the cylinder at the interface.");
        return nullptr;
    }
    return waterPhysicalVolume;
}
