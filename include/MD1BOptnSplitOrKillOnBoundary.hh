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

#ifndef MD1_BOPTR_SPLIT_OR_KILL_ON_BOUNDARY_H
#define MD1_BOPTR_SPLIT_OR_KILL_ON_BOUNDARY_H 1

// Geant4 Headers
#include "G4VBiasingOperation.hh"
#include "G4ParticleChange.hh"
#include "G4ParticleChangeForNothing.hh"

class G4LogicalVolume;

namespace MD1
{

class MD1BOptnSplitOrKillOnBoundary : public G4VBiasingOperation {
public:
  // -- Constructor :
  MD1BOptnSplitOrKillOnBoundary(G4String name);
  // -- destructor:
  virtual ~MD1BOptnSplitOrKillOnBoundary();

public:
  // ----------------------------------------------
  // -- Methods from G4VBiasingOperation interface:
  // ----------------------------------------------
  // -- Unused:
  virtual const G4VBiasingInteractionLaw*
  ProvideOccurenceBiasingInteractionLaw( const G4BiasingProcessInterface*,
                                         G4ForceCondition&                 ) {return 0;}
  virtual G4VParticleChange*                            
  ApplyFinalStateBiasing               ( const G4BiasingProcessInterface*,
                                         const G4Track*,
                                         const G4Step*,
                                         G4bool&                           ) {return 0;}

  // -- Used methods ("non-physics biasing methods"):
  // ------------------------------------------------
  // -- Method to return the distance or the condition under which
  // -- requesting the biasing. The "condition" flag will be indeed
  // -- used to apply the operation on the geometry boundary.
  virtual G4double
  DistanceToApplyOperation              ( const G4Track*,
                                          G4double,
                                          G4ForceCondition* condition      );
  // -- Method the generate the final state, ie, either the final states
  // -- corresponding to the splitting or killing cases:
  virtual G4VParticleChange* 
  GenerateBiasingFinalState             ( const G4Track*,
                                          const G4Step*                    );

  // -- Specific to this example:
  // ----------------------------
  // -- Set the integer splitting factor (should be >= 2):
  // -- This determines also the killing probability : 1/splittingFactor
  void SetSplittingFactor( G4int splittingFactor )
  { fSplittingFactor = splittingFactor; }
  // -- An "invention" (?) of this example: defines a probability
  // -- to apply the splitting(killing) techniques:
  // --   - If proba == 1, the technique is applied normally
  // --   - if proba < 1 (say 70%) then the splitting(killing) is
  // --     applied in only 70% of the cases.
  // -- This "trick" is useful when the desired splitting factor
  // -- would rather be a non-integer value, ie in case where:
  // --    splittingFactor = k   is (a bit) too small
  // --    splittingFactor = k+1 is (a bit) too large
  // -- then the proba can allow to tune an in between application
  // -- of the technique.
  // -- Specially useful when wanting to split between "1 and 2".
  void SetApplyProbability( G4double proba )
  { fApplyProbability = proba; }
  
  G4int    GetSplittingFactor()  const { return fSplittingFactor;  }
  G4double GetApplyProbability() const { return fApplyProbability; }
  
private:
  G4ParticleChange            fParticleChange;
  G4ParticleChangeForNothing  fParticleChangeForNothing;
  G4int    fSplittingFactor;
  G4double fApplyProbability;
};

}
#endif // MD1_BOPTR_SPLIT_OR_KILL_ON_BOUNDARY_H
