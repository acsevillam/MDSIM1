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

#include "geometry/detectors/cube/readout/CubeCalibrationTable.hh"

#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#include "G4Exception.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"

namespace {

struct CubeCalibrationEntry {
    G4String materialName;
    G4double cubeSide;
    G4double calibrationFactor;
    G4double calibrationFactorError;
};

constexpr G4double kCubeSideTolerance = 1.e-6 * mm;
const G4String gCalibrationFilePath = "src/geometry/detectors/cube/geometry/CubeCalibrationTable.dat";

G4double ParseLengthUnitOrThrow(const std::string& sideUnit, G4int lineNumber) {
    const G4double unitValue = G4UIcommand::ValueOf(sideUnit.c_str());
    if (unitValue <= 0.) {
        G4Exception("CubeCalibrationTable::ParseLengthUnitOrThrow",
                    "CubeCalibrationInvalidUnit",
                    FatalException,
                    ("Invalid length unit in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ": " + sideUnit).c_str());
    }
    return unitValue;
}

void ValidateCalibrationEntryOrThrow(const std::string& materialName,
                                     G4double cubeSide,
                                     G4double calibrationFactorValue,
                                     G4double calibrationFactorErrorValue,
                                     const std::vector<CubeCalibrationEntry>& calibrationEntries,
                                     G4int lineNumber) {
    if (cubeSide <= 0.) {
        G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CubeCalibrationInvalidSide",
                    FatalException,
                    ("Cube side must be positive in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorValue <= 0.) {
        G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CubeCalibrationInvalidFactor",
                    FatalException,
                    ("Calibration factor must be positive in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorErrorValue < 0.) {
        G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CubeCalibrationInvalidFactorError",
                    FatalException,
                    ("Calibration factor uncertainty must be non-negative in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }

    for (const auto& existingEntry : calibrationEntries) {
        if (existingEntry.materialName == materialName &&
            std::abs(existingEntry.cubeSide - cubeSide) <= kCubeSideTolerance) {
            G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                        "CubeCalibrationDuplicateEntry",
                        FatalException,
                        ("Duplicate calibration entry for material " + materialName +
                         " in " + gCalibrationFilePath +
                         " at line " + std::to_string(lineNumber) + ".").c_str());
        }
    }
}

std::vector<CubeCalibrationEntry> LoadCalibrationEntries() {
    std::ifstream inputFile(gCalibrationFilePath.c_str());
    if (!inputFile.is_open()) {
        G4Exception("CubeCalibrationTable::LoadCalibrationEntries",
                    "CubeCalibrationFileNotFound",
                    FatalException,
                    ("Could not open cube calibration table file: " + gCalibrationFilePath).c_str());
        return {};
    }

    std::vector<CubeCalibrationEntry> calibrationEntries;

    std::string line;
    G4int lineNumber = 0;
    while (std::getline(inputFile, line)) {
        ++lineNumber;
        const auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        std::istringstream lineStream(line);
        std::string materialName;
        G4double sideValue = 0.;
        std::string sideUnit;
        G4double calibrationFactorValue = 0.;
        G4double calibrationFactorErrorValue = 0.;

        if (!(lineStream >> materialName)) {
            continue;
        }

        if (!(lineStream >> sideValue >> sideUnit >> calibrationFactorValue >> calibrationFactorErrorValue)) {
            G4Exception("CubeCalibrationTable::LoadCalibrationEntries",
                        "CubeCalibrationInvalidLine",
                        FatalException,
                        ("Invalid cube calibration entry in " + gCalibrationFilePath +
                         " at line " + std::to_string(lineNumber)).c_str());
            return {};
        }

        const G4double sideUnitValue = ParseLengthUnitOrThrow(sideUnit, lineNumber);
        const G4double cubeSide = sideValue * sideUnitValue;
        ValidateCalibrationEntryOrThrow(
            materialName,
            cubeSide,
            calibrationFactorValue,
            calibrationFactorErrorValue,
            calibrationEntries,
            lineNumber);

        calibrationEntries.push_back(
            {materialName,
             cubeSide,
             calibrationFactorValue * (1e-2 * gray) / (1e-9 * coulomb),
             calibrationFactorErrorValue * (1e-2 * gray) / (1e-9 * coulomb)});
    }

    return calibrationEntries;
}

const std::vector<CubeCalibrationEntry>& GetCalibrationEntries() {
    static const std::vector<CubeCalibrationEntry> calibrationEntries = LoadCalibrationEntries();
    return calibrationEntries;
}

const CubeCalibrationEntry* FindEntry(const G4String& materialName, G4double cubeSide) {
    const auto& calibrationEntries = GetCalibrationEntries();
    for (const auto& entry : calibrationEntries) {
        if (entry.materialName == materialName && std::abs(entry.cubeSide - cubeSide) <= kCubeSideTolerance) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace

G4bool CubeCalibrationTable::HasCalibrationFactor(const G4String& materialName, G4double cubeSide) {
    return FindEntry(materialName, cubeSide) != nullptr;
}

G4double CubeCalibrationTable::GetCalibrationFactor(const G4String& materialName, G4double cubeSide) {
    return GetCalibrationData(materialName, cubeSide).factor;
}

CubeCalibrationTable::CalibrationData CubeCalibrationTable::GetCalibrationData(const G4String& materialName,
                                                                               G4double cubeSide) {
    const auto* entry = FindEntry(materialName, cubeSide);
    if (entry == nullptr) {
        G4Exception("CubeCalibrationTable::GetCalibrationData",
                    "CubeCalibrationEntryNotFound",
                    FatalException,
                    ("No experimental cube calibration factor was found for material " + materialName + ".").c_str());
        return {};
    }

    return {entry->calibrationFactor, entry->calibrationFactorError};
}
