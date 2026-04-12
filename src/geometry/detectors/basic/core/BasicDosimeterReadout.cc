#include "geometry/detectors/basic/core/BasicDosimeterReadout.hh"

#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"

namespace {

G4double GetMeanEnergyPerIonForMaterial(const G4String& materialName) {
    if (materialName == "G4_Si") {
        return 3.62 * eV;
    }
    if (materialName == "G4_Ge") {
        return 2.96 * eV;
    }
    if (materialName == "G4_WATER") {
        return 25.5 * eV;
    }
    if (materialName == "G4_AIR") {
        return 33.97 * eV;
    }

    return 25.5 * eV;
}

} // namespace

namespace MD1 {

BasicDosimeterReadoutParameters BuildBasicDosimeterReadoutParameters(
    const G4String& materialName,
    std::initializer_list<BasicDosimeterDimensionCheck> dimensions,
    const G4String& exceptionOrigin) {
    BasicDosimeterReadoutParameters parameters;
    parameters.meanEnergyPerIon = GetMeanEnergyPerIonForMaterial(materialName);
    parameters.elementaryCharge = 1.60217663e-19 * coulomb;

    auto* material = G4NistManager::Instance()->FindOrBuildMaterial(materialName, false);
    if (material == nullptr) {
        G4Exception(exceptionOrigin.c_str(),
                    "BasicDosimeterInvalidMaterial",
                    FatalException,
                    ("Material " + materialName + " was not found in the NIST database.").c_str());
    }

    for (const auto& dimension : dimensions) {
        if (dimension.value <= 0.) {
            const G4String message = G4String("Basic dosimeter dimension '") + dimension.label +
                                     "' must be positive.";
            G4Exception(exceptionOrigin.c_str(),
                        "BasicDosimeterInvalidDimension",
                        FatalException,
                        message.c_str());
        }
    }

    return parameters;
}

G4double ComputeBasicDosimeterCollectedCharge(
    G4double edep,
    const BasicDosimeterReadoutParameters& readoutParameters) {
    return static_cast<G4int>(edep / readoutParameters.meanEnergyPerIon) *
           readoutParameters.elementaryCharge;
}

G4double GetBasicDosimeterSensitiveVolumeMassOrThrow(const G4String& logicalVolumeName,
                                                     const G4String& exceptionOrigin,
                                                     const G4String& exceptionCode) {
    auto* logicalVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(logicalVolumeName, false);
    if (logicalVolume == nullptr) {
        G4Exception(exceptionOrigin.c_str(),
                    exceptionCode.c_str(),
                    FatalException,
                    ("Logical volume " + logicalVolumeName + " was not found.").c_str());
        return 0.;
    }
    return logicalVolume->GetMass(true, false);
}

} // namespace MD1
