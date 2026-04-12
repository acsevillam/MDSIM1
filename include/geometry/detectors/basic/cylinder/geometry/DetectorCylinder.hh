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

#pragma once

#include <map>
#include <vector>

#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VSensitiveDetector.hh"
#include "globals.hh"

#include "geometry/base/GenericGeometry.hh"
#include "geometry/detectors/basic/cylinder/messenger/DetectorCylinderMessenger.hh"

class G4VisAttributes;
class G4VSolid;

class DetectorCylinder : public GenericGeometry {
public:
    DetectorCylinder(G4double cylinderRadius,
                     G4double cylinderHeight,
                     const G4String& materialName,
                     G4double envelopeThickness = 0.,
                     const G4String& envelopeMaterialName = "G4_AIR",
                     G4bool splitAtInterface = false,
                     G4double calibrationFactor = 0.,
                     G4double calibrationFactorError = 0.);
    ~DetectorCylinder() override;

    void DefineMaterials() override;
    void DefineVolumes() override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume,
                     const G4ThreeVector& position,
                     G4RotationMatrix* rotation,
                     G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume,
                     G4Transform3D* transformation,
                     G4int copyNo) override;

    void SetCylinderRadius(G4double cylinderRadius) { fCylinderRadius = cylinderRadius; }
    G4double GetCylinderRadius() const { return fCylinderRadius; }

    void SetCylinderHeight(G4double cylinderHeight) { fCylinderHeight = cylinderHeight; }
    G4double GetCylinderHeight() const { return fCylinderHeight; }

    void SetCylinderMaterial(const G4String& materialName) { fMaterialName = materialName; }
    const G4String& GetCylinderMaterial() const { return fMaterialName; }

    void SetEnvelopeThickness(G4double envelopeThickness) { fEnvelopeThickness = envelopeThickness; }
    G4double GetEnvelopeThickness() const { return fEnvelopeThickness; }

    void SetEnvelopeMaterial(const G4String& materialName) { fEnvelopeMaterialName = materialName; }
    const G4String& GetEnvelopeMaterial() const { return fEnvelopeMaterialName; }

    void SetSplitAtInterface(G4bool splitAtInterface) { fSplitAtInterface = splitAtInterface; }
    G4bool GetSplitAtInterface() const { return fSplitAtInterface; }

    void SetCalibrationFactor(G4double calibrationFactor) { fCalibrationFactor = calibrationFactor; }
    G4double GetCalibrationFactor() const { return fCalibrationFactor; }

    void SetCalibrationFactorError(G4double calibrationFactorError) {
        fCalibrationFactorError = calibrationFactorError;
    }
    G4double GetCalibrationFactorError() const { return fCalibrationFactorError; }

    void AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector);

protected:
    G4bool RequiresPlacementRebuild(const G4int& copyNo) const override;
    void OnAfterPlacementRemoval(const G4int& copyNo) override;

private:
    G4bool IsSensitiveLogicalVolumeName(const G4String& logicalVolumeName) const;
    G4VSensitiveDetector* GetCurrentSensitiveDetector() const;

    struct SplitPlacementOwnedResources {
        std::vector<G4VPhysicalVolume*> nestedPhysicalVolumes;
        std::vector<G4LogicalVolume*> logicalVolumes;
        std::vector<G4VSolid*> solids;
        std::vector<G4VisAttributes*> visAttributes;
    };

    struct SplitPlacementParts {
        G4LogicalVolume* waterSensitive = nullptr;
        G4LogicalVolume* airSensitive = nullptr;
        G4LogicalVolume* waterEnvelope = nullptr;
        G4LogicalVolume* airEnvelope = nullptr;
        SplitPlacementOwnedResources ownedResources;
    };

    SplitPlacementParts BuildSplitPlacementVolumes(const G4String& suffix,
                                                   G4double waterOuterHeight,
                                                   G4double airOuterHeight,
                                                   G4double waterSensitiveHeight,
                                                   G4double airSensitiveHeight);
    void PlaceSplitPlacement(SplitPlacementParts& parts,
                             G4LogicalVolume* waterMother,
                             G4LogicalVolume* worldMother,
                             const G4String& suffix,
                             const G4ThreeVector& centerRelativeToWater,
                             const G4ThreeVector& waterWorldTranslation,
                             G4double interfaceRelativeZ,
                             G4int copyNo);
    G4double GetOuterRadius() const;
    G4double GetOuterHalfHeight() const;
    G4bool HasNonIdentityRotation(const G4int& copyNo) const;
    void ReleaseSplitPlacementResources(const G4int& copyNo);
    void ValidateSplitPlacementSupport(G4LogicalVolume* motherVolume,
                                       const G4ThreeVector& centerRelativeToWater,
                                       G4double outerRadius,
                                       G4double outerHalfHeight,
                                       G4int copyNo) const;
    G4VPhysicalVolume* GetWaterPhysicalVolume() const;

    G4double fCylinderRadius;
    G4double fCylinderHeight;
    G4String fMaterialName;
    G4double fEnvelopeThickness;
    G4String fEnvelopeMaterialName;
    G4bool fSplitAtInterface;
    G4double fCalibrationFactor;
    G4double fCalibrationFactorError;
    DetectorCylinderMessenger* fDetectorCylinderMessenger;
    std::map<G4int, SplitPlacementOwnedResources> fSplitPlacementResources;
};
