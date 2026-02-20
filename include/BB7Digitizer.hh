/*
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
 */

#ifndef BB7_DIGITIZER_H
#define BB7_DIGITIZER_H

// Geant4 Headers
#include "G4VDigitizerModule.hh"
#include "G4THitsCollection.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "BB7Hit.hh"
#include "BB7Digit.hh"

/**
 * @class BB7Digitizer
 * @brief Class responsible for digitizing hits in the BB7 detector in Geant4 simulations.
 */
class BB7Digitizer : public G4VDigitizerModule {
public:
    /**
     * @brief Constructor for BB7Digitizer.
     * @param name Name of the digitizer module.
     */
    BB7Digitizer(const G4String& name);

    /**
     * @brief Destructor for BB7Digitizer.
     */
    ~BB7Digitizer() override;

    /**
     * @brief Perform the digitization process.
     */
    void Digitize() override;

private:
    G4THitsCollection<BB7Hit>* fHitsCollection; ///< Hits collection
    BB7DigitsCollection* fDigitsCollection; ///< Digits collection
    G4int fDCID; ///< Digits collection ID
    const G4double fMeanEnergyPerIon = 3.6 * eV; ///< Mean energy per e- ion
    const G4double feCharge = 1.60217663e-19 * coulomb; ///< electron charge
    const G4double fCallibrationFactor = 1 * gray / coulomb; // (1 / 1.56811) * (1e-6 * gray) / (1e-9 * coulomb); ///< Calibration factor (± 0.6%)
};

#endif // BB7_DIGITIZER_H
