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

#ifndef BB7_DIGIT_H
#define BB7_DIGIT_H

// Geant4 Headers
#include "geometry/base/BaseDigit.hh"
#include "G4TDigiCollection.hh"
#include "G4ThreeVector.hh"
#include "G4Allocator.hh"

/**
 * @class BB7Digit
 * @brief Class representing a digit for the BB7 detector.
 */
class BB7Digit : public BaseDigit {
public:
    /**
     * @brief Default constructor for BB7Digit.
     */
    BB7Digit() : BaseDigit("BB7") {}

    /**
     * @brief Copy constructor for BB7Digit.
     */
    BB7Digit(const BB7Digit&) = default;

    /**
     * @brief Default destructor for BB7Digit.
     */
    ~BB7Digit() override = default;

    /**
     * @brief Assignment operator.
     * @param right The digit to assign.
     * @return Reference to this digit.
     */
    BB7Digit& operator=(const BB7Digit&) = default;

    /**
     * @brief Equality operator.
     * @param right The digit to compare with.
     * @return True if the digits are equal, false otherwise.
     */
    G4bool operator==(const BB7Digit&) const;

    /**
     * @brief Override of new operator.
     * @param size The size of memory to allocate.
     * @return Pointer to the allocated memory.
     */
    inline void* operator new(size_t);

    /**
     * @brief Override of delete operator.
     * @param digit Pointer to the digit to delete.
     */
    inline void  operator delete(void*);

    /**
     * @brief Draw the digit (no implementation).
     */
    void Draw() override {}

    /**
     * @brief Print the digit information.
     */
    void Print() override;

    /**
     * @brief Set the detector ID.
     * @param detectorID The detector ID.
     */
    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }

    /**
     * @brief Set the sensor ID.
     * @param id The sensor ID to set.
     */
    void SetSensorID(G4int id) { fSensorID = id; }

    /**
     * @brief Set the strip ID.
     * @param id The strip ID to set.
     */
    void SetStripID(G4int id) { fStripID = id; }

    /**
     * @brief Set the energy deposition.
     * @param edep The energy deposition.
     */
    void SetEdep(G4double edep) { fEdep = edep; }

    /**
     * @brief Set the collected charge.
     * @param collectedCharge The collected charge.
     */
    void SetCollectedCharge(G4double collectedCharge) { fCollectedCharge = collectedCharge; }

    /**
     * @brief Set the dose.
     * @param dose The dose.
     */
    void SetDose(G4double dose) { fDose = dose; }
    void SetEstimatedDoseToWater(G4double dose) { fEstimatedDoseToWater = dose; }

    /**
     * @brief Get the detector ID.
     * @return The detector ID.
     */
    G4int GetDetectorID() const { return fDetectorID; }

    /**
     * @brief Get the sensor ID.
     * @return The sensor ID.
     */
    G4int GetSensorID() const { return fSensorID; }

    /**
     * @brief Get the strip ID.
     * @return The strip ID.
     */
    G4int GetStripID() const { return fStripID; }

    /**
     * @brief Get the energy deposition.
     * @return The energy deposition.
     */
    G4double GetEdep() const { return fEdep; }

    /**
     * @brief Get the collected charge.
     * @return The collected charge.
     */
    G4double GetCollectedCharge() const { return fCollectedCharge; }

    /**
     * @brief Get the dose.
     * @return The dose.
     */
    G4double GetDose() const { return fDose; }
    G4double GetEstimatedDoseToWater() const { return fEstimatedDoseToWater; }

private:
    G4int fDetectorID; ///< Detector ID
    G4int fSensorID;   ///< Sensor ID
    G4int fStripID;    ///< Strip ID
    G4double fEdep; ///< Energy deposition
    G4double fCollectedCharge; ///< Collected charge
    G4double fDose; ///< Absorbed dose in the detector sensitive volume
    G4double fEstimatedDoseToWater; ///< Estimated absorbed dose in water from calibration
};

typedef G4TDigiCollection<BB7Digit> BB7DigitsCollection;

extern G4ThreadLocal G4Allocator<BB7Digit>* BB7DigitAllocator;

inline void* BB7Digit::operator new(size_t) {
    if (!BB7DigitAllocator) {
        BB7DigitAllocator = new G4Allocator<BB7Digit>;
    }
    return (void*)BB7DigitAllocator->MallocSingle();
}

inline void BB7Digit::operator delete(void* digit) {
    BB7DigitAllocator->FreeSingle((BB7Digit*) digit);
}

#endif // BB7_DIGIT_H
