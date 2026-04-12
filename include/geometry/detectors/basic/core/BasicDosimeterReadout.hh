#pragma once

#include <initializer_list>

#include "globals.hh"

namespace MD1 {

struct BasicDosimeterReadoutParameters {
    G4double meanEnergyPerIon = 0.;
    G4double elementaryCharge = 0.;
};

struct BasicDosimeterDimensionCheck {
    const char* label = "";
    G4double value = 0.;
};

BasicDosimeterReadoutParameters BuildBasicDosimeterReadoutParameters(
    const G4String& materialName,
    std::initializer_list<BasicDosimeterDimensionCheck> dimensions,
    const G4String& exceptionOrigin);

G4double ComputeBasicDosimeterCollectedCharge(
    G4double edep,
    const BasicDosimeterReadoutParameters& readoutParameters);

G4double GetBasicDosimeterSensitiveVolumeMassOrThrow(const G4String& logicalVolumeName,
                                                     const G4String& exceptionOrigin,
                                                     const G4String& exceptionCode);

} // namespace MD1
