/*
 *
 * Geant4 MultiDetector Simulation v1
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

#ifndef PHANTOM_WATER_TUBE_H
#define PHANTOM_WATER_TUBE_H

#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4UserLimits.hh"
#include "globals.hh"

#include "geometry/base/GenericGeometry.hh"

class G4UImessenger;

/**
 * @class PhantomWaterTube
 * @brief Cylindrical water phantom based on the existing waterBox workflow.
 */
class PhantomWaterTube : public GenericGeometry {
public:
    PhantomWaterTube(G4double waterTubeRadius, G4double waterTubeHeight);

    ~PhantomWaterTube() override;

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

    void SetWaterTubeRadius(G4double waterTubeRadius);
    G4double GetWaterTubeRadius() const { return fWaterTubeRadius; }
    void SetWaterTubeHeight(G4double waterTubeHeight);
    G4double GetWaterTubeHeight() const { return fWaterTubeHeight; }

private:
    void InvalidateGeometryDefinition();

    G4UserLimits* fStepLimit;
    G4double fWaterTubeRadius;
    G4double fWaterTubeHeight;
    G4UImessenger* fPhantomWaterTubeMessenger;
};

#endif // PHANTOM_WATER_TUBE_H
