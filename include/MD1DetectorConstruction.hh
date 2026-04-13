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
class GenericGeometry;
class PhantomWaterBox;
class PhantomWaterTube;

namespace MD1 {

class MD1DetectorConstructionMessenger;

enum class WaterPhantomType {
    WaterBox,
    WaterTube
};

class MD1DetectorConstruction : public G4VUserDetectorConstruction {
public:
    MD1DetectorConstruction();
    ~MD1DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
    void SetDefaultWaterPhantomType(const G4String& phantomType);
    G4String GetDefaultWaterPhantomTypeName() const;

private:
    void SetupGeometry(G4LogicalVolume* motherVolume);
    GenericGeometry* GetActiveWaterPhantom() const;
    G4String GetActiveWaterPhantomLogicalVolumeName() const;
    G4double GetActiveWaterPhantomHeight() const;

    G4LogicalVolume* fBiasingVolume = nullptr;
    std::unique_ptr<ClinacTrueBeam> fClinacTrueBeam;
    WaterPhantomType fDefaultWaterPhantomType = WaterPhantomType::WaterBox;
    std::unique_ptr<PhantomWaterBox> fPhantomWaterBox;
    std::unique_ptr<PhantomWaterTube> fPhantomWaterTube;
    std::unique_ptr<MD1DetectorConstructionMessenger> fMessenger;
};

} // namespace MD1

#endif // MD1_DETECTOR_CONSTRUCTION_H
