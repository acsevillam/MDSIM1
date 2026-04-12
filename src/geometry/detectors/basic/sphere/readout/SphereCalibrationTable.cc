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

#include "geometry/detectors/basic/sphere/readout/SphereCalibrationTable.hh"

#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#include "G4Exception.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcommand.hh"

namespace {

struct SphereCalibrationEntry {
    G4String materialName;
    G4double sphereRadius = 0.;
    G4String envelopeMaterialName;
    G4double envelopeThickness = 0.;
    G4double calibrationFactor = 0.;
    G4double calibrationFactorError = 0.;
};

constexpr G4double kSphereRadiusTolerance = 1.e-6 * mm;
constexpr G4double kEnvelopeThicknessTolerance = 1.e-6 * mm;
const G4String gCalibrationFilePath =
    "src/geometry/detectors/basic/sphere/geometry/SphereCalibrationTable.dat";

G4double ParseLengthUnitOrThrow(const std::string& unitName, G4int lineNumber) {
    const G4double unitValue = G4UIcommand::ValueOf(unitName.c_str());
    if (unitValue <= 0.) {
        G4Exception("SphereCalibrationTable::ParseLengthUnitOrThrow",
                    "SphereCalibrationInvalidUnit",
                    FatalException,
                    ("Invalid length unit in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ": " + unitName).c_str());
    }
    return unitValue;
}

void ValidateCalibrationEntryOrThrow(const std::string& materialName,
                                     G4double sphereRadius,
                                     const std::string& envelopeMaterialName,
                                     G4double envelopeThickness,
                                     G4double calibrationFactorValue,
                                     G4double calibrationFactorErrorValue,
                                     const std::vector<SphereCalibrationEntry>& calibrationEntries,
                                     G4int lineNumber) {
    if (sphereRadius <= 0.) {
        G4Exception("SphereCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "SphereCalibrationInvalidRadius",
                    FatalException,
                    ("Sphere radius must be positive in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorValue <= 0.) {
        G4Exception("SphereCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "SphereCalibrationInvalidFactor",
                    FatalException,
                    ("Calibration factor must be positive in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }
    if (calibrationFactorErrorValue < 0.) {
        G4Exception("SphereCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "SphereCalibrationInvalidFactorError",
                    FatalException,
                    ("Calibration factor uncertainty must be non-negative in " + gCalibrationFilePath +
                     " at line " + std::to_string(lineNumber) + ".").c_str());
    }
    if (envelopeThickness < 0.) {
        G4Exception("SphereCalibrationTable::ValidateCalibrationEntryOrThrow",
                    "SphereCalibrationInvalidEnvelopeThickness",
                    FatalException,
                    ("Envelope thickness must be non-negative in " + gCalibrationFilePath + " at line " +
                     std::to_string(lineNumber) + ".").c_str());
    }

    for (const auto& existingEntry : calibrationEntries) {
        if (existingEntry.materialName == materialName &&
            std::abs(existingEntry.sphereRadius - sphereRadius) <= kSphereRadiusTolerance &&
            existingEntry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(existingEntry.envelopeThickness - envelopeThickness) <=
                kEnvelopeThicknessTolerance) {
            G4Exception("SphereCalibrationTable::ValidateCalibrationEntryOrThrow",
                        "SphereCalibrationDuplicateEntry",
                        FatalException,
                        ("Duplicate calibration entry for material " + materialName +
                         ", envelope material " + envelopeMaterialName + " in " +
                         gCalibrationFilePath + " at line " + std::to_string(lineNumber) + ".").c_str());
        }
    }
}

std::vector<SphereCalibrationEntry> LoadCalibrationEntries() {
    std::ifstream inputFile(gCalibrationFilePath.c_str());
    if (!inputFile.is_open()) {
        G4Exception("SphereCalibrationTable::LoadCalibrationEntries",
                    "SphereCalibrationFileNotFound",
                    FatalException,
                    ("Could not open sphere calibration table file: " + gCalibrationFilePath).c_str());
        return {};
    }

    std::vector<SphereCalibrationEntry> calibrationEntries;

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
        std::string envelopeMaterialName;
        G4double envelopeThicknessValue = 0.;
        std::string envelopeThicknessUnit;
        G4double calibrationFactorValue = 0.;
        G4double calibrationFactorErrorValue = 0.;

        if (!(lineStream >> materialName)) {
            continue;
        }

        if (!(lineStream >> radiusValue >> radiusUnit >> envelopeMaterialName >>
              envelopeThicknessValue >> envelopeThicknessUnit >> calibrationFactorValue >>
              calibrationFactorErrorValue)) {
            G4Exception("SphereCalibrationTable::LoadCalibrationEntries",
                        "SphereCalibrationInvalidLine",
                        FatalException,
                        ("Invalid sphere calibration entry in " + gCalibrationFilePath + " at line " +
                         std::to_string(lineNumber))
                            .c_str());
            return {};
        }

        const G4double radiusUnitValue = ParseLengthUnitOrThrow(radiusUnit, lineNumber);
        const G4double envelopeThicknessUnitValue =
            ParseLengthUnitOrThrow(envelopeThicknessUnit, lineNumber);
        const G4double sphereRadius = radiusValue * radiusUnitValue;
        const G4double envelopeThickness = envelopeThicknessValue * envelopeThicknessUnitValue;

        ValidateCalibrationEntryOrThrow(materialName,
                                        sphereRadius,
                                        envelopeMaterialName,
                                        envelopeThickness,
                                        calibrationFactorValue,
                                        calibrationFactorErrorValue,
                                        calibrationEntries,
                                        lineNumber);

        SphereCalibrationEntry entry;
        entry.materialName = materialName;
        entry.sphereRadius = sphereRadius;
        entry.envelopeMaterialName = envelopeMaterialName;
        entry.envelopeThickness = envelopeThickness;
        entry.calibrationFactor = calibrationFactorValue * (1e-2 * gray) / (1e-9 * coulomb);
        entry.calibrationFactorError =
            calibrationFactorErrorValue * (1e-2 * gray) / (1e-9 * coulomb);
        calibrationEntries.push_back(entry);
    }

    return calibrationEntries;
}

const std::vector<SphereCalibrationEntry>& GetCalibrationEntries() {
    static const std::vector<SphereCalibrationEntry> calibrationEntries = LoadCalibrationEntries();
    return calibrationEntries;
}

const SphereCalibrationEntry* FindEntry(const G4String& materialName,
                                        G4double sphereRadius,
                                        const G4String& envelopeMaterialName,
                                        G4double envelopeThickness) {
    const auto& calibrationEntries = GetCalibrationEntries();
    for (const auto& entry : calibrationEntries) {
        if (entry.materialName == materialName &&
            std::abs(entry.sphereRadius - sphereRadius) <= kSphereRadiusTolerance &&
            entry.envelopeMaterialName == envelopeMaterialName &&
            std::abs(entry.envelopeThickness - envelopeThickness) <=
                kEnvelopeThicknessTolerance) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace

G4bool SphereCalibrationTable::HasCalibrationFactor(const G4String& materialName,
                                                    G4double sphereRadius,
                                                    const G4String& envelopeMaterialName,
                                                    G4double envelopeThickness) {
    return FindEntry(materialName, sphereRadius, envelopeMaterialName, envelopeThickness) != nullptr;
}

G4double SphereCalibrationTable::GetCalibrationFactor(const G4String& materialName,
                                                      G4double sphereRadius,
                                                      const G4String& envelopeMaterialName,
                                                      G4double envelopeThickness) {
    return GetCalibrationData(materialName, sphereRadius, envelopeMaterialName, envelopeThickness)
        .factor;
}

SphereCalibrationTable::CalibrationData SphereCalibrationTable::GetCalibrationData(
    const G4String& materialName,
    G4double sphereRadius,
    const G4String& envelopeMaterialName,
    G4double envelopeThickness) {
    const auto* entry = FindEntry(materialName, sphereRadius, envelopeMaterialName, envelopeThickness);
    if (entry == nullptr) {
        G4Exception("SphereCalibrationTable::GetCalibrationData",
                    "SphereCalibrationEntryNotFound",
                    FatalException,
                    ("No experimental sphere calibration factor was found for material " +
                     materialName + ", radius " + std::to_string(sphereRadius / mm) +
                     " mm, envelope material " + envelopeMaterialName +
                     " and envelope thickness " + std::to_string(envelopeThickness / mm) + " mm.")
                        .c_str());
        return {};
    }

    return {entry->calibrationFactor, entry->calibrationFactorError};
}
