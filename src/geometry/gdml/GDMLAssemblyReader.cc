#include "geometry/gdml/GDMLAssemblyReader.hh"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "G4AssemblyTriplet.hh"
#include "G4AssemblyVolume.hh"
#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"
#include "G4VPhysicalVolume.hh"

#include "geometry/gdml/MD1GDMLReadStructure.hh"

namespace {

namespace fs = std::filesystem;

struct BuiltAssemblyData {
    G4String rootVolumeName;
    G4String rootPhysicalVolumeName;
    std::vector<MD1::GDMLAssemblyPart> parts;
    std::set<G4String> availableVolumeNames;
};

struct FlattenedAssemblyPart {
    G4LogicalVolume* logicalVolume = nullptr;
    G4Transform3D transform;
};

G4String ResolveGDMLPathOrThrow(const G4String& gdmlPath) {
    const fs::path requestedPath(gdmlPath.c_str());
    const fs::path resolvedPath = fs::absolute(requestedPath).lexically_normal();
    if (!fs::exists(resolvedPath)) {
        G4Exception("GDMLAssemblyReader::ResolveGDMLPathOrThrow",
                    "ImportedGDMLFileNotFound",
                    FatalException,
                    ("Could not find imported GDML file: " + resolvedPath.string()).c_str());
    }

    return resolvedPath.string();
}

G4String NormalizeOptionalPath(const G4String& path) {
    if (path.empty()) {
        return "";
    }

    return fs::absolute(fs::path(path.c_str())).lexically_normal().string();
}

G4String TrimName(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

G4String RootSelectorTypeToString(const MD1::GDMLRootSelectorType type) {
    switch (type) {
        case MD1::GDMLRootSelectorType::Logical:
            return "logical";
        case MD1::GDMLRootSelectorType::Physical:
            return "physical";
        case MD1::GDMLRootSelectorType::Assembly:
            return "assembly";
        case MD1::GDMLRootSelectorType::Auto:
        default:
            return "auto";
    }
}

MD1::GDMLRootSelector NormalizeRootSelector(const MD1::GDMLRootSelector& selector) {
    MD1::GDMLRootSelector normalized = selector;
    normalized.name = TrimName(selector.name);
    if (normalized.type == MD1::GDMLRootSelectorType::Auto && !normalized.name.empty()) {
        G4Exception("GDMLAssemblyReader::NormalizeRootSelector",
                    "ImportedGDMLRootSelectorInvalid",
                    FatalException,
                    ("Imported GDML auto root selection no longer accepts an explicit name ('" +
                     normalized.name +
                     "'). Use a typed selector instead: logical, physical, or assembly.")
                        .c_str());
    }
    if (normalized.type != MD1::GDMLRootSelectorType::Auto && normalized.name.empty()) {
        G4Exception("GDMLAssemblyReader::NormalizeRootSelector",
                    "ImportedGDMLRootEmpty",
                    FatalException,
                    ("Imported GDML root selector of type '" +
                     RootSelectorTypeToString(normalized.type) + "' requires a non-empty name.")
                        .c_str());
    }
    return normalized;
}

MD1::GDMLReadOptions NormalizeReadOptions(const MD1::GDMLReadOptions& options) {
    MD1::GDMLReadOptions normalized = options;
    normalized.schemaPath = NormalizeOptionalPath(options.schemaPath);
    return normalized;
}

void ValidateLogicalVolumeOrThrow(const G4LogicalVolume* logicalVolume,
                                  const G4String& context,
                                  const G4String& gdmlPath) {
    if (logicalVolume == nullptr) {
        G4Exception(context,
                    "ImportedGDMLMissingLogicalVolume",
                    FatalException,
                    ("Imported GDML " + gdmlPath + " did not provide a valid logical volume.").c_str());
    }
}

void ValidatePhysicalVolumeOrThrow(const G4VPhysicalVolume* physicalVolume,
                                   const G4String& context,
                                   const G4String& gdmlPath) {
    if (physicalVolume == nullptr) {
        G4Exception(context,
                    "ImportedGDMLMissingPhysicalVolume",
                    FatalException,
                    ("Imported GDML " + gdmlPath + " did not provide a valid physical volume.").c_str());
    }
}

G4String JoinNames(const std::vector<G4String>& names) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < names.size(); ++index) {
        if (index != 0) {
            stream << ", ";
        }
        stream << names[index];
    }
    return stream.str();
}

