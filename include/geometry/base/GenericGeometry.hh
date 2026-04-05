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

#ifndef MD1_GENERIC_GEOMETRY_H
#define MD1_GENERIC_GEOMETRY_H

// Standard Headers
#include <map>
#include <string>

// Geant4 Headers
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

/**
 * @class GenericGeometry
 * @brief Abstract base class for defining generic geometry in Geant4 simulations.
 */
class GenericGeometry {
public:
    /**
     * @brief Constructor for GenericGeometry.
     */
    GenericGeometry();

    /**
     * @brief Default destructor for GenericGeometry.
     */
    virtual ~GenericGeometry();

    /**
     * @brief Define the materials used in the geometry.
     */
    virtual void DefineMaterials() = 0;

    /**
     * @brief Define the geometrical volumes of the geometry.
     */
    virtual void DefineVolumes() = 0;

    /**
     * @brief Add geometry to the mother volume.
     * @param motherVolume Logical volume of the mother.
     * @param copyNo Copy number for the geometry.
     */
    virtual void AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) = 0;

    /**
     * @brief Add geometry to the mother volume with a translation and rotation.
     * @param motherVolume Logical volume of the mother.
     * @param position Translation vector.
     * @param rotation Rotation matrix.
     * @param copyNo Copy number for the geometry.
     */
    virtual void AddGeometry(G4LogicalVolume* motherVolume, const G4ThreeVector& position, G4RotationMatrix* rotation, G4int copyNo) = 0;

    /**
     * @brief Add geometry to the mother volume with a transformation.
     * @param motherVolume Logical volume of the mother.
     * @param transformation Transformation to apply.
     * @param copyNo Copy number for the geometry.
     */
    virtual void AddGeometry(G4LogicalVolume* motherVolume, G4Transform3D* transformation, G4int copyNo) = 0;

    /**
     * @brief Rotate the geometry to the specified angles.
     * @param copyNo Copy number of the geometry.
     * @param theta Angle theta for rotation.
     * @param phi Angle phi for rotation.
     * @param psi Angle psi for rotation.
     */
    void RotateTo(const G4int& copyNo, const G4double& theta, const G4double& phi, const G4double& psi);

    /**
     * @brief Rotate the geometry around the X-axis.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateX(const G4int& copyNo, const G4double& delta);

    /**
     * @brief Rotate the geometry around the Y-axis.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateY(const G4int& copyNo, const G4double& delta);

    /**
     * @brief Rotate the geometry around the Z-axis.
     * @param copyNo Copy number of the geometry.
     * @param delta Rotation angle.
     */
    void RotateZ(const G4int& copyNo, const G4double& delta);

    /**
     * @brief Add the geometry to the specified volume.
     * @param volumeName Name of the volume.
     * @param copyNo Copy number of the geometry.
     */
    void AddGeometryTo(const G4String& volumeName, const G4int& copyNo);

    /**
     * @brief Remove the geometry.
     * @param copyNo Copy number of the geometry.
     */
    void RemoveGeometry(const G4int& copyNo);

    /**
     * @brief Translate the geometry to the specified position.
     * @param copyNo Copy number of the geometry.
     * @param position Position to translate to.
     */
    void TranslateTo(const G4int& copyNo, const G4ThreeVector& position);

    /**
     * @brief Translate the geometry by the specified delta.
     * @param copyNo Copy number of the geometry.
     * @param delta Delta to translate by.
     */
    void Translate(const G4int& copyNo, const G4ThreeVector& delta);

    /**
     * @brief Apply transformation to the geometry .
     * @param copyNo Copy number of the geometry.
     * @param transformation 3D transformation.
     */
    void ApplyTransformation(const G4int& copyNo, const G4Transform3D& transformation);

    /**
     * @brief Get the detector origin.
     * @return Detector origin as a G4ThreeVector.
     */
    inline G4ThreeVector GetDetectorOrigin() const { return det_origin; }

    /**
     * @brief Get the material by name.
     * @param matName Name of the material.
     * @return Pointer to the material.
     */
    G4Material* GetMaterial(const G4String& matName);

    /**
     * @brief Get the geometrical volume by name.
     * @param geoVolumeName Name of the geometrical volume.
     * @return Pointer to the geometrical volume.
     */
    G4VSolid* GetGeoVolume(const G4String& geoVolumeName);

    /**
     * @brief Get the logical volume by name.
     * @param logVolumeName Name of the logical volume.
     * @return Pointer to the logical volume.
     */
    G4LogicalVolume* GetLogVolume(const G4String& logVolumeName);

    /**
     * @brief Get the physical volume by copy number.
     * @param physVolumeName Name of the physical volume.
     * @return Pointer to the physical volume.
     */
    G4VPhysicalVolume* GetPhysVolume(const G4String& physVolumeName);

    /**
     * @brief Get the frame physical volume by copy number.
     * @param copyNo Copy number of the physical volume.
     * @return Pointer to the physical volume.
     */
    G4VPhysicalVolume* GetFrameVolume(const G4int& copyNo);

    /**
     * @brief Creates a new RotMatrix on the heap (using "new") and copies its argument into it.
     * @param rotation Rotation matrix.
     * @return Pointer to the new rotation matrix.
     */
    static G4RotationMatrix* NewPtrRotMatrix(const G4RotationMatrix &RotMat);

    /**
     * @brief Update the geometry.
     */
    void UpdateGeometry();

    G4bool HasPlacementRequests() const;
    G4bool HasAssembledGeometry() const;
    void AssembleRequestedGeometries();

protected:
    G4RotationMatrix* EnsureRotationMatrix(const G4int& copyNo);

    G4String geometryName; ///< Name of the geometry.

    // Material definitions
    std::map<G4String, G4Material*> detMat; ///< Map of materials by name

    // Geometrical volumes
    std::map<G4String, G4VSolid*> detGeo; ///< Map of geometrical volumes by name

    // Logical volumes
    std::map<G4String, G4LogicalVolume*> detLog; ///< Map of logical volumes by name

    // Physical volumes
    std::map<G4String, G4VPhysicalVolume*> detPhys; ///< Map of physical volumes by copy number

    // Frame physical volumes
    std::map<G4int, G4VPhysicalVolume*> detFrame; ///< Map of frame physical volumes by copy number

    // Detector origin
    G4ThreeVector det_origin; ///< Origin of the detector

    // Detectors position
    std::map<G4int, G4ThreeVector> detPosition; ///< Map of detector positions by copy number

    // Detectors rotation matrix
    std::map<G4int, G4RotationMatrix*> detRotMat; ///< Map of rotation matrices by copy number

    // Requested mother logical volumes for detector placements
    std::map<G4int, G4String> detMotherVolumeNames; ///< Map of requested mother logical volumes by copy number

    G4bool fAreVolumensDefined; ///< Flag indicating if the detector volumes are defined
    G4bool fAreVolumensAssembled; ///< Flag indicating if the detector is assembled
};

#endif // MD1_GENERIC_GEOMETRY_H
