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

#ifndef PHANTOM_WATER_BOX_H
#define PHANTOM_WATER_BOX_H

// Geant4 Headers
#include "globals.hh"
#include "G4UserLimits.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

// MultiDetector Headers
#include "geometry/base/GenericGeometry.hh"
#include "geometry/phantoms/PhantomWaterBoxMessenger.hh"

/**
 * @class PhantomWaterBox
 * @brief Class representing a phantom water box for Geant4 simulation.
 */
class PhantomWaterBox : public GenericGeometry {
public:
    /**
     * @brief Constructor for PhantomWaterBox.
     * @param fWaterBoxDx Dimension of the water box.
     */
    PhantomWaterBox(G4double fWaterBoxDx);

    /**
     * @brief Default destructor for PhantomWaterBox.
     */
    ~PhantomWaterBox() override;

    /**
     * @brief Define the materials used in the phantom water box.
     */
    void DefineMaterials() override;

    /**
     * @brief Define the geometrical volumes of the phantom water box.
     */
    void DefineVolumes() override;

    /**
     * @brief Add geometry to the mother volume.
     * @param motherVolume Logical volume of the mother.
     * @param copyNo Copy number for the geometry.
     */
    void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) override;

    /**
     * @brief Add geometry to the mother volume with a translation and rotation.
     * @param motherVolume Logical volume of the mother.
     * @param position Translation vector.
     * @param rotation Rotation matrix.
     * @param copyNo Copy number for the geometry.
     */
    void AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& position, G4RotationMatrix* rotation, G4int copyNo) override;

    /**
     * @brief Add geometry to the mother volume with a transformation.
     * @param motherVolume Logical volume of the mother.
     * @param transformation Transformation to apply.
     * @param copyNo Copy number for the geometry.
     */
    void AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) override;

    void SetWaterBoxSide(G4double waterBoxSide);
    void SetWaterBoxSize(const G4ThreeVector& waterBoxSize);
    G4double GetWaterBoxDx() const { return fWaterBoxDx; }
    G4double GetWaterBoxDy() const { return fWaterBoxDy; }
    G4double GetWaterBoxDz() const { return fWaterBoxDz; }
    G4ThreeVector GetWaterBoxSize() const { return G4ThreeVector(fWaterBoxDx, fWaterBoxDy, fWaterBoxDz); }

private:
    void InvalidateGeometryDefinition();

    G4UserLimits* fStepLimit; ///< Pointer to user step limits
    G4double fWaterBoxDx; ///< Water box size along X
    G4double fWaterBoxDy; ///< Water box size along Y
    G4double fWaterBoxDz; ///< Water box size along Z
    G4UImessenger* fPhantomWaterBoxMessenger;  ///< Water box messenger
};

#endif // PHANTOM_WATER_BOX_H
