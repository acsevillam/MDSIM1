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

#include "geometry/detectors/cube/readout/CubeReadoutModel.hh"

#include "G4Exception.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "geometry/detectors/cube/readout/CubeCalibrationTable.hh"

namespace {

G4double GetMeanEnergyPerIonForMaterial(const G4String& materialName) {
    if (materialName == "G4_Si") {
        return 3.6 * eV;
    }
    if (materialName == "G4_Ge") {
        return 2.9 * eV;
    }
    if (materialName == "G4_C") {
        return 13.0 * eV;
    }
    if (materialName == "G4_WATER" || materialName == "G4_AIR") {
        return 33.97 * eV;
    }

    return 10.0 * eV;
}

} // namespace

CubeReadoutParameters CubeReadoutModel::Build(const G4String& materialName,
                                              G4double cubeSide,
                                              G4double calibrationFactor) {
    CubeReadoutParameters parameters;
    parameters.meanEnergyPerIon = GetMeanEnergyPerIonForMaterial(materialName);
    parameters.elementaryCharge = 1.60217663e-19 * coulomb;
    if (calibrationFactor > 0.) {
        parameters.calibrationFactor = calibrationFactor;
    } else {
        parameters.calibrationFactor = CubeCalibrationTable::GetCalibrationFactor(materialName, cubeSide);
    }

    auto* material = G4NistManager::Instance()->FindOrBuildMaterial(materialName, false);
    if (material == nullptr) {
        G4Exception("CubeReadoutModel::Build",
                    "CubeReadoutInvalidMaterial",
                    FatalException,
                    ("Material " + materialName + " was not found in the NIST database.").c_str());
        return parameters;
    }

    if (cubeSide <= 0.) {
        G4Exception("CubeReadoutModel::Build",
                    "CubeReadoutInvalidCubeSide",
                    FatalException,
                    "Cube side length must be positive.");
        return parameters;
    }

    if (parameters.calibrationFactor <= 0.) {
        G4Exception("CubeReadoutModel::Build",
                    "CubeReadoutInvalidCalibrationFactor",
                    FatalException,
                    "Cube calibration factor must be defined by override or local calibration table and be positive.");
        return parameters;
    }

    return parameters;
}

G4bool CubeReadoutModel::HasCalibrationFactor(const G4String& materialName,
                                              G4double cubeSide,
                                              G4double calibrationFactor) {
    if (calibrationFactor > 0.) {
        return true;
    }

    return CubeCalibrationTable::HasCalibrationFactor(materialName, cubeSide);
}
