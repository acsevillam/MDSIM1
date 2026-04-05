/*
 *
 * Geant4 MultiDetector Simulation v1
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

#include "MD1PhspSourceConfig.hh"

#include "G4GenericMessenger.hh"
#include "G4ApplicationState.hh"
#include "G4ios.hh"

#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <memory>
#include <set>

namespace MD1 {

MD1PhspSourceConfig* MD1PhspSourceConfig::instance = nullptr;

namespace {

constexpr char kPhspExtension[] = ".IAEAphsp";

bool EndsWith(const G4String& value, const G4String& suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

} // namespace

MD1PhspSourceConfig::MD1PhspSourceConfig()
    : fMessenger(new G4GenericMessenger(this,
                                        "/MultiDetector1/beamline/clinac/phsp/",
                                        "Phase-space multi-source configuration")) {
    auto& addFileCmd =
        fMessenger->DeclareMethod("addFile", &MD1PhspSourceConfig::AddFile,
                                  "Add a phase-space source file (base name or .IAEAphsp path).");
    addFileCmd.SetStates(G4State_PreInit);

    auto& clearFilesCmd =
        fMessenger->DeclareMethod("clearFiles", &MD1PhspSourceConfig::ClearFiles,
                                  "Clear the explicit phase-space source list.");
    clearFilesCmd.SetStates(G4State_PreInit);

    auto& setPrefixCmd =
        fMessenger->DeclareMethod("setPrefix", &MD1PhspSourceConfig::SetPrefix,
                                  "Set a phase-space file prefix for auto-discovery.");
    setPrefixCmd.SetStates(G4State_PreInit);

    auto& clearPrefixCmd =
        fMessenger->DeclareMethod("clearPrefix", &MD1PhspSourceConfig::ClearPrefix,
                                  "Clear the auto-discovery phase-space prefix.");
    clearPrefixCmd.SetStates(G4State_PreInit);

    auto& listFilesCmd =
        fMessenger->DeclareMethod("listFiles", &MD1PhspSourceConfig::ListResolvedFiles,
                                  "List the resolved phase-space sources.");
    listFilesCmd.SetStates(G4State_PreInit, G4State_Idle);
}

MD1PhspSourceConfig::~MD1PhspSourceConfig() {
    delete fMessenger;
    fMessenger = nullptr;
}

MD1PhspSourceConfig* MD1PhspSourceConfig::GetInstance() {
    if (instance == nullptr) {
        instance = new MD1PhspSourceConfig();
    }
    return instance;
}

void MD1PhspSourceConfig::Kill() {
    delete instance;
    instance = nullptr;
}

G4String MD1PhspSourceConfig::NormalizeBaseName(const G4String& fileName) const {
    if (EndsWith(fileName, kPhspExtension)) {
        return fileName.substr(0, fileName.size() - std::strlen(kPhspExtension));
    }
    return fileName;
}

void MD1PhspSourceConfig::AddFile(const G4String& fileName) {
    fExplicitFiles.push_back(NormalizeBaseName(fileName));
}

void MD1PhspSourceConfig::ClearFiles() {
    fExplicitFiles.clear();
}

void MD1PhspSourceConfig::SetPrefix(const G4String& prefix) {
    fPrefix = NormalizeBaseName(prefix);
}

void MD1PhspSourceConfig::ClearPrefix() {
    fPrefix.clear();
}

std::vector<G4String> MD1PhspSourceConfig::ResolveFromPrefix() const {
    std::vector<G4String> resolved;
    if (fPrefix.empty()) {
        return resolved;
    }

    const std::size_t slashPos = fPrefix.find_last_of('/');
    const G4String directory = (slashPos == G4String::npos) ? G4String(".") : G4String(fPrefix.substr(0, slashPos));
    const G4String basenamePrefix = (slashPos == G4String::npos) ? fPrefix : G4String(fPrefix.substr(slashPos + 1));

    std::unique_ptr<DIR, int (*)(DIR*)> dir(opendir(directory.c_str()), closedir);
    if (!dir) {
        return resolved;
    }

    std::set<G4String> sortedMatches;
    while (auto* entry = readdir(dir.get())) {
        const G4String fileName = entry->d_name;
        if (!EndsWith(fileName, kPhspExtension)) {
            continue;
        }
        if (fileName.rfind(basenamePrefix, 0) != 0) {
            continue;
        }
        sortedMatches.insert(directory + "/" + NormalizeBaseName(fileName));
    }

    resolved.assign(sortedMatches.begin(), sortedMatches.end());
    return resolved;
}

std::vector<G4String> MD1PhspSourceConfig::ResolveSourceBaseNames() const {
    if (!fExplicitFiles.empty()) {
        return fExplicitFiles;
    }
    return ResolveFromPrefix();
}

void MD1PhspSourceConfig::ListResolvedFiles() {
    const auto files = ResolveSourceBaseNames();
    G4cout << "Resolved PHSP sources:" << G4endl;
    if (files.empty()) {
        G4cout << " - none" << G4endl;
        return;
    }
    for (const auto& file : files) {
        G4cout << " - " << file << G4endl;
    }
}

} // namespace MD1
