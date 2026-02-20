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

// MultiDetector Headers
#include "MD1ActionInitialization.hh"
#include "MD1RunAction.hh"
#include "MD1EventAction.hh"
#include "MD1Control.hh"
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1PrimaryGeneratorAction2.hh"

namespace MD1
{

void MD1ActionInitialization::BuildForMaster() const
{
	auto runAction = new MD1RunAction;
  SetUserAction(runAction);
}

void MD1ActionInitialization::Build() const
{
  auto control = MD1Control::GetInstance();
  auto primaryGeneratorType = control->GetPrimaryGeneratorType();
  switch (primaryGeneratorType)
  {
  case 1:
    SetUserAction(new MD1PrimaryGeneratorAction1);
    break;
  case 2:
    SetUserAction(new MD1PrimaryGeneratorAction2);
    break;
  
  default:
    SetUserAction(new MD1PrimaryGeneratorAction1);
    break;
  }

  auto runAction = new MD1RunAction; 
  SetUserAction(runAction); 

  auto eventAction = new MD1EventAction(runAction);
  SetUserAction(eventAction);

}  

}
