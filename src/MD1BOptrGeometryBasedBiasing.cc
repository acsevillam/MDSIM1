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

#include "MD1BOptrGeometryBasedBiasing.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericMessenger.hh"


namespace MD1
{

MD1BOptrGeometryBasedBiasing::MD1BOptrGeometryBasedBiasing()
: G4VBiasingOperator("MD1BOptrGeometryBasedBiasing"),
  fSplittingFactor(4),
  fApplyProbability(1.0)
{
  fSplitAndKillOperation = new MD1BOptnSplitOrKillOnBoundary("splitAndkill");
  
  // -- Define messengers:
  fSplittingFactorMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/biasing/","Biasing control" );
  
  G4GenericMessenger::Command& splittingFactorCmd = 
    fSplittingFactorMessenger->DeclareProperty("setSplittingFactor", fSplittingFactor,
                                "Define the splitting factor." );
  splittingFactorCmd.SetStates(G4State_Idle);
  
  fApplyProbabilityMessenger = 
    new G4GenericMessenger(this, "/MultiDetector1/biasing/","Biasing control" );
  
  G4GenericMessenger::Command& applyProbCmd = 
    fApplyProbabilityMessenger->DeclareProperty("setApplyProbability", fApplyProbability,
                                "Define the probability to apply the splitting/killing." );
  applyProbCmd.SetStates(G4State_Idle);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

MD1BOptrGeometryBasedBiasing::~MD1BOptrGeometryBasedBiasing()
{
  delete fSplitAndKillOperation;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void MD1BOptrGeometryBasedBiasing::StartRun()
{
  fSplitAndKillOperation->SetSplittingFactor ( fSplittingFactor  );
  fSplitAndKillOperation->SetApplyProbability( fApplyProbability );
  G4cout << GetName() << " : starting run with splitting factor = " << fSplittingFactor
         << ", and probability for applying the technique " << fApplyProbability
         << " . " << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VBiasingOperation*
MD1BOptrGeometryBasedBiasing::
ProposeNonPhysicsBiasingOperation( const G4Track*                   /* track */,
                                   const G4BiasingProcessInterface* /* callingProcess */ )
{
  // Here, we always return the split and kill biasing operation:
  return fSplitAndKillOperation;
}

}