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

#ifndef CLINAC_TRUE_BEAM_H
#define CLINAC_TRUE_BEAM_H

// Geant4 Headers
#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

// MultiDetector Headers
#include "geometry/base/GenericGeometry.hh"
#include "geometry/beamline/ClinacTrueBeamMessenger.hh"

/**
 * @class ClinacTrueBeam
 * @brief Class representing a Clinac TrueBeam accelerator for Geant4 simulation.
 */
class ClinacTrueBeam : public GenericGeometry {
public:
    /**
     * @brief Constructor for ClinacTrueBeam.
     */
    ClinacTrueBeam();

    /**
     * @brief Default destructor for ClinacTrueBeam.
     */
    ~ClinacTrueBeam() override = default;

    /**
     * @brief Define the materials used in the Clinac TrueBeam.
     */
    void DefineMaterials() override;

    /**
     * @brief Define the geometrical volumes of the Clinac TrueBeam.
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
     * @brief Set the jaw 1X aperture.
     * @param copyNo Copy number of the geometry.
     * @param value Aperture value.
     */
    void SetJaw1X(const G4int& copyNo, const G4double& value);

    /**
     * @brief Set the jaw 1Y aperture.
     * @param copyNo Copy number of the geometry.
     * @param value Aperture value.
     */
    void SetJaw1Y(const G4int& copyNo, const G4double& value);

    /**
     * @brief Set the jaw 2X aperture.
     * @param copyNo Copy number of the geometry.
     * @param value Aperture value.
     */
    void SetJaw2X(const G4int& copyNo, const G4double& value);

    /**
     * @brief Set the jaw 2Y aperture.
     * @param copyNo Copy number of the geometry.
     * @param value Aperture value.
     */
    void SetJaw2Y(const G4int& copyNo, const G4double& value);

    /**
     * @brief Rotate the gantry around the Y-axis to the specified angle.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateGantryTo(const G4int& copyNo, const G4double& angle);

    /**
     * @brief Rotate the gantry around the Y-axis.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateGantry(const G4int& copyNo, const G4double& delta);

    /**
     * @brief Rotate the collimator around the Z'-axis to the specified angle.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateCollimatorTo(const G4int& copyNo, const G4double& angle);

    /**
     * @brief Rotate the collimator around the Z'-axis.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateCollimator(const G4int& copyNo, const G4double& delta);

private:

    /**
     * @brief Constructs the accelerator.
     * @param motherVolume Logical volume of the mother.
     * @param copyNo Copy number for the geometry.
     */
    void ConstructAccelerator(G4LogicalVolume* motherVolume, G4int copyNo);

    /**
     * @brief Constructs the bias volume.
     * @param motherVolume Logical volume of the mother.
     */
    void ConstructBiasVolume(G4LogicalVolume* motherVolume, G4int copyNo);

    /**
     * @brief Constructs the jaws along the X-axis.
     * @param motherVolume Logical volume of the mother.
     */
    void ConstructJawsX(G4LogicalVolume* motherVolume, G4int copyNo);

    /**
     * @brief Constructs the jaws along the Y-axis.
     * @param motherVolume Logical volume of the mother.
     */
    void ConstructJawsY(G4LogicalVolume* motherVolume, G4int copyNo);

    /**
     * @brief Constructs the base plate.
     * @param motherVolume Logical volume of the mother.
     */
    void ConstructBasePlate(G4LogicalVolume* motherVolume, G4int copyNo);

    /**
     * @brief Constructs the multi-leaf collimator (MLC).
     * @param motherVolume Logical volume of the mother.
     */
    void ConstructMLC(G4LogicalVolume* motherVolume, G4int copyNo);

    G4double fJaw1XAperture; ///< Jaw aperture value of Jaw 1 along X-axis
    G4double fJaw2XAperture; ///< Jaw aperture value of Jaw 2 along X-axis
    G4double fJaw1YAperture; ///< Jaw aperture value of Jaw 1 along Y-axis
    G4double fJaw2YAperture; ///< Jaw aperture value of Jaw 2 along Y-axis
    G4double fJawXPenumbraCorrection; ///< Jaw aperture correction for penumbra along X-axis
    G4double fJawYPenumbraCorrection; ///< Jaw aperture correction for penumbra along X-axis
    G4double fGantryAngle; ///< Gantry angle around X-axis
    G4double fCollimatorAngle; ///< Collimator angle around Z'-axis
    G4double fIsocenterDistance; ///< Isocenter distance
    G4double fReferenceDistance; ///< Reference distance
    G4ThreeVector fJaw1XInitialPos; ///< Initial position of Jaw 1 along X-axis
    G4ThreeVector fJaw2XInitialPos; ///< Initial position of Jaw 2 along X-axis
    G4ThreeVector fJaw1YInitialPos; ///< Initial position of Jaw 1 along Y-axis
    G4ThreeVector fJaw2YInitialPos; ///< Initial position of Jaw 2 along Y-axis
    G4UserLimits* fStepLimit; ///< Pointer to user step limits
    G4RotationMatrix* fRot; ///< Pointer to rotation matrix
    G4UImessenger* fClinacTrueBeamMessenger; ///< TrueBeam messenger
};

#endif // CLINAC_TRUE_BEAM_H