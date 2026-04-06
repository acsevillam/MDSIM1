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
#include <memory>
#include <vector>

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
    const G4IAEAphspReader* GetParticleSource() const;

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
	    struct ReaderConfiguration {
	        G4IAEAphspReader::EOFPolicy eofPolicy = G4IAEAphspReader::EOFPolicy::Abort;
	        G4double gantryAngle = 0.;
	        G4double collimatorAngle = 0.;
	        G4ThreeVector phspShift = G4ThreeVector();
	        G4int workerIndex = 0;
	        G4int totalWorkers = 1;
	    };

	    void EnsureReadersAreSynchronized();
	    void ApplyReaderConfiguration(G4IAEAphspReader& reader,
	                                  G4IAEAphspReader::EOFPolicy eofPolicy,
	                                  G4double gantryAngle,
	                                  G4double collimatorAngle,
	                                  const G4ThreeVector& phspShift,
	                                  G4int workerIndex,
	                                  G4int totalWorkers) const;
	    G4IAEAphspReader& GetOrCreateReader(std::size_t sourceIndex);

	    std::vector<std::unique_ptr<G4IAEAphspReader>> fReaders;
	    std::vector<G4String> fAppliedSourceFiles;
	    ReaderConfiguration fAppliedReaderConfiguration;
	    G4bool fHasAppliedReaderConfiguration;
	    G4int fAppliedSourceConfigVersion;
	    G4int fAppliedTransformVersion;
	    G4int fAppliedWorkerIndex;
    G4int fAppliedTotalWorkers;
    G4GenericMessenger*  fRotateGantryToMessenger;
    G4GenericMessenger*  fRotateGantryMessenger;
    G4GenericMessenger*  fRotateCollimatorToMessenger;
    G4GenericMessenger*  fRotateCollimatorMessenger;
};

}


#endif // MD1_PRIMARY_GENERATOR_ACTION_H