std::vector<G4String> SortedKeys(const std::set<G4String>& names) {
    return std::vector<G4String>(names.begin(), names.end());
}

void CollectAvailableNamesFromLogical(const G4LogicalVolume* logicalVolume,
                                      std::set<G4String>& availableNames,
                                      std::set<const G4LogicalVolume*>& visitedLogicals) {
    if (logicalVolume == nullptr || !visitedLogicals.insert(logicalVolume).second) {
        return;
    }

    availableNames.insert(logicalVolume->GetName());

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        if (!daughter->GetName().empty()) {
            availableNames.insert(daughter->GetName());
        }
        CollectAvailableNamesFromLogical(daughter->GetLogicalVolume(), availableNames, visitedLogicals);
    }
}

MD1::GDMLAssemblyPart BuildPartFromPhysicalVolume(const G4VPhysicalVolume* physicalVolume,
                                                  const G4Transform3D& transform,
                                                  const G4String& gdmlPath) {
    ValidatePhysicalVolumeOrThrow(
        physicalVolume, "GDMLAssemblyReader::BuildPartFromPhysicalVolume", gdmlPath);
    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(
        logicalVolume, "GDMLAssemblyReader::BuildPartFromPhysicalVolume", gdmlPath);

    MD1::GDMLAssemblyPart part;
    part.name = physicalVolume->GetName();
    part.logicalVolume = logicalVolume;
    part.physicalVolume = physicalVolume;
    part.translation = transform.getTranslation();
    part.rotation = transform.getRotation();
    return part;
}

MD1::GDMLAssemblyPart BuildPartFromLogicalVolume(G4LogicalVolume* logicalVolume,
                                                 const G4Transform3D& transform,
                                                 const G4String& displayName,
                                                 const G4String& gdmlPath) {
    ValidateLogicalVolumeOrThrow(
        logicalVolume, "GDMLAssemblyReader::BuildPartFromLogicalVolume", gdmlPath);

    MD1::GDMLAssemblyPart part;
    part.name = displayName.empty() ? logicalVolume->GetName() : displayName;
    part.logicalVolume = logicalVolume;
    part.physicalVolume = nullptr;
    part.translation = transform.getTranslation();
    part.rotation = transform.getRotation();
    return part;
}

void FlattenAssemblyOrThrow(G4AssemblyVolume* assembly,
                            const G4Transform3D& parentTransform,
                            const G4String& gdmlPath,
                            std::vector<FlattenedAssemblyPart>& flattenedParts) {
    if (assembly == nullptr) {
        G4Exception("GDMLAssemblyReader::FlattenAssemblyOrThrow",
                    "ImportedGDMLAssemblyMissing",
                    FatalException,
                    ("Imported GDML " + gdmlPath + " referenced a null assembly volume.").c_str());
    }

    auto tripletIt = assembly->GetTripletsIterator();
    for (std::size_t index = 0; index < assembly->TotalTriplets(); ++index, ++tripletIt) {
        const auto& triplet = *tripletIt;
        if (triplet.IsReflection()) {
            G4Exception("GDMLAssemblyReader::FlattenAssemblyOrThrow",
                        "ImportedGDMLAssemblyReflectionUnsupported",
                        FatalException,
                        ("Imported GDML " + gdmlPath +
                         " contains a reflected assembly element, which model11 does not support.")
                            .c_str());
        }

        const G4RotationMatrix tripletRotation =
            (triplet.GetRotation() != nullptr) ? *triplet.GetRotation() : G4RotationMatrix();
        const G4Transform3D tripletTransform(tripletRotation, triplet.GetTranslation());
        const G4Transform3D currentTransform = parentTransform * tripletTransform;

        if (auto* logicalVolume = triplet.GetVolume(); logicalVolume != nullptr) {
            flattenedParts.push_back(FlattenedAssemblyPart{logicalVolume, currentTransform});
            continue;
        }

        if (auto* nestedAssembly = triplet.GetAssembly(); nestedAssembly != nullptr) {
            FlattenAssemblyOrThrow(nestedAssembly, currentTransform, gdmlPath, flattenedParts);
            continue;
        }

        G4Exception("GDMLAssemblyReader::FlattenAssemblyOrThrow",
                    "ImportedGDMLAssemblyElementInvalid",
                    FatalException,
                    ("Imported GDML " + gdmlPath +
                     " contains an assembly triplet with neither a logical volume nor a nested assembly.")
                        .c_str());
    }
}

