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

#include "geometry/detectors/basic/cube/readout/CubeCalibrationTable.hh"

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
    G4String envelopeMaterialName;
    G4double envelopeThickness;
    G4double calibrationFactor;
    G4double calibrationFactorError;
};

constexpr G4double kCubeSideTolerance = 1.e-6 * mm;
constexpr G4double kEnvelopeThicknessTolerance = 1.e-6 * mm;
const G4String gCalibrationFilePath = "src/geometry/detectors/basic/cube/geometry/CubeCalibrationTable.dat";

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
                                     const std::string& envelopeMaterialName,
                                     G4double envelopeThickness,
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
    if (envelopeThickness < 0.) {
        G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CubeCalibrationInvalidEnvelopeThickness",
                    FatalException,
                    ("Envelope thickness must be non-negative in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }

    for (const auto& existingEntry : calibrationEntries) {
        if (existingEntry.materialName == materialName &&
            std::abs(existingEntry.cubeSide - cubeSide) <= kCubeSideTolerance &&
            existingEntry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(existingEntry.envelopeThickness - envelopeThickness) <= kEnvelopeThicknessTolerance) {
            G4Exception("CubeCalibrationTable::ValidateCalibrationEntryOrThrow",
                        "CubeCalibrationDuplicateEntry",
                        FatalException,
                        ("Duplicate calibration entry for material " + materialName +
                         ", envelope material " + envelopeMaterialName +
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
        G4double envelopeThicknessValue = 0.;
        std::string envelopeThicknessUnit;
        G4double calibrationFactorValue = 0.;
        G4double calibrationFactorErrorValue = 0.;

        if (!(lineStream >> materialName)) {
            continue;
        }

        std::string envelopeMaterialName;
        if (!(lineStream >> sideValue >> sideUnit >> envelopeMaterialName >> envelopeThicknessValue >>
              envelopeThicknessUnit >> calibrationFactorValue >> calibrationFactorErrorValue)) {
            G4Exception("CubeCalibrationTable::LoadCalibrationEntries",
                        "CubeCalibrationInvalidLine",
                        FatalException,
                        ("Invalid cube calibration entry in " + gCalibrationFilePath +
                         " at line " + std::to_string(lineNumber)).c_str());
            return {};
        }

        const G4double sideUnitValue = ParseLengthUnitOrThrow(sideUnit, lineNumber);
        const G4double envelopeThicknessUnitValue =
            ParseLengthUnitOrThrow(envelopeThicknessUnit, lineNumber);
        const G4double cubeSide = sideValue * sideUnitValue;
        const G4double envelopeThickness = envelopeThicknessValue * envelopeThicknessUnitValue;
        ValidateCalibrationEntryOrThrow(
            materialName,
            cubeSide,
            envelopeMaterialName,
            envelopeThickness,
            calibrationFactorValue,
            calibrationFactorErrorValue,
            calibrationEntries,
            lineNumber);

        CubeCalibrationEntry entry;
        entry.materialName = materialName;
        entry.cubeSide = cubeSide;
        entry.envelopeMaterialName = envelopeMaterialName;
        entry.envelopeThickness = envelopeThickness;
        entry.calibrationFactor = calibrationFactorValue * (1e-2 * gray) / (1e-9 * coulomb);
        entry.calibrationFactorError = calibrationFactorErrorValue * (1e-2 * gray) / (1e-9 * coulomb);
        calibrationEntries.push_back(entry);
    }

    return calibrationEntries;
}

const std::vector<CubeCalibrationEntry>& GetCalibrationEntries() {
    static const std::vector<CubeCalibrationEntry> calibrationEntries = LoadCalibrationEntries();
    return calibrationEntries;
}

const CubeCalibrationEntry* FindEntry(const G4String& materialName,
                                      G4double cubeSide,
                                      const G4String& envelopeMaterialName,
                                      G4double envelopeThickness) {
    const auto& calibrationEntries = GetCalibrationEntries();
    for (const auto& entry : calibrationEntries) {
        if (entry.materialName == materialName &&
            std::abs(entry.cubeSide - cubeSide) <= kCubeSideTolerance &&
            entry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(entry.envelopeThickness - envelopeThickness) <= kEnvelopeThicknessTolerance) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace

G4bool CubeCalibrationTable::HasCalibrationFactor(const G4String& materialName,
                                                  G4double cubeSide,
                                                  const G4String& envelopeMaterialName,
                                                  G4double envelopeThickness) {
    return FindEntry(materialName, cubeSide, envelopeMaterialName, envelopeThickness) != nullptr;
}

G4double CubeCalibrationTable::GetCalibrationFactor(const G4String& materialName,
                                                    G4double cubeSide,
                                                    const G4String& envelopeMaterialName,
                                                    G4double envelopeThickness) {
    return GetCalibrationData(materialName, cubeSide, envelopeMaterialName, envelopeThickness).factor;
}

CubeCalibrationTable::CalibrationData CubeCalibrationTable::GetCalibrationData(const G4String& materialName,
                                                                               G4double cubeSide,
                                                                               const G4String& envelopeMaterialName,
                                                                               G4double envelopeThickness) {
    const auto* entry = FindEntry(materialName, cubeSide, envelopeMaterialName, envelopeThickness);
    if (entry == nullptr) {
        G4Exception("CubeCalibrationTable::GetCalibrationData",
                    "CubeCalibrationEntryNotFound",
                    FatalException,
                    ("No experimental cube calibration factor was found for material " + materialName +
                     ", side " + std::to_string(cubeSide / mm) + " mm, envelope material " +
                     envelopeMaterialName + " and envelope thickness " +
                     std::to_string(envelopeThickness / mm) + " mm.").c_str());
        return {};
    }

    return {entry->calibrationFactor, entry->calibrationFactorError};
}
