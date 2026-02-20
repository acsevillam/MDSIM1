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

#include "Randomize.hh"
#include "MD1BOptnSplitOrKillOnBoundary.hh"


namespace MD1
{

MD1BOptnSplitOrKillOnBoundary::MD1BOptnSplitOrKillOnBoundary(G4String name)
: G4VBiasingOperation(name),
  fParticleChange(),
  fParticleChangeForNothing()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

MD1BOptnSplitOrKillOnBoundary::~MD1BOptnSplitOrKillOnBoundary()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double MD1BOptnSplitOrKillOnBoundary::
DistanceToApplyOperation( const G4Track*,
                          G4double,
                          G4ForceCondition* condition)
{
  // -- return "infinite" distance for interaction, but asks for GenerateBiasingFinalState
  // -- being called anyway at the end of the step, by returning the "Forced" condition
  // -- flag.
  *condition = Forced;
  return DBL_MAX;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VParticleChange* 
MD1BOptnSplitOrKillOnBoundary::GenerateBiasingFinalState( const G4Track* track,
                                                           const G4Step*  step )
{
  
  // Check if step is limited by the geometry: as we attached the biasing operator
  // to the absorber layer, this volume boundary is the one of the absorber.
  // (check of current step # of track is inelegant, but is to fix a "feature"
  // that a cloned track can wrongly be seen in the wrong volume, because of numerical
  // precision issue. In this case it makes a tiny step, which should be disregarded).
  if ( ( step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary ) &&
       ( track->GetCurrentStepNumber() != 1 ) )
    {
      
      // -- Before deciding for killing or splitting, we make decision on applying
      // -- the technique or not:
      G4double trial = G4UniformRand(); // -- Note: G4UniformRand() is thread-safe
                                        // -- engine for random numbers
      if ( trial <= fApplyProbability )
        {
          // -- Get z component of track, to see if it moves forward or backward:
          G4double pz = track->GetMomentum().z();
          
          if ( pz < 0.0 )
            {
              // -------------------------------------------------
              // Here, we are moving "forward". We do "splitting":
              // -------------------------------------------------
              
              // Get track weight:
              G4double initialWeight = track->GetWeight();
              // Define the tracks weight:
              G4double weightOfTrack = initialWeight/fSplittingFactor;
              
              // The "particle change" is the object to be used to communicate to
              // the tracking the update of the primary state and/or creation
              // secondary tracks.
              fParticleChange.Initialize(*track);
              
              // ask currect track weight to be changed to new value:
              fParticleChange.ProposeParentWeight( weightOfTrack );
              
              // Now make clones of this track (this is the actual splitting):
              // we will then have the primary and N-1 clones of it, hence the
              // splitting by a factor N:
              fParticleChange.SetNumberOfSecondaries( fSplittingFactor-1 );
              for ( G4int iSplit = 1 ; iSplit <  fSplittingFactor ; iSplit++ )
                {
                  G4Track* clone = new G4Track( *track );
                  clone->SetWeight( weightOfTrack );
                  fParticleChange.AddSecondary( clone );
                }
              fParticleChange.SetSecondaryWeightByProcess(true); // -- tricky
              // -- take it as is ;) [though not necessary here, put for safety]

              // this new final state is returned to the tracking;
              return &fParticleChange;
              
            }
          
          else
            
            {
              // --------------------------------------------------------------
              // Here, we are moving backward. We do killing, playing a russian
              // roulette, killing 1/fSplittingFactor of the tracks in average:
              // --------------------------------------------------------------
              
              // Get track weight:
              G4double initialWeight = track->GetWeight();
              
              // The "particle change" is the object to be used to communicate to
              // the tracking the update of the primary state and/or creation
              // secondary tracks.
              fParticleChange.Initialize(*track);
              
              // Shoot a random number (in ]0,1[ segment):
              G4double random = G4UniformRand();
              
              // Decide to kill, keeping 1/fSplittingFactor of tracks:
              G4double survivingProbability = 1.0/fSplittingFactor;
              if ( random > survivingProbability )
                {
                  // We ask for the the track to be killed:
                  fParticleChange.ProposeTrackStatus(fStopAndKill);
                }
              else
                {
                  // In this case, the track survives. We change its weight
                  // to conserve weight among killed and survival tracks:
                  fParticleChange.ProposeParentWeight( initialWeight*fSplittingFactor );
                }
              
              // this new final state is returned to the tracking;
              return &fParticleChange;
            }
        }  // -- end of : if ( trial > probaForApplying )
    }      // -- end of : if ( ( step->GetPostStepPoint()->GetStepStatus() ==
           //                                                       fGeomBoundary ) ...
 

  // Here, the step was not limited by the geometry (but certainly by a physics
  // process). We do nothing: ie we make no change to the current track.
  fParticleChangeForNothing.Initialize(*track);
  return &fParticleChangeForNothing;
  
}

}