BuiltAssemblyData BuildAssemblyFromPhysicalRoot(const MD1::MD1GDMLReadStructure::PhysicalVolumeMatch& rootMatch,
                                                const G4String& displayRootName,
                                                const G4String& gdmlPath) {
    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = displayRootName;
    builtAssembly.rootPhysicalVolumeName =
        (rootMatch.physicalVolume != nullptr) ? rootMatch.physicalVolume->GetName() : "";
    builtAssembly.parts.push_back(BuildPartFromPhysicalVolume(rootMatch.physicalVolume,
                                                              rootMatch.transform,
                                                              gdmlPath));

    std::set<const G4LogicalVolume*> visitedLogicals;
    CollectAvailableNamesFromLogical(rootMatch.physicalVolume->GetLogicalVolume(),
                                     builtAssembly.availableVolumeNames,
                                     visitedLogicals);
    if (rootMatch.physicalVolume != nullptr && !rootMatch.physicalVolume->GetName().empty()) {
        builtAssembly.availableVolumeNames.insert(rootMatch.physicalVolume->GetName());
    }

    return builtAssembly;
}

BuiltAssemblyData BuildAssemblyFromLogicalRoot(G4LogicalVolume* logicalVolume,
                                               const G4String& displayRootName,
                                               const G4String& gdmlPath) {
    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = displayRootName.empty() ? logicalVolume->GetName() : displayRootName;
    builtAssembly.rootPhysicalVolumeName = builtAssembly.rootVolumeName;
    builtAssembly.parts.push_back(BuildPartFromLogicalVolume(logicalVolume,
                                                             G4Transform3D(),
                                                             builtAssembly.rootVolumeName,
                                                             gdmlPath));

    std::set<const G4LogicalVolume*> visitedLogicals;
    CollectAvailableNamesFromLogical(logicalVolume, builtAssembly.availableVolumeNames, visitedLogicals);
    return builtAssembly;
}

BuiltAssemblyData BuildAssemblyFromFlattenedAssembly(
    const std::vector<FlattenedAssemblyPart>& flattenedParts,
    const G4String& assemblyName,
    const G4String& gdmlPath) {
    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = assemblyName;
    builtAssembly.rootPhysicalVolumeName = assemblyName;

    std::set<const G4LogicalVolume*> visitedLogicals;
    for (const auto& flattenedPart : flattenedParts) {
        builtAssembly.parts.push_back(BuildPartFromLogicalVolume(flattenedPart.logicalVolume,
                                                                 flattenedPart.transform,
                                                                 flattenedPart.logicalVolume->GetName(),
                                                                 gdmlPath));
        CollectAvailableNamesFromLogical(flattenedPart.logicalVolume,
                                         builtAssembly.availableVolumeNames,
                                         visitedLogicals);
    }

    return builtAssembly;
}

BuiltAssemblyData BuildAssemblyFromWorldTopLevelAssembly(const G4LogicalVolume* worldLogicalVolume,
                                                         const G4String& assemblyName,
                                                         const G4String& gdmlPath) {
    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = assemblyName;
    builtAssembly.rootPhysicalVolumeName = assemblyName;

    std::set<const G4LogicalVolume*> visitedLogicals;
    for (G4int daughterIndex = 0; daughterIndex < worldLogicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = worldLogicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        const G4Transform3D daughterTransform(daughter->GetObjectRotationValue(),
                                              daughter->GetObjectTranslation());
        builtAssembly.parts.push_back(BuildPartFromPhysicalVolume(daughter, daughterTransform, gdmlPath));
        if (!daughter->GetName().empty()) {
            builtAssembly.availableVolumeNames.insert(daughter->GetName());
        }
        CollectAvailableNamesFromLogical(daughter->GetLogicalVolume(),
                                         builtAssembly.availableVolumeNames,
                                         visitedLogicals);
    }

    return builtAssembly;
}

