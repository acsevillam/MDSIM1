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

#ifndef DETECTOR_SINGLE_BB7_H
#define DETECTOR_SINGLE_BB7_H

// Multidetector header files
#include "GenericGeometry.hh"

// Geant4 header files
#include "globals.hh"
#include "G4UserLimits.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

/**
 * @class DetectorSingleBB7
 * @brief Class representing a single detector BB7 for Geant4 simulation.
 */
class DetectorSingleBB7 : public GenericGeometry {

public:
    /**
     * @brief Constructor for DetectorSingleBB7.
     */
    DetectorSingleBB7();

    /**
     * @brief Default destructor for DetectorSingleBB7.
     */
    ~DetectorSingleBB7() override = default;

    /**
     * @brief Define the materials used in the detector.
     */
    void DefineMaterials() override;

    /**
     * @brief Define the geometrical volumes of the detector.
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

    /**
     * @brief Get the number of strips in the detector.
     * @return Number of strips.
     */
    inline G4int GetNbOfStrips() { return fNbOfStrips; };

private:
    G4UserLimits* fStepLimit; ///< User limits for the step size.
    G4int fNbOfStrips; ///< Number of strips in the detector.
    
};

#endif // DETECTOR_SINGLE_BB7_H
