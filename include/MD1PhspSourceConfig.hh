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

#ifndef MD1_PHSP_SOURCE_CONFIG_H
#define MD1_PHSP_SOURCE_CONFIG_H 1

#include "globals.hh"

#include <atomic>
#include <mutex>
#include <vector>

class G4GenericMessenger;

namespace MD1 {

class MD1PhspSourceConfig {
public:
    struct Snapshot {
        std::vector<G4String> sourceFiles;
        G4String eofPolicy;
        G4int version = 0;
    };

    static MD1PhspSourceConfig* GetInstance();
    static void Kill();

    void AddFile(const G4String& fileName);
    void ClearFiles();
    void SetPrefix(const G4String& prefix);
    void ClearPrefix();
    void SetEOFPolicy(const G4String& policyName);
    G4String GetEOFPolicy() const;
    void ListResolvedFiles();

    G4int GetVersion() const;
    Snapshot GetSnapshot() const;
    std::vector<G4String> ResolveSourceBaseNames() const;

private:
    MD1PhspSourceConfig();
    ~MD1PhspSourceConfig();

    static MD1PhspSourceConfig* instance;

    G4String NormalizeBaseName(const G4String& fileName) const;
    std::vector<G4String> ResolveFromPrefixLocked() const;
    std::vector<G4String> ResolveSourceBaseNamesLocked() const;

    std::vector<G4String> fExplicitFiles;
    G4String fPrefix;
    G4String fEOFPolicy;
    mutable std::vector<G4String> fCachedResolvedFiles;
    mutable G4int fCachedResolvedVersion;
    std::atomic<G4int> fVersion;
    mutable std::mutex fMutex;
    G4GenericMessenger* fMessenger;
};

} // namespace MD1

#endif // MD1_PHSP_SOURCE_CONFIG_H
