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

#include "G4DigiManager.hh"
#include "G4UserEventAction.hh"
#include "globals.hh"

namespace MD1
{

class MD1RunAction;

class MD1EventAction : public G4UserEventAction
{
  public:
    explicit MD1EventAction(MD1RunAction* runAction);
    ~MD1EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

  private:
    MD1RunAction* fRunAction;
};

}

#endif // MD1_EVENT_ACTION_H
