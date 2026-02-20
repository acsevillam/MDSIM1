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

// Geant4 Headers
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4Threading.hh"
#include "G4AutoDelete.hh"
#include "G4AutoLock.hh"
#include "G4GenericMessenger.hh"

// MultiDetector Headers
#include "MD1PrimaryGeneratorAction2.hh"
#include "MD1Control.hh"
#include "G4GeneralParticleSource.hh"

// Espacio de nombres MD1
namespace MD1 {

  MD1PrimaryGeneratorAction2::MD1PrimaryGeneratorAction2()
    : G4VUserPrimaryGeneratorAction()
  {
	G4cout<<"01 - Primary Generator action have started !!!"<<G4endl;
	fParticleSource = new G4GeneralParticleSource();

  }

  MD1PrimaryGeneratorAction2::~MD1PrimaryGeneratorAction2()
  {}

  void MD1PrimaryGeneratorAction2::GeneratePrimaries(G4Event* anEvent)
  {
   fParticleSource->GeneratePrimaryVertex(anEvent);
  }

}