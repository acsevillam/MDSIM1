/*
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
 */

#ifndef MLC80_H
#define MLC80_H

// Geant4 Headers
#include "globals.hh"
#include "G4UserLimits.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

// MultiDetector Headers
#include "GenericGeometry.hh"
#include "MLC80Messenger.hh"

class MLC80 : public GenericGeometry {
public:
    MLC80();
    ~MLC80() override = default;

    void DefineMaterials() override;
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

    /**
     * @brief Set the position of a specific leaf in bank A.
     * @param id Identifier of the leaf.
     * @param pos Position to set.
     */
    void SetLeafYPositionA(G4int id, G4double pos) { fLeafYPositionsA[id] = pos; }

    /**
     * @brief Set the width of a specific leaf in bank A.
     * @param id Identifier of the leaf.
     * @param width Width to set.
     */
    void SetLeafXPositionhA(G4int id, G4double width) { fLeafXPositionsA[id] = width; }

    /**
     * @brief Set the position of a specific leaf in bank B.
     * @param id Identifier of the leaf.
     * @param pos Position to set.
     */
    void SetLeafYPositionB(G4int id, G4double pos) { fLeafYPositionsB[id] = pos; }

    /**
     * @brief Set the width of a specific leaf in bank B.
     * @param id Identifier of the leaf.
     * @param width Width to set.
     */
    void SetLeafXPositionhB(G4int id, G4double width) { fLeafXPositionsB[id] = width; }

private:
    /**
     * @brief Print the parameters of the MLC.
     */
    void PrintParametersMLC();

    /**
     * @brief Get the position of a specific leaf in bank A.
     * @param id Identifier of the leaf.
     * @return Position of the leaf.
     */
    G4double GetLeafYPositionA(G4int id) { return fLeafYPositionsA[id]; }

    /**
     * @brief Get the width of a specific leaf in bank A.
     * @param id Identifier of the leaf.
     * @return Width of the leaf.
     */
    G4double GetLeafXPositionsA(G4int id) { return fLeafXPositionsA[id]; }

    /**
     * @brief Get the position of a specific leaf in bank B.
     * @param id Identifier of the leaf.
     * @return Position of the leaf.
     */
    G4double GetLeafYPositionB(G4int id) { return fLeafYPositionsB[id]; }

    /**
     * @brief Get the width of a specific leaf in bank B.
     * @param id Identifier of the leaf.
     * @return Width of the leaf.
     */
    G4double GetLeafXPositionsB(G4int id) { return fLeafXPositionsB[id]; }

    G4int fNLeafs; ///< Number of leafs per bank

    // Leaf positions and widths for bank A
    std::map<G4int, G4double> fLeafYPositionsA; ///< Map of bank A y-axis leaf positions by id
    std::map<G4int, G4double> fLeafXPositionsA; ///< Map of bank A x-axis leaf positions by id

    // Leaf positions and widths for bank B
    std::map<G4int, G4double> fLeafYPositionsB; ///< Map of bank B y-axis leaf positions by id
    std::map<G4int, G4double> fLeafXPositionsB; ///< Map of bank B x-axis leaf positions by id

    G4UImessenger* fMLC80Messenger; ///< Messenger for user interface commands
};

#endif // MLC80_H
