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

#include "geometry/detectors/basic/sphere/geometry/DetectorSphere.hh"

#include <algorithm>
#include <cmath>

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Sphere.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"

namespace {

G4bool StartsWith(const G4String& value, const G4String& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

struct ThetaRange {
    G4double start = 0.;
    G4double delta = 0.;

    G4bool IsEmpty() const { return delta <= 0.; }
};

ThetaRange BuildUpperCapThetaRange(G4double radius, G4double cutZRelativeToCenter) {
    if (cutZRelativeToCenter >= radius) {
        return {};
    }
    if (cutZRelativeToCenter <= -radius) {
        return {0., 180. * deg};
    }

    const G4double thetaCut = std::acos(cutZRelativeToCenter / radius);
    return {0., thetaCut};
}

ThetaRange BuildLowerCapThetaRange(G4double radius, G4double cutZRelativeToCenter) {
    if (cutZRelativeToCenter <= -radius) {
        return {};
    }
    if (cutZRelativeToCenter >= radius) {
        return {0., 180. * deg};
    }

    const G4double thetaCut = std::acos(cutZRelativeToCenter / radius);
    return {thetaCut, 180. * deg - thetaCut};
}

} // namespace

DetectorSphere::DetectorSphere(G4double sphereRadius,
                               const G4String& materialName,
                               G4double envelopeThickness,
                               const G4String& envelopeMaterialName,
                               G4bool splitAtInterface,
                               G4double calibrationFactor,
                               G4double calibrationFactorError)
    : fSphereRadius(sphereRadius),
      fMaterialName(materialName),
      fEnvelopeThickness(envelopeThickness),
      fEnvelopeMaterialName(envelopeMaterialName),
      fSplitAtInterface(splitAtInterface),
      fCalibrationFactor(calibrationFactor),
      fCalibrationFactorError(calibrationFactorError),
      fDetectorSphereMessenger(nullptr) {
    geometryName = "DetectorSphere";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorSphereMessenger = new DetectorSphereMessenger(this);
}

DetectorSphere::~DetectorSphere() {
    delete fDetectorSphereMessenger;
    fDetectorSphereMessenger = nullptr;
}

void DetectorSphere::DefineMaterials() {
    if (detMat.find(fMaterialName) != detMat.end()) {
        if (fEnvelopeThickness <= 0. || detMat.find(fEnvelopeMaterialName) != detMat.end()) {
            return;
        }
    }

    auto* nistManager = G4NistManager::Instance();
    auto* material = nistManager->FindOrBuildMaterial(fMaterialName, false);
    if (material == nullptr) {
        G4Exception("DetectorSphere::DefineMaterials",
                    "DetectorSphereInvalidMaterial",
                    FatalException,
                    ("Material " + fMaterialName + " was not found in the NIST database.").c_str());
        return;
    }

    detMat[fMaterialName] = material;

    if (fEnvelopeThickness > 0. && detMat.find(fEnvelopeMaterialName) == detMat.end()) {
        auto* envelopeMaterial = nistManager->FindOrBuildMaterial(fEnvelopeMaterialName, false);
        if (envelopeMaterial == nullptr) {
            G4Exception("DetectorSphere::DefineMaterials",
                        "DetectorSphereInvalidEnvelopeMaterial",
                        FatalException,
                        ("Envelope material " + fEnvelopeMaterialName +
                         " was not found in the NIST database.")
                            .c_str());
            return;
        }

        detMat[fEnvelopeMaterialName] = envelopeMaterial;
    }
}

void DetectorSphere::DefineVolumes() {
    if (fAreVolumensDefined) {
        return;
    }

    DefineMaterials();

    auto* geoDetectorSphere =
        new G4Sphere("DetectorSphere", 0., fSphereRadius, 0., 360. * deg, 0., 180. * deg);
    detGeo["DetectorSphere"] = geoDetectorSphere;

    auto* logDetectorSphere =
        new G4LogicalVolume(geoDetectorSphere, detMat[fMaterialName], "DetectorSphere");
    auto* visDetectorSphere = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
    visDetectorSphere->SetVisibility(true);
    visDetectorSphere->SetForceSolid(true);
    logDetectorSphere->SetVisAttributes(visDetectorSphere);
    detLog["DetectorSphere"] = logDetectorSphere;

    if (fEnvelopeThickness > 0.) {
        const G4double outerRadius = GetOuterRadius();
        auto* geoDetectorSphereEnvelope = new G4Sphere(
            "DetectorSphereEnvelope", 0., outerRadius, 0., 360. * deg, 0., 180. * deg);
        detGeo["DetectorSphereEnvelope"] = geoDetectorSphereEnvelope;

        auto* logDetectorSphereEnvelope = new G4LogicalVolume(
            geoDetectorSphereEnvelope, detMat[fEnvelopeMaterialName], "DetectorSphereEnvelope");
        auto* visDetectorSphereEnvelope = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
        visDetectorSphereEnvelope->SetVisibility(true);
        logDetectorSphereEnvelope->SetVisAttributes(visDetectorSphereEnvelope);
        detLog["DetectorSphereEnvelope"] = logDetectorSphereEnvelope;

        new G4PVPlacement(nullptr,
                          G4ThreeVector(),
                          logDetectorSphere,
                          "DetectorSphere_core_phys",
                          logDetectorSphereEnvelope,
                          false,
                          0,
                          true);
    }

    fAreVolumensDefined = true;
}

void DetectorSphere::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorSphere::AddGeometry(G4LogicalVolume* motherVolume,
                                 const G4ThreeVector& position,
                                 G4RotationMatrix* rotation,
                                 G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorSphere::AddGeometry(G4LogicalVolume* motherVolume,
                                 G4Transform3D* transformation,
                                 G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    const G4Transform3D finalTransform = (*transformation) * G4Translate3D(det_origin);
    const G4ThreeVector centerRelativeToMother = finalTransform.getTranslation();
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    const G4double outerRadius = GetOuterRadius();

    auto rotationIt = detRotMat.find(copyNo);
    if (rotationIt != detRotMat.end()) {
        delete rotationIt->second;
        rotationIt->second = nullptr;
    }
    detRotMat[copyNo] = NewPtrRotMatrix(rotation);

    if (fSplitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterTube") {
        G4Exception("DetectorSphere::AddGeometry",
                    "DetectorSphereSplitWaterTubeUnsupported",
                    FatalException,
                    "split at interface is not supported when the sphere is added to WaterTube.");
        return;
    }

    if (fSplitAtInterface && motherVolume != nullptr && motherVolume->GetName() == "WaterBox") {
        ValidateSplitPlacementSupport(motherVolume, centerRelativeToMother, outerRadius, copyNo);

        auto* waterPhysicalVolume = GetWaterPhysicalVolume();
        const G4ThreeVector waterWorldTranslation = waterPhysicalVolume->GetTranslation();
        auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
        auto* worldMother = G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false);
        if (worldMother == nullptr) {
            G4Exception("DetectorSphere::AddGeometry",
                        "DetectorSphereWorldLogicalVolumeNotFound",
                        FatalException,
                        "Could not resolve world_log while placing the sphere at the water-air interface.");
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
        const G4double outerMinZ = centerRelativeToMother.z() - outerRadius;
        const G4double outerMaxZ = centerRelativeToMother.z() + outerRadius;
        const G4bool fullyOutsideWater =
            outerMaxX <= waterMinX || outerMinX >= waterMaxX || outerMaxY <= waterMinY ||
            outerMinY >= waterMaxY || outerMaxZ <= waterMinZ || outerMinZ >= interfaceRelativeZ;

        if (fullyOutsideWater) {
            auto* placementLogical =
                (fEnvelopeThickness > 0.) ? detLog["DetectorSphereEnvelope"] : detLog["DetectorSphere"];
            auto* placement = new G4PVPlacement(nullptr,
                                                G4ThreeVector(waterWorldTranslation.x() +
                                                                  centerRelativeToMother.x(),
                                                              waterWorldTranslation.y() +
                                                                  centerRelativeToMother.y(),
                                                              waterWorldTranslation.z() +
                                                                  centerRelativeToMother.z()),
                                                placementLogical,
                                                "DetectorSphere_air_phys",
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
            const G4double cutZRelativeToCenter = interfaceRelativeZ - centerRelativeToMother.z();
            const G4String suffix = "_" + std::to_string(copyNo);
            auto splitPlacement = BuildSplitPlacementVolumes(suffix, cutZRelativeToCenter);
            PlaceSplitPlacement(splitPlacement,
                                motherVolume,
                                worldMother,
                                suffix,
                                centerRelativeToMother,
                                waterWorldTranslation,
                                copyNo);
            fSplitPlacementResources[copyNo] = std::move(splitPlacement.ownedResources);
            fAreVolumensAssembled = true;
            detPosition[copyNo] = centerRelativeToMother;
            return;
        }
    }

    fAreVolumensAssembled = true;
    auto* placementLogical =
        (fEnvelopeThickness > 0.) ? detLog["DetectorSphereEnvelope"] : detLog["DetectorSphere"];
    auto* placement = new G4PVPlacement(finalTransform,
                                        placementLogical,
                                        "DetectorSphere_phys",
                                        motherVolume,
                                        false,
                                        copyNo,
                                        true);
    SetPrimaryFrameVolume(copyNo, placement);
    detPosition[copyNo] = centerRelativeToMother;
}

void DetectorSphere::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
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

G4bool DetectorSphere::IsSensitiveLogicalVolumeName(const G4String& logicalVolumeName) const {
    return logicalVolumeName == "DetectorSphere" ||
           StartsWith(logicalVolumeName, "DetectorSphere_water") ||
           StartsWith(logicalVolumeName, "DetectorSphere_air");
}

G4VSensitiveDetector* DetectorSphere::GetCurrentSensitiveDetector() const {
    const auto it = detLog.find("DetectorSphere");
    if (it != detLog.end() && it->second != nullptr) {
        return it->second->GetSensitiveDetector();
    }

    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume("DetectorSphere", false);
    return (logicalVolume != nullptr) ? logicalVolume->GetSensitiveDetector() : nullptr;
}

G4bool DetectorSphere::RequiresPlacementRebuild(const G4int& copyNo) const {
    if (!fSplitAtInterface) {
        return false;
    }

    auto motherIt = detMotherVolumeNames.find(copyNo);
    return motherIt != detMotherVolumeNames.end() && motherIt->second == "WaterBox";
}

DetectorSphere::SplitPlacementParts DetectorSphere::BuildSplitPlacementVolumes(
    const G4String& suffix,
    G4double cutZRelativeToCenter) {
    SplitPlacementParts parts;

    const auto buildSphereLogical = [&](const G4String& name,
                                        G4double radius,
                                        const G4String& materialName,
                                        const ThetaRange& thetaRange,
                                        const G4Colour& color,
                                        G4bool forceSolid) -> G4LogicalVolume* {
        if (radius <= 0. || thetaRange.IsEmpty()) {
            return nullptr;
        }

        auto* solid =
            new G4Sphere(name, 0., radius, 0., 360. * deg, thetaRange.start, thetaRange.delta);
        parts.ownedResources.solids.push_back(solid);
        auto* logical = new G4LogicalVolume(solid, detMat[materialName], name);
        parts.ownedResources.logicalVolumes.push_back(logical);
        auto* vis = new G4VisAttributes(color);
        parts.ownedResources.visAttributes.push_back(vis);
        vis->SetVisibility(true);
        if (forceSolid) {
            vis->SetForceSolid(true);
        }
        logical->SetVisAttributes(vis);
        return logical;
    };

    const auto waterOuterTheta = BuildLowerCapThetaRange(GetOuterRadius(), cutZRelativeToCenter);
    const auto airOuterTheta = BuildUpperCapThetaRange(GetOuterRadius(), cutZRelativeToCenter);
    const auto waterSensitiveTheta = BuildLowerCapThetaRange(fSphereRadius, cutZRelativeToCenter);
    const auto airSensitiveTheta = BuildUpperCapThetaRange(fSphereRadius, cutZRelativeToCenter);

    if (fEnvelopeThickness <= 0.) {
        parts.waterSensitive = buildSphereLogical("DetectorSphere_water" + suffix,
                                                  fSphereRadius,
                                                  fMaterialName,
                                                  waterSensitiveTheta,
                                                  G4Colour(0.0, 0.0, 1.0, 1.0),
                                                  true);
        parts.airSensitive = buildSphereLogical("DetectorSphere_air" + suffix,
                                                fSphereRadius,
                                                fMaterialName,
                                                airSensitiveTheta,
                                                G4Colour(0.0, 0.0, 1.0, 1.0),
                                                true);
        if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
            if (parts.waterSensitive != nullptr) {
                parts.waterSensitive->SetSensitiveDetector(sensitiveDetector);
            }
            if (parts.airSensitive != nullptr) {
                parts.airSensitive->SetSensitiveDetector(sensitiveDetector);
            }
        }
        return parts;
    }

    parts.waterEnvelope = buildSphereLogical("DetectorSphereEnvelope_water" + suffix,
                                             GetOuterRadius(),
                                             fEnvelopeMaterialName,
                                             waterOuterTheta,
                                             G4Colour(0.0, 0.7, 0.7, 0.25),
                                             false);
    parts.airEnvelope = buildSphereLogical("DetectorSphereEnvelope_air" + suffix,
                                           GetOuterRadius(),
                                           fEnvelopeMaterialName,
                                           airOuterTheta,
                                           G4Colour(0.0, 0.7, 0.7, 0.25),
                                           false);
    parts.waterSensitive = buildSphereLogical("DetectorSphere_water" + suffix,
                                              fSphereRadius,
                                              fMaterialName,
                                              waterSensitiveTheta,
                                              G4Colour(0.0, 0.0, 1.0, 1.0),
                                              true);
    parts.airSensitive = buildSphereLogical("DetectorSphere_air" + suffix,
                                            fSphereRadius,
                                            fMaterialName,
                                            airSensitiveTheta,
                                            G4Colour(0.0, 0.0, 1.0, 1.0),
                                            true);

    if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
        if (parts.waterSensitive != nullptr) {
            parts.waterSensitive->SetSensitiveDetector(sensitiveDetector);
        }
        if (parts.airSensitive != nullptr) {
            parts.airSensitive->SetSensitiveDetector(sensitiveDetector);
        }
    }

    return parts;
}

void DetectorSphere::PlaceSplitPlacement(SplitPlacementParts& parts,
                                         G4LogicalVolume* waterMother,
                                         G4LogicalVolume* worldMother,
                                         const G4String& suffix,
                                         const G4ThreeVector& centerRelativeToWater,
                                         const G4ThreeVector& waterWorldTranslation,
                                         G4int copyNo) {
    auto placeSensitiveIntoEnvelope = [&](G4LogicalVolume* sensitiveLogical,
                                          G4LogicalVolume* envelopeLogical) {
        if (sensitiveLogical == nullptr || envelopeLogical == nullptr) {
            return;
        }
        auto* nestedPlacement = new G4PVPlacement(nullptr,
                                                  G4ThreeVector(),
                                                  sensitiveLogical,
                                                  sensitiveLogical->GetName() + "_core_phys",
                                                  envelopeLogical,
                                                  false,
                                                  copyNo,
                                                  true);
        parts.ownedResources.nestedPhysicalVolumes.push_back(nestedPlacement);
    };

    if (fEnvelopeThickness > 0.) {
        placeSensitiveIntoEnvelope(parts.waterSensitive, parts.waterEnvelope);
        placeSensitiveIntoEnvelope(parts.airSensitive, parts.airEnvelope);
    }

    if (parts.waterEnvelope != nullptr || parts.waterSensitive != nullptr) {
        auto* logical = (parts.waterEnvelope != nullptr) ? parts.waterEnvelope : parts.waterSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            centerRelativeToWater,
                                            logical,
                                            "DetectorSphere_water_phys" + suffix,
                                            waterMother,
                                            false,
                                            copyNo,
                                            true);
        SetPrimaryFrameVolume(copyNo, placement);
    }

