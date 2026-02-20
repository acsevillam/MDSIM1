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


#ifndef MD1_BOPTR_GEOMETRY_BASED_BIASING_H
#define MD1_BOPTR_GEOMETRY_BASED_BIASING_H 1

// Geant4 Headers
#include "G4VBiasingOperator.hh"

// MultiDetector Headers
#include "MD1BOptnSplitOrKillOnBoundary.hh"

class G4GenericMessenger;

namespace MD1
{

class MD1BOptrGeometryBasedBiasing : public G4VBiasingOperator
{
  public:
    MD1BOptrGeometryBasedBiasing();
    virtual ~MD1BOptrGeometryBasedBiasing();

public:
  // ------------------------------
  // Method added for this example:
  // ------------------------------
  MD1BOptnSplitOrKillOnBoundary* GetSplitAndKillOperation() const
  { return fSplitAndKillOperation; }

  // -------------------------
  // Optional from base class:
  // -------------------------
  void StartRun();
  
private:
  // --------------------------
  // Mandatory from base class:
  // --------------------------
  // Used for splitting/killing:
  virtual G4VBiasingOperation*
  ProposeNonPhysicsBiasingOperation( const G4Track* track,
                                     const G4BiasingProcessInterface* callingProcess );

  // Not used here:
  virtual G4VBiasingOperation* 
  ProposeOccurenceBiasingOperation( const G4Track*,
                                    const G4BiasingProcessInterface* ) { return 0; }
  // Not used here:
  virtual G4VBiasingOperation*
  ProposeFinalStateBiasingOperation( const G4Track*,
                                     const G4BiasingProcessInterface* ) { return 0; }

private:
  MD1BOptnSplitOrKillOnBoundary* fSplitAndKillOperation;
  G4int    fSplittingFactor;
  G4double fApplyProbability;
  // Messengers to change the 
  G4GenericMessenger*  fSplittingFactorMessenger;
  G4GenericMessenger* fApplyProbabilityMessenger;

};

}

#endif // MD1_BOPTR_GEOMETRY_BASED_BIASING_H

    
