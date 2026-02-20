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

#ifndef MD1_EVENT_ACTION_H
#define MD1_EVENT_ACTION_H

// Geant4 Headers
#include "G4UserEventAction.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4THitsMap.hh"
#include "G4DigiManager.hh"

// MultiDetector Headers
#include "BB7Hit.hh"
#include "BB7Digit.hh"

namespace MD1
{

class MD1RunAction;

class MD1EventAction : public G4UserEventAction
{
  public:
	MD1EventAction(MD1RunAction* runAction);
    ~MD1EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    BB7HitsCollection* GetHitsCollection(G4int hcID,const G4Event* event) const;
    BB7DigitsCollection* GetDigitsCollection(G4int dcID, G4DigiManager* digiManager) const;

  private:

    MD1RunAction*			fRunAction;
    G4int             fHCID = -1;
    G4int             fDCID = -1;
};

}

#endif // MD1_EVENT_ACTION_H

