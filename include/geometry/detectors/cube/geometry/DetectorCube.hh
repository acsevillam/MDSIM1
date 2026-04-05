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

#ifndef DETECTOR_CUBE_H
#define DETECTOR_CUBE_H

// Geant4 Headers
#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"

// MultiDetector Headers
#include "geometry/base/GenericGeometry.hh"
#include "geometry/detectors/cube/messenger/DetectorCubeMessenger.hh"

class DetectorCube : public GenericGeometry {
public:
    DetectorCube(G4double cubeSide, const G4String& materialName, G4double calibrationFactor = 0.);
    ~DetectorCube() override;

    void DefineMaterials() override;
    void DefineVolumes() override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& position, G4RotationMatrix* rotation, G4int copyNo) override;
    void AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) override;

    void SetCubeSide(G4double cubeSide) { fCubeSide = cubeSide; }
    G4double GetCubeSide() const { return fCubeSide; }

    void SetCubeMaterial(const G4String& materialName) { fMaterialName = materialName; }
    const G4String& GetCubeMaterial() const { return fMaterialName; }

    void SetCalibrationFactor(G4double calibrationFactor) { fCalibrationFactor = calibrationFactor; }
    G4double GetCalibrationFactor() const { return fCalibrationFactor; }

private:
    G4double fCubeSide;
    G4String fMaterialName;
    G4double fCalibrationFactor;
    DetectorCubeMessenger* fDetectorCubeMessenger;
};

#endif // DETECTOR_CUBE_H
