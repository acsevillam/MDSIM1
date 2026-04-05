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

// Geant4 Headers
#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4StatDouble.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "analysis/MD1StatAccumulable.hh"
#include "MD1DetectorConstruction.hh"
#include "MD1RunActionMessenger.hh"

namespace MD1 {

struct DetectorRunAccumulables {
    G4String name;
    G4Accumulable<G4int> events;
    MD1StatAccumulable totalEdep;
    MD1StatAccumulable collectedCharge;
    MD1StatAccumulable dose;
    MD1StatAccumulable estimatedDoseToWater;

    explicit DetectorRunAccumulables(const G4String& detectorName)
        : name(detectorName),
          events(0),
          totalEdep(detectorName + "_Edep"),
          collectedCharge(detectorName + "_Charge"),
          dose(detectorName + "_Dose"),
          estimatedDoseToWater(detectorName + "_EstimatedDoseToWater") {}
};

class MD1RunAction : public G4UserRunAction {
  public:
    MD1RunAction();
    ~MD1RunAction() override = default;

    // Called at the beginning of each run
    void BeginOfRunAction(const G4Run*) override;

    // Called at the end of each run
    void EndOfRunAction(const G4Run*) override;

    // Method to create histograms
    void CreateHistos();

    // Method to create N-Tuples
    void CreateNTuples();

    // Method to count energy deposition events
    void CountEdepEvent() { fEDepEvents += 1; }

    // Method to add total energy deposition
    void AddTotalEdep(G4double Edep) { fTotalEdep.Fill(Edep); }

    // Method to add total collected charge
    void AddTotalCollectedCharge(G4double CollectedCharge) { fCollectedCharge.Fill(CollectedCharge); }

    // Method to add total dose
    void AddTotalDose(G4double dose) { fDose.Fill(dose); }
    void AddTotalEstimatedDoseToWater(G4double dose) { fEstimatedDoseToWater.Fill(dose); }

    void AddDetectorTotals(const G4String& detectorName,
                           G4double edep,
                           G4double collectedCharge,
                           G4double dose,
                           G4double estimatedDoseToWater);

    // Method to set monitor units
    void SetMU(G4int MU) { fSimulatedMU = MU; }


  private:
    // Accumulable for counting energy deposition events
    G4Accumulable<G4int> fEDepEvents;

    // Statistical accumulables for energy deposition, collected charge and dose
    MD1StatAccumulable fTotalEdep;
    MD1StatAccumulable fCollectedCharge;
    MD1StatAccumulable fDose;
    MD1StatAccumulable fEstimatedDoseToWater;

    // Accumulables for total energy deposition and its square
    G4int fSimulatedMU = 1; //
    G4double fScaleFactorMU = 1; // [cGy/ev -> cGy/UM (dmax)] =  1/2.385e-13

    // Run action messenger
    G4UImessenger* 	 				fRunActionMessenger ;
    std::vector<DetectorRunAccumulables> fDetectorAccumulables;
};

} // namespace MD1

#endif // MD1_RUN_ACTION_H
