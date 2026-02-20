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

#ifndef MD1_PRIMARY_GENERATOR_ACTION2_H
#define MD1_PRIMARY_GENERATOR_ACTION2_H

// Geant4 Headers
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "globals.hh"

class G4GenericMessenger;

namespace MD1
{

class MD1PrimaryGeneratorAction2 : public G4VUserPrimaryGeneratorAction
{
public:

  public:
    MD1PrimaryGeneratorAction2();
    ~MD1PrimaryGeneratorAction2() override;

    // method from the base class
    void GeneratePrimaries(G4Event*) override;    
    inline const G4GeneralParticleSource* GetParticleSource() const {return fParticleSource; } ;
  
  private:
	G4GeneralParticleSource* 	fParticleSource;
};

}


#endif // MD1_PRIMARY_GENERATOR_ACTION2_H