bool MatchesWorldTopLevelAssembly(const G4LogicalVolume* worldLogicalVolume,
                                  G4AssemblyVolume* assembly,
                                  const G4String& gdmlPath) {
    if (worldLogicalVolume == nullptr || assembly == nullptr) {
        return false;
    }

    std::vector<FlattenedAssemblyPart> flattenedParts;
    FlattenAssemblyOrThrow(assembly, G4Transform3D(), gdmlPath, flattenedParts);
    if (flattenedParts.size() != static_cast<std::size_t>(worldLogicalVolume->GetNoDaughters())) {
        return false;
    }

    std::multiset<G4LogicalVolume*> assemblyLogicals;
    std::multiset<G4LogicalVolume*> worldLogicals;
    for (const auto& part : flattenedParts) {
        assemblyLogicals.insert(part.logicalVolume);
    }
    for (G4int daughterIndex = 0; daughterIndex < worldLogicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* daughter = worldLogicalVolume->GetDaughter(daughterIndex);
        worldLogicals.insert((daughter != nullptr) ? daughter->GetLogicalVolume() : nullptr);
    }

    return assemblyLogicals == worldLogicals;
}

std::vector<G4String> FindTopLevelAssemblyCandidates(const MD1::MD1GDMLReadStructure& reader,
                                                     const G4LogicalVolume* worldLogicalVolume,
                                                     const G4String& gdmlPath) {
    std::vector<G4String> candidates;
    for (const auto& [assemblyName, assembly] : reader.GetAssemblyMap()) {
        if (MatchesWorldTopLevelAssembly(worldLogicalVolume, assembly, gdmlPath)) {
            candidates.push_back(assemblyName);
        }
    }

    std::sort(candidates.begin(), candidates.end());
    return candidates;
}

void ThrowRootNotFound(const G4String& gdmlPath,
                       const G4String& rootName,
                       const G4String& rootKind,
                       const std::vector<G4String>& candidates) {
    const G4String candidateMessage =
        candidates.empty() ? "" : (" Available " + rootKind + " roots: " + JoinNames(candidates) + ".");
    G4Exception("GDMLAssemblyReader::ReadAssembly",
                "ImportedGDMLRootNotFound",
                FatalException,
                ("Imported GDML " + gdmlPath + " does not contain a " + rootKind +
                 " root named '" + rootName + "'." + candidateMessage)
                    .c_str());
}

void ThrowTypedAmbiguity(const G4String& gdmlPath,
                         const G4String& rootName,
                         const G4String& rootKind,
                         const std::size_t matchCount) {
    G4Exception("GDMLAssemblyReader::ReadAssembly",
                "ImportedGDMLRootAmbiguous",
                FatalException,
                ("Imported GDML " + gdmlPath + " contains " + std::to_string(matchCount) + " " +
                 rootKind + " roots named '" + rootName + "'. Use a unique exact name.")
                    .c_str());
}

BuiltAssemblyData ResolveAutoRoot(const MD1::MD1GDMLReadStructure& reader,
                                  const G4LogicalVolume* worldLogicalVolume,
                                  const G4String& gdmlPath,
                                  G4String& resolvedRootName) {
    if (worldLogicalVolume->GetNoDaughters() == 1) {
        const auto* rootPhysical = worldLogicalVolume->GetDaughter(0);
        resolvedRootName = (rootPhysical != nullptr && rootPhysical->GetLogicalVolume() != nullptr)
                               ? rootPhysical->GetLogicalVolume()->GetName()
                               : "world_daughter";
        return BuildAssemblyFromPhysicalRoot(
            MD1::MD1GDMLReadStructure::PhysicalVolumeMatch{
                rootPhysical,
                G4Transform3D(rootPhysical->GetObjectRotationValue(), rootPhysical->GetObjectTranslation())},
            resolvedRootName,
            gdmlPath);
    }

    const auto topLevelAssemblyCandidates =
        FindTopLevelAssemblyCandidates(reader, worldLogicalVolume, gdmlPath);
    if (topLevelAssemblyCandidates.size() == 1) {
        resolvedRootName = topLevelAssemblyCandidates.front();
        return BuildAssemblyFromWorldTopLevelAssembly(worldLogicalVolume, resolvedRootName, gdmlPath);
    }

    const G4String assemblyHint =
        topLevelAssemblyCandidates.empty()
            ? ""
            : (" Matching top-level assemblies: " + JoinNames(topLevelAssemblyCandidates) + ".");
    G4Exception("GDMLAssemblyReader::ReadAssembly",
                "ImportedGDMLInvalidTopLevel",
                FatalException,
                ("Imported GDML " + gdmlPath +
                 " must define exactly one top-level daughter or one unique top-level assembly "
                 "when no explicit root is configured." + assemblyHint)
                    .c_str());
    return {};
}

