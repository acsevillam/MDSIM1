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
#include "geometry/base/BaseDigitizer.hh"
#include "G4THitsCollection.hh"
#include "G4SystemOfUnits.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/readout/BB7Hit.hh"
#include "geometry/detectors/BB7/readout/BB7Digit.hh"
#include "geometry/detectors/BB7/readout/BB7ReadoutParameters.hh"

/**
 * @class BB7Digitizer
 * @brief Class responsible for digitizing hits in the BB7 detector in Geant4 simulations.
 */
class BB7Digitizer : public BaseDigitizer {
public:
    /**
     * @brief Constructor for BB7Digitizer.
     * @param name Name of the digitizer module.
     */
    BB7Digitizer(const G4String& name, const BB7ReadoutParameters& readoutParameters);

    /**
     * @brief Destructor for BB7Digitizer.
     */
    ~BB7Digitizer() override;

    /**
     * @brief Perform the digitization process.
     */
    void Digitize() override;

private:
    const BB7HitsCollection* fHitsCollection; ///< Hits collection
    BB7DigitsCollection* fDigitsCollection; ///< Digits collection
    G4int fDCID; ///< Digits collection ID
    BB7ReadoutParameters fReadoutParameters; ///< BB7 readout parameters and calibration
};

#endif // BB7_DIGITIZER_H
