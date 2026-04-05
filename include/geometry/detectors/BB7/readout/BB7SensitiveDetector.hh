/*
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
 */

#ifndef BB7_SENSITIVE_DETECTOR_H
#define BB7_SENSITIVE_DETECTOR_H

// Geant4 Headers
#include "geometry/base/BaseSensitiveDetector.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/readout/BB7Hit.hh"

class G4Step;
class G4HCofThisEvent;

/**
 * @class BB7SensitiveDetector
 * @brief Class representing a sensitive detector for the BB7 detector in Geant4 simulations.
 */
class BB7SensitiveDetector : public BaseSensitiveDetector {
public:
    /**
     * @brief Constructor for BB7SensitiveDetector.
     * @param name Name of the sensitive detector.
     */
    BB7SensitiveDetector(const G4String& name);

    /**
     * @brief Destructor for BB7SensitiveDetector.
     */
    ~BB7SensitiveDetector() override;

    /**
     * @brief Initialize the hits collection at the beginning of an event.
     * @param hce Hits collection of this event.
     */
    void Initialize(G4HCofThisEvent* hce) override;

    /**
     * @brief Process hits in the sensitive detector.
     * @param step Step information.
     * @param history Touchable history.
     * @return True if a hit was processed, false otherwise.
     */
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;

    /**
     * @brief End of event processing for the sensitive detector.
     * @param hce Hits collection of this event.
     */
    void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    BB7HitsCollection* fHitsCollection; ///< Hits collection for this sensitive detector
    G4int fHCID; ///< Hits collection ID
};

#endif // BB7_SENSITIVE_DETECTOR_H
