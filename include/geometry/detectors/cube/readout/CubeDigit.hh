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

#ifndef CUBE_DIGIT_H
#define CUBE_DIGIT_H

// Geant4 Headers
#include "G4Allocator.hh"
#include "G4TDigiCollection.hh"
#include "geometry/base/BaseDigit.hh"

class CubeDigit : public BaseDigit {
public:
    CubeDigit() : BaseDigit("cube") {}
    CubeDigit(const CubeDigit&) = default;
    ~CubeDigit() override = default;

    CubeDigit& operator=(const CubeDigit&) = default;
    G4bool operator==(const CubeDigit& right) const;

    inline void* operator new(size_t);
    inline void operator delete(void* digit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetCollectedCharge(G4double collectedCharge) { fCollectedCharge = collectedCharge; }
    void SetDose(G4double dose) { fDose = dose; }
    void SetEstimatedDoseToWater(G4double dose) { fEstimatedDoseToWater = dose; }
    void SetEstimatedDoseToWaterCalibrationError(G4double doseError) {
        fEstimatedDoseToWaterCalibrationError = doseError;
    }

    G4int GetDetectorID() const { return fDetectorID; }
    G4double GetEdep() const { return fEdep; }
    G4double GetCollectedCharge() const { return fCollectedCharge; }
    G4double GetDose() const { return fDose; }
    G4double GetEstimatedDoseToWater() const { return fEstimatedDoseToWater; }
    G4double GetEstimatedDoseToWaterCalibrationError() const {
        return fEstimatedDoseToWaterCalibrationError;
    }

private:
    G4int fDetectorID = -1;
    G4double fEdep = 0.;
    G4double fCollectedCharge = 0.;
    G4double fDose = 0.;
    G4double fEstimatedDoseToWater = 0.;
    G4double fEstimatedDoseToWaterCalibrationError = 0.;
};

using CubeDigitsCollection = G4TDigiCollection<CubeDigit>;

extern G4ThreadLocal G4Allocator<CubeDigit>* CubeDigitAllocator;

inline void* CubeDigit::operator new(size_t) {
    if (!CubeDigitAllocator) {
        CubeDigitAllocator = new G4Allocator<CubeDigit>;
    }
    return CubeDigitAllocator->MallocSingle();
}

inline void CubeDigit::operator delete(void* digit) {
    if (!CubeDigitAllocator) {
        CubeDigitAllocator = new G4Allocator<CubeDigit>;
    }
    CubeDigitAllocator->FreeSingle(static_cast<CubeDigit*>(digit));
}

#endif // CUBE_DIGIT_H
