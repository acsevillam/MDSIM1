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

#pragma once


// Geant4 Headers
#include "G4THitsCollection.hh"
#include "geometry/base/BaseSensitiveDetector.hh"

// MultiDetector Headers
#include "geometry/detectors/basic/sphere/readout/SphereHit.hh"

class G4HCofThisEvent;
class G4Step;

class SphereSensitiveDetector : public BaseSensitiveDetector {
public:
    explicit SphereSensitiveDetector(const G4String& name);
    ~SphereSensitiveDetector() override;

    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    SphereHitsCollection* fHitsCollection;
    G4int fHCID;
};

