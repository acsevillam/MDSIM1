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
#include "globals.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "MD1DetectorConstruction.hh"
#include "MD1RunActionMessenger.hh"

namespace MD1 {

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
    void AddTotalEdep(G4double Edep) { fTotalEdep += Edep; fTotalEdep2 += Edep * Edep; }

    // Method to add total collected charge
    void AddTotalCollectedCharge(G4double CollectedCharge) { fCollectedCharge += CollectedCharge; fCollectedCharge2 += CollectedCharge * CollectedCharge; }

    // Method to add total dose
    void AddTotalDose(G4double Dose) { fDose += Dose; fDose2 += Dose * Dose; }

    // Method to set monitor units
    void SetMU(G4int MU) { fSimulatedMU = MU; }


  private:
    // Accumulable for counting energy deposition events
    G4Accumulable<G4int> fEDepEvents;

    // Accumulables for total energy deposition and its square
    G4Accumulable<G4double> fTotalEdep;
    G4Accumulable<G4double> fTotalEdep2;

    // Accumulables for total collected charge and its square
    G4Accumulable<G4double> fCollectedCharge;
    G4Accumulable<G4double> fCollectedCharge2;

    // Accumulables for total dose and its square
    G4Accumulable<G4double> fDose;
    G4Accumulable<G4double> fDose2;

    // Accumulables for total energy deposition and its square
    G4int fSimulatedMU = 1; //
    G4double fScaleFactorMU = 1/2.3514811546434146e-13; // [cGy/ev -> cGy/UM (dmax)] =  1/2.3514811546434146e-13

    // Run action messenger
    G4UImessenger* 	 				fRunActionMessenger ;
};

} // namespace MD1

#endif // MD1_RUN_ACTION_H