BuiltAssemblyData ResolveExplicitAssemblyRoot(const MD1::MD1GDMLReadStructure& reader,
                                              const G4LogicalVolume* worldLogicalVolume,
                                              const G4String& gdmlPath,
                                              const G4String& assemblyName) {
    const auto matchingAssemblies = reader.FindAssemblies(assemblyName);
    if (matchingAssemblies.empty()) {
        ThrowRootNotFound(gdmlPath, assemblyName, "assembly", reader.GetAssemblyNames());
    }
    if (matchingAssemblies.size() > 1) {
        ThrowTypedAmbiguity(gdmlPath, assemblyName, "assembly", matchingAssemblies.size());
    }

    const auto topLevelAssemblyCandidates =
        FindTopLevelAssemblyCandidates(reader, worldLogicalVolume, gdmlPath);
    if (std::find(topLevelAssemblyCandidates.begin(),
                  topLevelAssemblyCandidates.end(),
                  assemblyName) != topLevelAssemblyCandidates.end()) {
        return BuildAssemblyFromWorldTopLevelAssembly(worldLogicalVolume, assemblyName, gdmlPath);
    }

    std::vector<FlattenedAssemblyPart> flattenedParts;
    FlattenAssemblyOrThrow(matchingAssemblies.front(), G4Transform3D(), gdmlPath, flattenedParts);
    return BuildAssemblyFromFlattenedAssembly(flattenedParts, assemblyName, gdmlPath);
}

void CollectAuxSensitiveNames(const MD1::GDMLImportedAssembly& assembly,
                              const G4LogicalVolume* logicalVolume,
                              std::set<G4String>& sensitiveNames,
                              std::set<const G4LogicalVolume*>& visitedLogicals) {
    if (logicalVolume == nullptr || !visitedLogicals.insert(logicalVolume).second) {
        return;
    }

    if (const auto* auxiliaries = assembly.GetAuxiliaryInfo(logicalVolume); auxiliaries != nullptr) {
        const auto hasSensDet =
            std::any_of(auxiliaries->begin(), auxiliaries->end(), [](const auto& aux) {
                return aux.type == "SensDet";
            });
        if (hasSensDet) {
            sensitiveNames.insert(logicalVolume->GetName());
        }
    }

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter != nullptr) {
            CollectAuxSensitiveNames(
                assembly, daughter->GetLogicalVolume(), sensitiveNames, visitedLogicals);
        }
    }
}

} // namespace

