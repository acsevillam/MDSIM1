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
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1Control.hh"
#include "G4IAEAphspReader.hh"

// Espacio de nombres MD1
namespace MD1 {
  // Declarar la variable thread-local dentro del espacio de nombres MD1
  G4ThreadLocal G4IAEAphspReader* fIAEAReader = nullptr;
  G4Mutex aMutex=G4MUTEX_INITIALIZER;

  MD1PrimaryGeneratorAction1::MD1PrimaryGeneratorAction1()
    : G4VUserPrimaryGeneratorAction()
  {

  fRotateGantryToMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/Clinac/" );

  G4GenericMessenger::Command& fRotateGantryToCmd = 
  fRotateGantryToMessenger->DeclareMethodWithUnit("rotateGantryPhSpTo", "deg", &MD1PrimaryGeneratorAction1::RotateGantryTo,
                                "Rotate the clinac phase space around X-axis to the specified angle." );
  fRotateGantryToCmd.SetStates(G4State_PreInit, G4State_Idle);

  fRotateGantryMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/Clinac/" );

  G4GenericMessenger::Command& fRotateGantryCmd = 
  fRotateGantryMessenger->DeclareMethodWithUnit("rotateGantryPhSp", "deg", &MD1PrimaryGeneratorAction1::RotateGantry,
                                "Rotate the clinac phase space around X-axis." );
  fRotateGantryCmd.SetStates(G4State_PreInit, G4State_Idle);

  fRotateCollimatorMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/Clinac/" );

  G4GenericMessenger::Command& fRotateCollimatorToCmd = 
  fRotateCollimatorMessenger->DeclareMethodWithUnit("rotateCollimatorPhSpTo", "deg", &MD1PrimaryGeneratorAction1::RotateCollimatorTo,
                                "Rotate the clinac phase space around Z'-axis to the specified angle." );
  fRotateCollimatorToCmd.SetStates(G4State_PreInit, G4State_Idle);

  fRotateCollimatorMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/Clinac/" );

  G4GenericMessenger::Command& fRotateCollimatorCmd = 
  fRotateCollimatorMessenger->DeclareMethodWithUnit("rotateCollimatorPhSp", "deg", &MD1PrimaryGeneratorAction1::RotateCollimator,
                                "Rotate the clinac phase space around Z'-axis." );
  fRotateCollimatorCmd.SetStates(G4State_PreInit, G4State_Idle);

    G4cout << "MD1PrimaryGeneratorAction1 constructor called on thread " << G4Threading::G4GetThreadId() << G4endl;
    G4AutoLock l(&aMutex);
    if (!fIAEAReader)
    {
	    fFileName = MD1::MD1Control::GetInstance()->GetPhspFileName(); 
      fIAEAReader = new G4IAEAphspReader(fFileName);

		G4ThreeVector psfShift(0., 0., 0.*cm);
		fIAEAReader->SetGlobalPhspTranslation(psfShift);
		fIAEAReader->SetIsocenterPosition(psfShift);
		G4cout << "IAEA Reader initialized with file: " << fFileName << " on thread " << G4Threading::G4GetThreadId() << G4endl;
		G4AutoDelete::Register(fIAEAReader);
    }
  }

  MD1PrimaryGeneratorAction1::~MD1PrimaryGeneratorAction1()
  {}

  void MD1PrimaryGeneratorAction1::GeneratePrimaries(G4Event* anEvent)
  {
   G4AutoLock l(&aMutex);
   fIAEAReader->GeneratePrimaryVertex(anEvent);

  }

void MD1PrimaryGeneratorAction1::RotateGantryTo(const G4double& angle) {
  fIAEAReader->SetGantryAngle(angle);
}

void MD1PrimaryGeneratorAction1::RotateGantry(const G4double& delta) {
  fGantryAngle += delta;
  fIAEAReader->SetGantryAngle(fGantryAngle);
}

void MD1PrimaryGeneratorAction1::RotateCollimatorTo(const G4double& angle) {
  fIAEAReader->SetCollimatorAngle(angle);
}

void MD1PrimaryGeneratorAction1::RotateCollimator(const G4double& delta) {
  fCollimatorAngle += delta;
  fIAEAReader->SetCollimatorAngle(fCollimatorAngle);
}
}