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
    fSensitiveLogicalVolumes.push_back(logDetectorCube);

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
    auto* rotation = new G4RotationMatrix();
    AddGeometry(motherVolume, G4ThreeVector(), rotation, copyNo);
}

void DetectorCube::AddGeometry(G4LogicalVolume* motherVolume,
                               const G4ThreeVector& position,
                               G4RotationMatrix* rotation,
                               G4int copyNo) {
    if (rotation == nullptr) {
        rotation = new G4RotationMatrix();
    }
    G4Transform3D transform(*rotation, position);
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
        const G4double interfaceRelativeZ = waterBoxSolid->GetZHalfLength();
        const G4double outerMinZ = centerRelativeToMother.z() - outerHalfSize;
        const G4double outerMaxZ = centerRelativeToMother.z() + outerHalfSize;

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
            const auto splitParts = BuildSplitPlacementVolumes(suffix,
                                                               waterOuterThickness,
                                                               airOuterThickness,
                                                               waterSensitiveThickness,
                                                               airSensitiveThickness);
            PlaceSplitPlacement(splitParts,
                                motherVolume,
                                G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false),
                                suffix,
                                centerRelativeToMother,
                                waterWorldTranslation,
                                interfaceRelativeZ,
                                copyNo);
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
    fActiveSensitiveDetector = sensitiveDetector;
    for (auto* logicalVolume : fSensitiveLogicalVolumes) {
        if (logicalVolume != nullptr) {
            logicalVolume->SetSensitiveDetector(sensitiveDetector);
        }
    }
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
        auto* logical = new G4LogicalVolume(solid, detMat[fMaterialName], name);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0));
        vis->SetVisibility(true);
        vis->SetForceSolid(true);
        logical->SetVisAttributes(vis);
        if (fActiveSensitiveDetector != nullptr) {
            logical->SetSensitiveDetector(fActiveSensitiveDetector);
        }
        fSensitiveLogicalVolumes.push_back(logical);
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
        auto* logical = new G4LogicalVolume(solid, detMat[fEnvelopeMaterialName], name);
        auto* vis = new G4VisAttributes(G4Colour(0.0, 0.7, 0.7, 0.25));
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

void DetectorCube::PlaceSplitPlacement(const SplitPlacementParts& parts,
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
        new G4PVPlacement(nullptr,
                          G4ThreeVector(0., 0., sensitiveCenterZ - envelopeCenterZ),
                          sensitiveLogical,
                          sensitiveLogical->GetName() + "_core_phys",
                          envelopeLogical,
                          false,
                          copyNo,
                          true);
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
    const G4double waterMinZ = -waterBoxSolid->GetZHalfLength();
    if (centerRelativeToWater.z() + outerHalfSize <= interfaceZ) {
        return;
    }

    if (centerRelativeToWater.z() - outerHalfSize >= interfaceZ) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitFullyOutsideWaterBox",
                    FatalException,
                    "split at interface does not support cube placements fully outside WaterBox.");
        return;
    }

    if (centerRelativeToWater.z() - outerHalfSize < waterMinZ) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface only supports crossing the top face of WaterBox.");
        return;
    }

    if (std::abs(centerRelativeToWater.x()) + outerHalfSize > waterBoxSolid->GetXHalfLength() ||
        std::abs(centerRelativeToWater.y()) + outerHalfSize > waterBoxSolid->GetYHalfLength()) {
        G4Exception("DetectorCube::ValidateSplitPlacementSupport",
                    "DetectorCubeSplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface does not support crossing the lateral faces of WaterBox.");
        return;
    }
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
