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
 *
 */

#include "geometry/detectors/BB7/readout/BB7ReadoutModel.hh"

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"

namespace {

constexpr G4double kStaticMeanEnergyPerIon = 3.62 * eV;
constexpr G4double kStaticElementaryCharge = 1.60217663e-19 * coulomb;
constexpr G4double kStaticCalibrationFactor = 1.0 * gray / coulomb;
constexpr G4double kStaticCalibrationFactorError = 0.01 * kStaticCalibrationFactor;

} // namespace

BB7ReadoutParameters BB7ReadoutModel::Build() {
    BB7ReadoutParameters parameters;
    parameters.meanEnergyPerIon = kStaticMeanEnergyPerIon;
    parameters.elementaryCharge = kStaticElementaryCharge;
    parameters.calibrationFactor = kStaticCalibrationFactor;
    parameters.calibrationFactorError = kStaticCalibrationFactorError;

    if (parameters.meanEnergyPerIon <= 0.) {
        G4Exception("BB7ReadoutModel::Build",
                    "BB7ReadoutInvalidMeanEnergyPerIon",
                    FatalException,
                    "BB7 mean energy per ion must be strictly positive.");
        return parameters;
    }

    if (parameters.elementaryCharge <= 0.) {
        G4Exception("BB7ReadoutModel::Build",
                    "BB7ReadoutInvalidElementaryCharge",
                    FatalException,
                    "BB7 elementary charge must be strictly positive.");
        return parameters;
    }

    if (parameters.calibrationFactor <= 0.) {
        G4Exception("BB7ReadoutModel::Build",
                    "BB7ReadoutInvalidCalibrationFactor",
                    FatalException,
                    "BB7 calibration factor must be strictly positive.");
        return parameters;
    }

    if (parameters.calibrationFactorError < 0.) {
        G4Exception("BB7ReadoutModel::Build",
                    "BB7ReadoutInvalidCalibrationFactorError",
                    FatalException,
                    "BB7 calibration factor uncertainty must be non-negative.");
        return parameters;
    }

    return parameters;
}
