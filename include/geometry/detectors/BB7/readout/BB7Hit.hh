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

#ifndef BB7_HIT_H
#define BB7_HIT_H

// Geant4 Headers
#include "geometry/base/BaseHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

/**
 * @class BB7Hit
 * @brief Class representing a hit in the BB7 detector for Geant4 simulations.
 */
class BB7Hit : public BaseHit {
public:
    /**
     * @brief Default constructor for BB7Hit.
     */
    BB7Hit() : BaseHit("BB7") {}

    /**
     * @brief Default copy constructor for BB7Hit.
     */
    BB7Hit(const BB7Hit&) = default;

    /**
     * @brief Default destructor for BB7Hit.
     */
    ~BB7Hit() override = default;

    /**
     * @brief Assignment operator.
     */
    BB7Hit& operator=(const BB7Hit&) = default;

    /**
     * @brief Equality operator.
     * @param other The other BB7Hit object to compare to.
     * @return True if equal, false otherwise.
     */
    G4bool operator==(const BB7Hit& other) const;

    /**
     * @brief Custom new operator.
     * @param size Size of the memory to allocate.
     * @return Pointer to the allocated memory.
     */
    inline void* operator new(size_t size);

    /**
     * @brief Custom delete operator.
     * @param hit Pointer to the memory to deallocate.
     */
    inline void operator delete(void* hit);

    /**
     * @brief Draw method from base class (no implementation).
     */
    void Draw() override {}

    /**
     * @brief Print method to print hit information.
     */
    void Print() override;

    /**
     * @brief Set the detector ID.
     * @param id The detector ID to set.
     */
    void SetDetectorID(G4int id) { fDetectorID = id; }

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
     * @param edep The energy deposition to set.
     */
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetWeight(G4double weight) { fWeight = weight; }

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
    G4double GetWeight() const { return fWeight; }

private:
    G4int fDetectorID; ///< Detector ID
    G4int fSensorID;   ///< Sensor ID
    G4int fStripID;    ///< Strip ID
    G4double fEdep;    ///< Energy deposition
    G4double fWeight = 1.; ///< Statistical weight
};

using BB7HitsCollection = G4THitsCollection<BB7Hit>;

extern G4ThreadLocal G4Allocator<BB7Hit>* BB7HitAllocator;

inline void* BB7Hit::operator new(size_t size) {
    if (!BB7HitAllocator) {
        BB7HitAllocator = new G4Allocator<BB7Hit>;
    }
    return BB7HitAllocator->MallocSingle();
}

inline void BB7Hit::operator delete(void* hit) {
    if (!BB7HitAllocator) {
        BB7HitAllocator = new G4Allocator<BB7Hit>;
    }
    BB7HitAllocator->FreeSingle(static_cast<BB7Hit*>(hit));
}

#endif // BB7_HIT_H
