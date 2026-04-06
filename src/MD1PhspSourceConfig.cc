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

#include "G4IAEAphspReader.hh"
#include "G4GenericMessenger.hh"
#include "G4ApplicationState.hh"
#include "G4ios.hh"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <system_error>

namespace MD1 {

MD1PhspSourceConfig* MD1PhspSourceConfig::instance = nullptr;

namespace {

constexpr char kPhspExtension[] = ".IAEAphsp";
constexpr char kHeaderExtension[] = ".IAEAheader";
constexpr char kEOFPolicyCandidates[] = "abort restart stop synthetic";

namespace fs = std::filesystem;

bool EndsWith(const G4String& value, const G4String& suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

G4String CanonicalEOFPolicyOrThrow(const G4String& policyName) {
    switch (G4IAEAphspReader::ParseEOFPolicy(policyName)) {
        case G4IAEAphspReader::EOFPolicy::Abort:
            return "abort";
        case G4IAEAphspReader::EOFPolicy::Restart:
            return "restart";
        case G4IAEAphspReader::EOFPolicy::Stop:
            return "stop";
        case G4IAEAphspReader::EOFPolicy::Synthetic:
            return "synthetic";
    }

    return "abort";
}

G4String GetConfiguredDataDirectory() {
    if (const char* envValue = std::getenv("MDSIM1_DATA_DIR")) {
        if (*envValue != '\0') {
            return envValue;
        }
    }

#ifdef MDSIM1_DATA_DIR_DEFAULT
    return MDSIM1_DATA_DIR_DEFAULT;
#else
    return "";
#endif
}

std::vector<fs::path> BuildSearchRoots() {
    std::vector<fs::path> roots;
    std::error_code ec;
    const fs::path currentPath = fs::current_path(ec);
    if (!ec && !currentPath.empty()) {
        roots.push_back(currentPath.lexically_normal());
    }

    const G4String configuredDataDirectory = GetConfiguredDataDirectory();
    if (!configuredDataDirectory.empty()) {
        const fs::path configuredPath(configuredDataDirectory.c_str());
        const fs::path normalizedConfiguredPath = configuredPath.lexically_normal();
        if (std::find(roots.begin(), roots.end(), normalizedConfiguredPath) == roots.end()) {
            roots.push_back(normalizedConfiguredPath);
        }
    }

    return roots;
}

bool FileExists(const fs::path& path) {
    std::error_code ec;
    return fs::is_regular_file(path, ec);
}

bool DirectoryExists(const fs::path& path) {
    std::error_code ec;
    return fs::is_directory(path, ec);
}

struct PhaseSpaceDatasetPresence {
    G4bool hasPhaseSpace = false;
    G4bool hasHeader = false;

    G4bool Exists() const {
        return hasPhaseSpace || hasHeader;
    }

    G4bool IsComplete() const {
        return hasPhaseSpace && hasHeader;
    }
};

PhaseSpaceDatasetPresence GetPhaseSpaceDatasetPresence(const fs::path& basePath) {
    PhaseSpaceDatasetPresence presence;
    const fs::path phspPath(basePath.string() + kPhspExtension);
    const fs::path headerPath(basePath.string() + kHeaderExtension);
    presence.hasPhaseSpace = FileExists(phspPath);
    presence.hasHeader = FileExists(headerPath);
    return presence;
}

void ThrowIncompletePhaseSpaceDataset(const fs::path& basePath,
                                      const PhaseSpaceDatasetPresence& presence) {
    G4String message = "Incomplete phase-space dataset '" + basePath.string() + "'. Missing ";
    if (!presence.hasPhaseSpace && !presence.hasHeader) {
        message += kPhspExtension;
        message += " and ";
        message += kHeaderExtension;
    } else if (!presence.hasPhaseSpace) {
        message += kPhspExtension;
    } else {
        message += kHeaderExtension;
    }
    message += ".";

    G4Exception("MD1PhspSourceConfig",
                "IncompletePhaseSpaceDataset",
                FatalException,
                message.c_str());
}

void ValidatePhaseSpaceDatasetIfPresentOrThrow(const fs::path& basePath) {
    const auto presence = GetPhaseSpaceDatasetPresence(basePath);
    if (presence.Exists() && !presence.IsComplete()) {
        ThrowIncompletePhaseSpaceDataset(basePath, presence);
    }
}

G4String ResolvePhaseSpaceBaseName(const G4String& normalizedBaseName) {
    const fs::path inputPath(normalizedBaseName.c_str());
    if (inputPath.is_absolute()) {
        ValidatePhaseSpaceDatasetIfPresentOrThrow(inputPath);
        return normalizedBaseName;
    }

    const auto inputPresence = GetPhaseSpaceDatasetPresence(inputPath);
    if (inputPresence.Exists()) {
        if (!inputPresence.IsComplete()) {
            ThrowIncompletePhaseSpaceDataset(inputPath, inputPresence);
        }
        return normalizedBaseName;
    }

    for (const auto& root : BuildSearchRoots()) {
        const fs::path candidate = (root / inputPath).lexically_normal();
        const auto candidatePresence = GetPhaseSpaceDatasetPresence(candidate);
        if (!candidatePresence.Exists()) {
            continue;
        }
        if (!candidatePresence.IsComplete()) {
            ThrowIncompletePhaseSpaceDataset(candidate, candidatePresence);
        }
        if (candidatePresence.IsComplete()) {
            return candidate.string();
        }
    }

    return normalizedBaseName;
}

std::vector<G4String> CollectPrefixMatches(const fs::path& directory,
                                           const G4String& basenamePrefix,
                                           G4bool preserveRelativePaths) {
    std::vector<G4String> resolved;
    if (!DirectoryExists(directory)) {
        return resolved;
    }

    std::unique_ptr<DIR, int (*)(DIR*)> dir(opendir(directory.c_str()), closedir);
    if (!dir) {
        return resolved;
    }

    std::map<G4String, PhaseSpaceDatasetPresence> sortedMatches;
    while (auto* entry = readdir(dir.get())) {
        const G4String fileName = entry->d_name;
        G4String baseName;
        PhaseSpaceDatasetPresence presence;
        if (EndsWith(fileName, kPhspExtension)) {
            baseName = fileName.substr(0, fileName.size() - std::strlen(kPhspExtension));
            presence.hasPhaseSpace = true;
        } else if (EndsWith(fileName, kHeaderExtension)) {
            baseName = fileName.substr(0, fileName.size() - std::strlen(kHeaderExtension));
            presence.hasHeader = true;
        } else {
            continue;
        }
        if (baseName.rfind(basenamePrefix, 0) != 0) {
            continue;
        }

        auto& datasetPresence = sortedMatches[baseName];
        datasetPresence.hasPhaseSpace = datasetPresence.hasPhaseSpace || presence.hasPhaseSpace;
        datasetPresence.hasHeader = datasetPresence.hasHeader || presence.hasHeader;
    }

    for (const auto& [baseName, presence] : sortedMatches) {
        const fs::path baseNamePath(baseName.c_str());
        const fs::path resolvedBasePath =
            preserveRelativePaths
                ? ((directory == fs::path(".")) ? baseNamePath : directory / baseNamePath)
                : (directory / baseNamePath).lexically_normal();

        if (!presence.IsComplete()) {
            ThrowIncompletePhaseSpaceDataset(resolvedBasePath, presence);
        }

        resolved.push_back(resolvedBasePath.string());
    }

    return resolved;
}

} // namespace

MD1PhspSourceConfig::MD1PhspSourceConfig()
    : fMessenger(new G4GenericMessenger(this,
                                        "/MultiDetector1/beamline/clinac/phsp/",
                                        "Phase-space multi-source configuration")),
      fEOFPolicy("abort"),
      fCachedResolvedVersion(-1),
      fVersion(0) {
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

    auto& setEOFPolicyCmd =
        fMessenger->DeclareMethod("setEOFPolicy", &MD1PhspSourceConfig::SetEOFPolicy,
                                  "Set the phase-space EOF policy: abort, restart, stop, or synthetic.");
    setEOFPolicyCmd.SetStates(G4State_PreInit, G4State_Idle);
    setEOFPolicyCmd.SetCandidates(kEOFPolicyCandidates);

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
    std::lock_guard<std::mutex> lock(fMutex);
    const G4String normalizedFileName = NormalizeBaseName(fileName);
    (void)ResolvePhaseSpaceBaseName(normalizedFileName);
    fExplicitFiles.push_back(normalizedFileName);
    fVersion.fetch_add(1, std::memory_order_release);
}

void MD1PhspSourceConfig::ClearFiles() {
    std::lock_guard<std::mutex> lock(fMutex);
    fExplicitFiles.clear();
    fVersion.fetch_add(1, std::memory_order_release);
}

void MD1PhspSourceConfig::SetPrefix(const G4String& prefix) {
    std::lock_guard<std::mutex> lock(fMutex);
    fPrefix = NormalizeBaseName(prefix);
    (void)ResolveFromPrefixLocked();
    fVersion.fetch_add(1, std::memory_order_release);
}

void MD1PhspSourceConfig::ClearPrefix() {
    std::lock_guard<std::mutex> lock(fMutex);
    fPrefix.clear();
    fVersion.fetch_add(1, std::memory_order_release);
}

void MD1PhspSourceConfig::SetEOFPolicy(const G4String& policyName) {
    std::lock_guard<std::mutex> lock(fMutex);
    fEOFPolicy = CanonicalEOFPolicyOrThrow(policyName);
    fVersion.fetch_add(1, std::memory_order_release);
}

G4String MD1PhspSourceConfig::GetEOFPolicy() const {
    std::lock_guard<std::mutex> lock(fMutex);
    return fEOFPolicy;
}

G4int MD1PhspSourceConfig::GetVersion() const {
    return fVersion.load(std::memory_order_acquire);
}

std::vector<G4String> MD1PhspSourceConfig::ResolveFromPrefixLocked() const {
    std::vector<G4String> resolved;
    if (fPrefix.empty()) {
        return resolved;
    }

    const fs::path prefixPath(fPrefix.c_str());
    const fs::path directory = prefixPath.has_parent_path() ? prefixPath.parent_path() : fs::path(".");
    const G4String basenamePrefix = prefixPath.filename().string();

    if (prefixPath.is_absolute()) {
        return CollectPrefixMatches(directory, basenamePrefix, false);
    }

    resolved = CollectPrefixMatches(directory, basenamePrefix, true);
    if (!resolved.empty()) {
        return resolved;
    }

    for (const auto& root : BuildSearchRoots()) {
        const auto candidateDirectory = (root / directory).lexically_normal();
        resolved = CollectPrefixMatches(candidateDirectory, basenamePrefix, false);
        if (!resolved.empty()) {
            return resolved;
        }
    }

    return resolved;
}

std::vector<G4String> MD1PhspSourceConfig::ResolveSourceBaseNamesLocked() const {
    if (!fExplicitFiles.empty()) {
        std::vector<G4String> resolved;
        resolved.reserve(fExplicitFiles.size());
        for (const auto& file : fExplicitFiles) {
            resolved.push_back(ResolvePhaseSpaceBaseName(file));
        }
        return resolved;
    }
    return ResolveFromPrefixLocked();
}

MD1PhspSourceConfig::Snapshot MD1PhspSourceConfig::GetSnapshot() const {
    std::lock_guard<std::mutex> lock(fMutex);
    const G4int version = fVersion.load(std::memory_order_relaxed);
    if (fCachedResolvedVersion != version) {
        fCachedResolvedFiles = ResolveSourceBaseNamesLocked();
        fCachedResolvedVersion = version;
    }
    Snapshot snapshot;
    snapshot.sourceFiles = fCachedResolvedFiles;
    snapshot.eofPolicy = fEOFPolicy;
    snapshot.version = version;
    return snapshot;
}

std::vector<G4String> MD1PhspSourceConfig::ResolveSourceBaseNames() const {
    return GetSnapshot().sourceFiles;
}

void MD1PhspSourceConfig::ListResolvedFiles() {
    const auto snapshot = GetSnapshot();
    G4cout << "Resolved PHSP sources:" << G4endl;
    if (snapshot.sourceFiles.empty()) {
        G4cout << " - none" << G4endl;
        return;
    }
    for (const auto& file : snapshot.sourceFiles) {
        G4cout << " - " << file << G4endl;
    }
}

} // namespace MD1
