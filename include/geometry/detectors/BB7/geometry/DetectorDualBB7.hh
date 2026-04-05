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

#ifndef DETECTOR_DUAL_BB7_H
#define DETECTOR_DUAL_BB7_H

// Geant4 Headers
#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4UserLimits.hh"

// MultiDetector Headers
#include "geometry/base/GenericGeometry.hh"
#include "geometry/detectors/BB7/geometry/DetectorSingleBB7.hh"
#include "geometry/detectors/BB7/messenger/DetectorDualBB7Messenger.hh"

/**
 * @class DetectorDualBB7
 * @brief Class representing a dual detector BB7 for Geant4 simulation.
 */
class DetectorDualBB7 : public GenericGeometry {
public:
    /**
     * @brief Constructor for DetectorDualBB7.
     */
    DetectorDualBB7();

    /**
     * @brief Default destructor for DetectorDualBB7.
     */
    ~DetectorDualBB7() override = default;

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

private:
    G4UserLimits* fStepLimit;  ///< Pointer to user step limits
    DetectorSingleBB7* fDetectorSingleBB7; ///< Pointer to a single BB7 detector
    G4UImessenger* fDetectorDualBB7Messenger; ///< Messenger for UI commands

};

#endif // DETECTOR_DUAL_BB7_H
