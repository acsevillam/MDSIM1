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

#ifndef MD1_PRIMARY_GENERATOR_ACTION_H
#define MD1_PRIMARY_GENERATOR_ACTION_H

// Geant4 Headers
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4IAEAphspReader.hh"
#include "globals.hh"

class G4GenericMessenger;

namespace MD1
{

class MD1PrimaryGeneratorAction1 : public G4VUserPrimaryGeneratorAction
{
public:

    MD1PrimaryGeneratorAction1();
    ~MD1PrimaryGeneratorAction1() override;

    // method from the base class
    void GeneratePrimaries(G4Event*) override;

	// method to access particle gun
    inline const G4IAEAphspReader* GetParticleSource() const {return fIAEAReader; } ;

    /**
     * @brief Rotate the gantry around the Y-axis to the specified angle.
     * @param angle Rotation angle.
     */
    void RotateGantryTo(const G4double& angle);

    /**
     * @brief Rotate the gantry around the Y-axis.
     * @param delta Rotation angle.
     */
    void RotateGantry(const G4double& delta);

    /**
     * @brief  Rotate the collimator around the Z'-axis to the specified angle.
     * @param angle Rotation angle.
     */
    void RotateCollimatorTo(const G4double& delta);

    /**
     * @brief Rotate the collimator around the Z'-axis.
     * @param delta Rotation angle.
     */
    void RotateCollimator(const G4double& delta);

private:
    G4IAEAphspReader* fIAEAReader;
    G4String fFileName;
    G4double fGantryAngle; ///< Gantry angle around X-axis
    G4double fCollimatorAngle; ///< Collimator angle around Z'-axis
    G4GenericMessenger*  fRotateGantryToMessenger;
    G4GenericMessenger*  fRotateGantryMessenger;
    G4GenericMessenger*  fRotateCollimatorToMessenger;
    G4GenericMessenger*  fRotateCollimatorMessenger;
};

}


#endif // MD1_PRIMARY_GENERATOR_ACTION_H
