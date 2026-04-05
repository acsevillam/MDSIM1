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
};

constexpr G4double kCubeSideTolerance = 1.e-6 * mm;
const G4String gCalibrationFilePath = "src/geometry/detectors/cube/geometry/CubeCalibrationTable.dat";
std::vector<CubeCalibrationEntry> gCalibrationEntries;
G4bool gCalibrationEntriesLoaded = false;

void LoadCalibrationEntries() {
    if (gCalibrationEntriesLoaded) {
        return;
    }

    std::ifstream inputFile(gCalibrationFilePath.c_str());
    if (!inputFile.is_open()) {
        G4Exception("CubeCalibrationTable::LoadCalibrationEntries",
                    "CubeCalibrationFileNotFound",
                    FatalException,
                    ("Could not open cube calibration table file: " + gCalibrationFilePath).c_str());
        return;
    }

    gCalibrationEntries.clear();

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

        if (!(lineStream >> materialName)) {
            continue;
        }

        if (!(lineStream >> sideValue >> sideUnit >> calibrationFactorValue)) {
            G4Exception("CubeCalibrationTable::LoadCalibrationEntries",
                        "CubeCalibrationInvalidLine",
                        FatalException,
                        ("Invalid cube calibration entry in " + gCalibrationFilePath +
                         " at line " + std::to_string(lineNumber)).c_str());
            return;
        }

        gCalibrationEntries.push_back(
            {materialName,
             sideValue * G4UIcommand::ValueOf(sideUnit.c_str()),
             calibrationFactorValue * gray / coulomb});
    }

    gCalibrationEntriesLoaded = true;
}

const CubeCalibrationEntry* FindEntry(const G4String& materialName, G4double cubeSide) {
    LoadCalibrationEntries();
    for (const auto& entry : gCalibrationEntries) {
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
    const auto* entry = FindEntry(materialName, cubeSide);
    if (entry == nullptr) {
        G4Exception("CubeCalibrationTable::GetCalibrationFactor",
                    "CubeCalibrationEntryNotFound",
                    FatalException,
                    ("No experimental cube calibration factor was found for material " + materialName + ".").c_str());
        return 0.;
    }

    return entry->calibrationFactor;
}
