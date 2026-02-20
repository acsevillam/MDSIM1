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

#ifndef MD1_DETECTOR_CONSTRUCTION_H
#define MD1_DETECTOR_CONSTRUCTION_H

// Standard Headers
#include <vector>
#include <memory>

// Geant4 Headers
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "globals.hh"

class G4LogicalVolume;

namespace MD1 {

class MD1DetectorConstruction : public G4VUserDetectorConstruction {
public:
    MD1DetectorConstruction() = default;
    ~MD1DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    void SetupGeometry(G4LogicalVolume* motherVolume);
    G4LogicalVolume* fBiasingVolume;

};

} // namespace MD1

#endif // MD1_DETECTOR_CONSTRUCTION_H
