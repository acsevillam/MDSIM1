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

#include <memory>

#include "G4LogicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4LogicalVolume;
class ClinacTrueBeam;
class PhantomWaterBox;

namespace MD1 {

class MD1DetectorConstruction : public G4VUserDetectorConstruction {
public:
    MD1DetectorConstruction();
    ~MD1DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

private:
    void SetupGeometry(G4LogicalVolume* motherVolume);

    G4LogicalVolume* fBiasingVolume = nullptr;
    std::unique_ptr<ClinacTrueBeam> fClinacTrueBeam;
    std::unique_ptr<PhantomWaterBox> fPhantomWaterBox;
};

} // namespace MD1

#endif // MD1_DETECTOR_CONSTRUCTION_H