namespace MD1 {

std::vector<G4String> GDMLImportedAssembly::GetAuxSensitiveVolumeNames() const {
    std::set<G4String> sensitiveNames;
    std::set<const G4LogicalVolume*> visitedLogicals;
    for (const auto& part : fParts) {
        CollectAuxSensitiveNames(*this, part.logicalVolume, sensitiveNames, visitedLogicals);
    }

    return SortedKeys(sensitiveNames);
}

const G4GDMLAuxListType* GDMLImportedAssembly::GetAuxiliaryInfo(
    const G4LogicalVolume* logicalVolume) const {
    if (fReader == nullptr) {
        return nullptr;
    }

    return fReader->FindAuxiliaryInformation(logicalVolume);
}

GDMLImportedAssembly GDMLAssemblyReader::ReadAssembly(const G4String& gdmlPath,
                                                      const GDMLRootSelector& rootSelector,
                                                      const GDMLReadOptions& readOptions) {
    GDMLImportedAssembly assembly;
    assembly.fSourcePath = ResolveGDMLPathOrThrow(gdmlPath);
    assembly.fRootSelector = NormalizeRootSelector(rootSelector);
    const auto normalizedReadOptions = NormalizeReadOptions(readOptions);

    auto* reader = new MD1GDMLReadStructure();
    assembly.fReader = reader;
    assembly.fParser = std::make_shared<G4GDMLParser>(reader);
    if (!normalizedReadOptions.schemaPath.empty()) {
        assembly.fParser->SetImportSchema(normalizedReadOptions.schemaPath);
    }
    assembly.fParser->Read(assembly.fSourcePath, normalizedReadOptions.validate);

    auto* worldPhysicalVolume = assembly.fParser->GetWorldVolume();
    ValidatePhysicalVolumeOrThrow(worldPhysicalVolume,
                                  "GDMLAssemblyReader::ReadAssembly",
                                  assembly.fSourcePath);

    auto* worldLogicalVolume = worldPhysicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(worldLogicalVolume,
                                 "GDMLAssemblyReader::ReadAssembly",
                                 assembly.fSourcePath);

    reader->BuildInventory(worldPhysicalVolume);
    assembly.fAvailableLogicalVolumeNames = reader->GetLogicalVolumeNames();
    assembly.fAvailablePhysicalVolumeNames = reader->GetPhysicalVolumeNames();
    assembly.fAvailableAssemblyNames = reader->GetAssemblyNames();

    BuiltAssemblyData builtAssembly;
    G4String resolvedRootName = assembly.fRootSelector.name;
    if (assembly.fRootSelector.type == GDMLRootSelectorType::Auto && resolvedRootName.empty()) {
        builtAssembly = ResolveAutoRoot(*reader, worldLogicalVolume, assembly.fSourcePath, resolvedRootName);
    } else if (assembly.fRootSelector.type == GDMLRootSelectorType::Logical) {
        const auto& logicalMatches = reader->FindLogicalVolumes(resolvedRootName);
        if (logicalMatches.empty()) {
            ThrowRootNotFound(
                assembly.fSourcePath, resolvedRootName, "logical", reader->GetLogicalVolumeNames());
        }
        if (logicalMatches.size() > 1) {
            ThrowTypedAmbiguity(
                assembly.fSourcePath, resolvedRootName, "logical", logicalMatches.size());
        }
        builtAssembly =
            BuildAssemblyFromLogicalRoot(logicalMatches.front(), resolvedRootName, assembly.fSourcePath);
    } else if (assembly.fRootSelector.type == GDMLRootSelectorType::Physical) {
        const auto& physicalMatches = reader->FindPhysicalVolumes(resolvedRootName);
        if (physicalMatches.empty()) {
            ThrowRootNotFound(
                assembly.fSourcePath, resolvedRootName, "physical", reader->GetPhysicalVolumeNames());
        }
        if (physicalMatches.size() > 1) {
            ThrowTypedAmbiguity(
                assembly.fSourcePath, resolvedRootName, "physical", physicalMatches.size());
        }
        builtAssembly =
            BuildAssemblyFromPhysicalRoot(physicalMatches.front(), resolvedRootName, assembly.fSourcePath);
    } else if (assembly.fRootSelector.type == GDMLRootSelectorType::Assembly) {
        builtAssembly = ResolveExplicitAssemblyRoot(
            *reader, worldLogicalVolume, assembly.fSourcePath, resolvedRootName);
    } else {
        G4Exception("GDMLAssemblyReader::ReadAssembly",
                    "ImportedGDMLRootSelectorInvalid",
                    FatalException,
                    ("Imported GDML " + assembly.fSourcePath +
                     " received an unsupported auto root selector with explicit name '" +
                     resolvedRootName +
                     "'. Use logical, physical, or assembly root selection instead.")
                        .c_str());
    }

    assembly.fRootVolumeName =
        builtAssembly.rootVolumeName.empty() ? resolvedRootName : builtAssembly.rootVolumeName;
    assembly.fRootPhysicalVolumeName = builtAssembly.rootPhysicalVolumeName;
    assembly.fParts = std::move(builtAssembly.parts);
    assembly.fAvailableVolumeNames = SortedKeys(builtAssembly.availableVolumeNames);

    if (assembly.fParts.empty()) {
        G4Exception("GDMLAssemblyReader::ReadAssembly",
                    "ImportedGDMLEmptyAssembly",
                    FatalException,
                    ("Imported GDML " + assembly.fSourcePath +
                     " did not provide any reusable parts for model11.")
                        .c_str());
    }

    return assembly;
}

} // namespace MD1
