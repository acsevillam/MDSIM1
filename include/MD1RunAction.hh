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

#ifndef MD1_RUN_ACTION_H
#define MD1_RUN_ACTION_H

#include <memory>
#include <vector>

// Geant4 Headers
#include "G4Accumulable.hh"
#include "G4StatDouble.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserRunAction.hh"
#include "globals.hh"

// MultiDetector Headers
#include "geometry/base/DetectorModule.hh"

class G4DigiManager;

namespace MD1 {

class MD1RunActionMessenger;

struct DetectorRuntimeEntry {
    DetectorModule* detector = nullptr;
    std::unique_ptr<DetectorRuntimeState> runtimeState;
};

struct ActiveDetectorRuntime {
    DetectorModule* detector = nullptr;
    DetectorRuntimeState* runtimeState = nullptr;
};

class MD1RunAction : public G4UserRunAction {
  public:
    MD1RunAction();
    ~MD1RunAction() override;

    // Called at the beginning of each run
    void BeginOfRunAction(const G4Run*) override;

    // Called at the end of each run
    void EndOfRunAction(const G4Run*) override;

    // Method to create histograms
    void CreateHistos();

    // Method to create N-Tuples
    void CreateNTuples();

    // Method to set monitor units
    void SetMU(G4int MU);
    void SetScaleFactorMU(G4double scaleFactorMU);
    void SetScaleFactorMUError(G4double scaleFactorMUError);
    void RegisterDetectorDigitizers(G4DigiManager* digiManager);
    const std::vector<ActiveDetectorRuntime>& GetActiveDetectors() const { return fActiveDetectors; }


  private:
    void BuildActiveDetectors();

    // Accumulables for total energy deposition and its square
    G4int fSimulatedMU = 1;
    G4double fScaleFactorMU = 1/2.370031e-13; // [cGy/ev -> cGy/UM (dmax)] = 1/2.370031e-13
    G4double fScaleFactorMUError = fScaleFactorMU*0.0026279; // absolute uncertainty on fScaleFactorMU = 6.22820010561e-16 cGy (0.26279 %)

    // Run action messenger
    std::unique_ptr<MD1RunActionMessenger> fRunActionMessenger;
    std::vector<DetectorRuntimeEntry> fDetectorRuntimeEntries;
    std::vector<ActiveDetectorRuntime> fActiveDetectors;
};

} // namespace MD1

#endif // MD1_RUN_ACTION_H