    if (parts.airEnvelope != nullptr || parts.airSensitive != nullptr) {
        auto* logical = (parts.airEnvelope != nullptr) ? parts.airEnvelope : parts.airSensitive;
        auto* placement = new G4PVPlacement(nullptr,
                                            waterWorldTranslation + centerRelativeToWater,
                                            logical,
                                            "DetectorSphere_air_phys" + suffix,
                                            worldMother,
                                            false,
                                            copyNo,
                                            true);
        AddAuxiliaryFrameVolume(copyNo, placement);
    }
}

G4double DetectorSphere::GetOuterRadius() const {
    return fSphereRadius + fEnvelopeThickness;
}

G4bool DetectorSphere::HasNonIdentityRotation(const G4int& copyNo) const {
    auto rotationIt = detRotMat.find(copyNo);
    return rotationIt != detRotMat.end() && rotationIt->second != nullptr &&
           !rotationIt->second->isIdentity();
}

void DetectorSphere::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleaseSplitPlacementResources(copyNo);
}

void DetectorSphere::ReleaseSplitPlacementResources(const G4int& copyNo) {
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

void DetectorSphere::ValidateSplitPlacementSupport(G4LogicalVolume* motherVolume,
                                                   const G4ThreeVector& centerRelativeToWater,
                                                   G4double outerRadius,
                                                   G4int copyNo) const {
    if (motherVolume == nullptr || motherVolume->GetName() != "WaterBox") {
        G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                    "DetectorSphereSplitMotherNotSupported",
                    FatalException,
                    "split at interface is only supported when the sphere is added to WaterBox.");
        return;
    }

    if (HasNonIdentityRotation(copyNo)) {
        G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                    "DetectorSphereSplitRotationNotSupported",
                    FatalException,
                    "split at interface does not support rotated sphere placements.");
        return;
    }

    auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
    if (waterBoxSolid == nullptr) {
        G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                    "DetectorSphereSplitMotherShapeNotSupported",
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
    const G4double outerMinZ = centerRelativeToWater.z() - outerRadius;
    const G4double outerMaxZ = centerRelativeToWater.z() + outerRadius;
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
        G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                    "DetectorSphereSplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface only supports crossing the top face of WaterBox.");
        return;
    }

    if (outerMinX < waterMinX || outerMaxX > waterMaxX || outerMinY < waterMinY ||
        outerMaxY > waterMaxY) {
        G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                    "DetectorSphereSplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface does not support crossing the lateral faces of WaterBox.");
        return;
    }

    if (outerMinZ < interfaceZ && outerMaxZ > interfaceZ) {
        return;
    }

    G4Exception("DetectorSphere::ValidateSplitPlacementSupport",
                "DetectorSphereSplitPlacementNotSupported",
                FatalException,
                "split at interface encountered an unsupported sphere placement relative to WaterBox.");
}

G4VPhysicalVolume* DetectorSphere::GetWaterPhysicalVolume() const {
    auto* waterPhysicalVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
    if (waterPhysicalVolume == nullptr) {
        G4Exception("DetectorSphere::GetWaterPhysicalVolume",
                    "DetectorSphereWaterBoxPhysicalVolumeNotFound",
                    FatalException,
                    "Could not resolve the WaterBox physical volume while splitting the sphere at the interface.");
        return nullptr;
    }
    return waterPhysicalVolume;
}
