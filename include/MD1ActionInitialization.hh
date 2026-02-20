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

#ifndef MD1_ACTION_INITIALIZATION_H
#define MD1_ACTION_INITIALIZATION_H

// Geant4 Headers
#include "G4VUserActionInitialization.hh"

namespace MD1
{

class MD1ActionInitialization : public G4VUserActionInitialization
{
  public:
    MD1ActionInitialization() = default;
    ~MD1ActionInitialization() override = default;

    void BuildForMaster() const override;
    void Build() const override;
};

#endif // MD1_ACTION_INITIALIZATION_H

}
