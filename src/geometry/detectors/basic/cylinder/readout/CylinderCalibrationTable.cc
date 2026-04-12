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

#include "geometry/detectors/basic/cylinder/readout/CylinderCalibrationTable.hh"

#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcommand.hh"

namespace {

struct CylinderCalibrationEntry {
    G4String materialName;
    G4double cylinderRadius = 0.;
    G4double cylinderHeight = 0.;
    G4String envelopeMaterialName;
    G4double envelopeThickness = 0.;
    G4double calibrationFactor = 0.;
    G4double calibrationFactorError = 0.;
};

constexpr G4double kCylinderRadiusTolerance = 1.e-6 * mm;
constexpr G4double kCylinderHeightTolerance = 1.e-6 * mm;
constexpr G4double kEnvelopeThicknessTolerance = 1.e-6 * mm;
const G4String gCalibrationFilePath =
    "src/geometry/detectors/basic/cylinder/geometry/CylinderCalibrationTable.dat";

G4double ParseLengthUnitOrThrow(const std::string& unitName, G4int lineNumber) {
    const G4double unitValue = G4UIcommand::ValueOf(unitName.c_str());
    if (unitValue <= 0.) {
        G4Exception("CylinderCalibrationTable::ParseLengthUnitOrThrow",
                    "CylinderCalibrationInvalidUnit",
                    FatalException,
                    ("Invalid length unit in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ": " + unitName).c_str());
    }
    return unitValue;
}

void ValidateCalibrationEntryOrThrow(const std::string& materialName,
                                     G4double cylinderRadius,
                                     G4double cylinderHeight,
                                     const std::string& envelopeMaterialName,
                                     G4double envelopeThickness,
                                     G4double calibrationFactorValue,
                                     G4double calibrationFactorErrorValue,
                                     const std::vector<CylinderCalibrationEntry>& calibrationEntries,
                                     G4int lineNumber) {
    if (cylinderRadius <= 0.) {
        G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CylinderCalibrationInvalidRadius",
                    FatalException,
                    ("Cylinder radius must be positive in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }
    if (cylinderHeight <= 0.) {
        G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CylinderCalibrationInvalidHeight",
                    FatalException,
                    ("Cylinder height must be positive in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorValue <= 0.) {
        G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CylinderCalibrationInvalidFactor",
                    FatalException,
                    ("Calibration factor must be positive in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorErrorValue < 0.) {
        G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CylinderCalibrationInvalidFactorError",
                    FatalException,
                    ("Calibration factor uncertainty must be non-negative in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }
    if (envelopeThickness < 0.) {
        G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "CylinderCalibrationInvalidEnvelopeThickness",
                    FatalException,
                    ("Envelope thickness must be non-negative in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }

    for (const auto& existingEntry : calibrationEntries) {
        if (existingEntry.materialName == materialName &&
            std::abs(existingEntry.cylinderRadius - cylinderRadius) <= kCylinderRadiusTolerance &&
            std::abs(existingEntry.cylinderHeight - cylinderHeight) <= kCylinderHeightTolerance &&
            existingEntry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(existingEntry.envelopeThickness - envelopeThickness) <=
                kEnvelopeThicknessTolerance) {
            G4Exception("CylinderCalibrationTable::ValidateCalibrationEntryOrThrow",
                        "CylinderCalibrationDuplicateEntry",
                        FatalException,
                        ("Duplicate calibration entry for material " + materialName +
                         ", envelope material " + envelopeMaterialName + " in " +
                         gCalibrationFilePath + " at line " + std::to_string(lineNumber) + ".").c_str());
        }
    }
}

std::vector<CylinderCalibrationEntry> LoadCalibrationEntries() {
    std::ifstream inputFile(gCalibrationFilePath.c_str());
    if (!inputFile.is_open()) {
        G4Exception("CylinderCalibrationTable::LoadCalibrationEntries",
                    "CylinderCalibrationFileNotFound",
                    FatalException,
                    ("Could not open cylinder calibration table file: " + gCalibrationFilePath).c_str());
        return {};
    }

    std::vector<CylinderCalibrationEntry> calibrationEntries;

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
        G4double radiusValue = 0.;
        std::string radiusUnit;
        G4double heightValue = 0.;
        std::string heightUnit;
        std::string envelopeMaterialName;
        G4double envelopeThicknessValue = 0.;
        std::string envelopeThicknessUnit;
        G4double calibrationFactorValue = 0.;
        G4double calibrationFactorErrorValue = 0.;

        if (!(lineStream >> materialName)) {
            continue;
        }

        if (!(lineStream >> radiusValue >> radiusUnit >> heightValue >> heightUnit >>
              envelopeMaterialName >> envelopeThicknessValue >> envelopeThicknessUnit >>
              calibrationFactorValue >> calibrationFactorErrorValue)) {
            G4Exception("CylinderCalibrationTable::LoadCalibrationEntries",
                        "CylinderCalibrationInvalidLine",
                        FatalException,
                        ("Invalid cylinder calibration entry in " + gCalibrationFilePath +
                         " at line " + std::to_string(lineNumber)).c_str());
            return {};
        }

        const G4double radiusUnitValue = ParseLengthUnitOrThrow(radiusUnit, lineNumber);
        const G4double heightUnitValue = ParseLengthUnitOrThrow(heightUnit, lineNumber);
        const G4double envelopeThicknessUnitValue =
            ParseLengthUnitOrThrow(envelopeThicknessUnit, lineNumber);
        const G4double cylinderRadius = radiusValue * radiusUnitValue;
        const G4double cylinderHeight = heightValue * heightUnitValue;
        const G4double envelopeThickness = envelopeThicknessValue * envelopeThicknessUnitValue;

        ValidateCalibrationEntryOrThrow(materialName,
                                        cylinderRadius,
                                        cylinderHeight,
                                        envelopeMaterialName,
                                        envelopeThickness,
                                        calibrationFactorValue,
                                        calibrationFactorErrorValue,
                                        calibrationEntries,
                                        lineNumber);

        CylinderCalibrationEntry entry;
        entry.materialName = materialName;
        entry.cylinderRadius = cylinderRadius;
        entry.cylinderHeight = cylinderHeight;
        entry.envelopeMaterialName = envelopeMaterialName;
        entry.envelopeThickness = envelopeThickness;
        entry.calibrationFactor = calibrationFactorValue * (1e-2 * gray) / (1e-9 * coulomb);
        entry.calibrationFactorError =
            calibrationFactorErrorValue * (1e-2 * gray) / (1e-9 * coulomb);
        calibrationEntries.push_back(entry);
    }

    return calibrationEntries;
}

const std::vector<CylinderCalibrationEntry>& GetCalibrationEntries() {
    static const std::vector<CylinderCalibrationEntry> calibrationEntries = LoadCalibrationEntries();
    return calibrationEntries;
}

const CylinderCalibrationEntry* FindEntry(const G4String& materialName,
                                          G4double cylinderRadius,
                                          G4double cylinderHeight,
                                          const G4String& envelopeMaterialName,
                                          G4double envelopeThickness) {
    const auto& calibrationEntries = GetCalibrationEntries();
    for (const auto& entry : calibrationEntries) {
        if (entry.materialName == materialName &&
            std::abs(entry.cylinderRadius - cylinderRadius) <= kCylinderRadiusTolerance &&
            std::abs(entry.cylinderHeight - cylinderHeight) <= kCylinderHeightTolerance &&
            entry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(entry.envelopeThickness - envelopeThickness) <=
                kEnvelopeThicknessTolerance) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace

G4bool CylinderCalibrationTable::HasCalibrationFactor(const G4String& materialName,
                                                      G4double cylinderRadius,
                                                      G4double cylinderHeight,
                                                      const G4String& envelopeMaterialName,
                                                      G4double envelopeThickness) {
    return FindEntry(materialName,
                     cylinderRadius,
                     cylinderHeight,
                     envelopeMaterialName,
                     envelopeThickness) != nullptr;
}

G4double CylinderCalibrationTable::GetCalibrationFactor(const G4String& materialName,
                                                        G4double cylinderRadius,
                                                        G4double cylinderHeight,
                                                        const G4String& envelopeMaterialName,
                                                        G4double envelopeThickness) {
    return GetCalibrationData(materialName,
                              cylinderRadius,
                              cylinderHeight,
                              envelopeMaterialName,
                              envelopeThickness)
        .factor;
}

CylinderCalibrationTable::CalibrationData CylinderCalibrationTable::GetCalibrationData(
    const G4String& materialName,
    G4double cylinderRadius,
    G4double cylinderHeight,
    const G4String& envelopeMaterialName,
    G4double envelopeThickness) {
    const auto* entry =
        FindEntry(materialName, cylinderRadius, cylinderHeight, envelopeMaterialName, envelopeThickness);
    if (entry == nullptr) {
        G4Exception("CylinderCalibrationTable::GetCalibrationData",
                    "CylinderCalibrationEntryNotFound",
                    FatalException,
                    ("No experimental cylinder calibration factor was found for material " +
                     materialName + ", radius " + std::to_string(cylinderRadius / mm) +
                     " mm, height " + std::to_string(cylinderHeight / mm) +
                     " mm, envelope material " + envelopeMaterialName +
                     " and envelope thickness " + std::to_string(envelopeThickness / mm) + " mm.")
                        .c_str());
        return {};
    }

    return {entry->calibrationFactor, entry->calibrationFactorError};
}
