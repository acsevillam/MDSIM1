#include "geometry/gdml/GDMLAssemblyReader.hh"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

namespace {

namespace fs = std::filesystem;

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

G4String TrimName(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string ReadTextFileOrThrow(const G4String& gdmlPath) {
    std::ifstream input(gdmlPath.c_str());
    if (!input.is_open()) {
        G4Exception("GDMLAssemblyReader::ReadTextFileOrThrow",
                    "ImportedGDMLFileNotReadable",
                    FatalException,
                    ("Could not open imported GDML file for text inspection: " + gdmlPath).c_str());
    }

    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::string EscapeRegexLiteral(const G4String& value) {
    std::string escaped;
    escaped.reserve(value.size() * 2);
    for (const auto ch : value) {
        switch (ch) {
            case '\\':
            case '^':
            case '$':
            case '.':
            case '|':
            case '?':
            case '*':
            case '+':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
                escaped.push_back('\\');
                break;
            default:
                break;
        }
        escaped.push_back(ch);
    }
    return escaped;
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

MD1::GDMLAssemblyPart BuildPartFromPhysicalVolumeOrThrow(const G4VPhysicalVolume* physicalVolume,
                                                         const G4String& gdmlPath) {
    ValidatePhysicalVolumeOrThrow(
        physicalVolume, "GDMLAssemblyReader::BuildPartFromPhysicalVolumeOrThrow", gdmlPath);

    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(
        logicalVolume, "GDMLAssemblyReader::BuildPartFromPhysicalVolumeOrThrow", gdmlPath);

    MD1::GDMLAssemblyPart part;
    part.name = physicalVolume->GetName();
    part.logicalVolume = logicalVolume;
    part.physicalVolume = physicalVolume;
    part.translation = physicalVolume->GetObjectTranslation();
    part.rotation = physicalVolume->GetObjectRotationValue();
    return part;
}

struct RootCandidate {
    const G4VPhysicalVolume* physicalVolume = nullptr;
    G4ThreeVector translation;
    G4RotationMatrix rotation;
};

struct BuiltAssemblyData {
    G4String rootVolumeName;
    G4String rootPhysicalVolumeName;
    std::vector<MD1::GDMLAssemblyPart> parts;
    std::set<G4String> availableVolumeNames;
};

struct GDMLWorldPhysvolHint {
    G4String name;
    G4String volumeRef;
};

struct GDMLStructureHint {
    G4String worldVolumeName;
    std::vector<GDMLWorldPhysvolHint> worldPhysvols;
    std::set<G4String> assemblyNames;
};

void CollectAvailableVolumeNames(const G4VPhysicalVolume* physicalVolume,
                                 std::set<G4String>& availableVolumeNames) {
    ValidatePhysicalVolumeOrThrow(
        physicalVolume, "GDMLAssemblyReader::CollectAvailableVolumeNames", "unknown");

    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(
        logicalVolume, "GDMLAssemblyReader::CollectAvailableVolumeNames", "unknown");

    availableVolumeNames.insert(logicalVolume->GetName());
    if (!physicalVolume->GetName().empty()) {
        availableVolumeNames.insert(physicalVolume->GetName());
    }

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        CollectAvailableVolumeNames(logicalVolume->GetDaughter(daughterIndex), availableVolumeNames);
    }
}

void CollectRootCandidates(const G4VPhysicalVolume* physicalVolume,
                           const G4String& targetName,
                           const G4ThreeVector& parentTranslation,
                           const G4RotationMatrix& parentRotation,
                           std::vector<RootCandidate>& matches) {
    ValidatePhysicalVolumeOrThrow(
        physicalVolume, "GDMLAssemblyReader::CollectRootCandidates", targetName);

    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(
        logicalVolume, "GDMLAssemblyReader::CollectRootCandidates", targetName);

    const auto localTranslation = physicalVolume->GetObjectTranslation();
    const auto localRotation = physicalVolume->GetObjectRotationValue();
    const auto currentTranslation = parentRotation * localTranslation + parentTranslation;
    const auto currentRotation = parentRotation * localRotation;

    if (physicalVolume->GetName() == targetName || logicalVolume->GetName() == targetName) {
        matches.push_back(RootCandidate{physicalVolume, currentTranslation, currentRotation});
    }

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        CollectRootCandidates(logicalVolume->GetDaughter(daughterIndex),
                              targetName,
                              currentTranslation,
                              currentRotation,
                              matches);
    }
}

GDMLStructureHint ParseGDMLStructureHint(const G4String& gdmlPath) {
    const std::string text = ReadTextFileOrThrow(gdmlPath);
    GDMLStructureHint hint;

    const std::regex setupWorldRegex(R"(<setup\b[\s\S]*?<world\b[^>]*ref=['"]([^'"]+)['"])",
                                     std::regex::icase);
    std::smatch setupMatch;
    if (!std::regex_search(text, setupMatch, setupWorldRegex) || setupMatch.size() < 2) {
        return hint;
    }
    hint.worldVolumeName = setupMatch[1].str();

    const std::regex assemblyRegex(R"(<assembly\b[^>]*name=['"]([^'"]+)['"])", std::regex::icase);
    for (std::sregex_iterator it(text.begin(), text.end(), assemblyRegex), end; it != end; ++it) {
        if ((*it).size() >= 2) {
            hint.assemblyNames.insert((*it)[1].str());
        }
    }

    const std::regex worldBlockRegex(
        "<volume\\b[^>]*name=['\"]" + EscapeRegexLiteral(hint.worldVolumeName) + "['\"][^>]*>([\\s\\S]*?)</volume>",
        std::regex::icase);
    std::smatch worldBlockMatch;
    if (!std::regex_search(text, worldBlockMatch, worldBlockRegex) || worldBlockMatch.size() < 2) {
        return hint;
    }

    const auto worldBlock = worldBlockMatch[1].str();
    const std::regex physvolRegex(R"(<physvol\b([^>]*)>([\s\S]*?)</physvol>)", std::regex::icase);
    const std::regex nameRegex(R"(name=['"]([^'"]+)['"])", std::regex::icase);
    const std::regex volumerefRegex(R"(<volumeref\b[^>]*ref=['"]([^'"]+)['"])", std::regex::icase);

    for (std::sregex_iterator it(worldBlock.begin(), worldBlock.end(), physvolRegex), end; it != end; ++it) {
        GDMLWorldPhysvolHint physvolHint;
        const auto attributes = (*it)[1].str();
        const auto body = (*it)[2].str();

        std::smatch attributeMatch;
        if (std::regex_search(attributes, attributeMatch, nameRegex) && attributeMatch.size() >= 2) {
            physvolHint.name = attributeMatch[1].str();
        }

        std::smatch volumerefMatch;
        if (std::regex_search(body, volumerefMatch, volumerefRegex) && volumerefMatch.size() >= 2) {
            physvolHint.volumeRef = volumerefMatch[1].str();
        }

        if (!physvolHint.volumeRef.empty()) {
            hint.worldPhysvols.push_back(std::move(physvolHint));
        }
    }

    return hint;
}

const GDMLWorldPhysvolHint* FindSingleTopLevelAssemblyHint(const GDMLStructureHint& hint) {
    if (hint.worldPhysvols.size() != 1) {
        return nullptr;
    }

    const auto& worldPhysvol = hint.worldPhysvols.front();
    if (hint.assemblyNames.find(worldPhysvol.volumeRef) == hint.assemblyNames.end()) {
        return nullptr;
    }

    return &worldPhysvol;
}

BuiltAssemblyData BuildAssemblyFromRoot(const G4VPhysicalVolume* rootPhysicalVolume,
                                        const G4ThreeVector& rootTranslation,
                                        const G4RotationMatrix& rootRotation,
                                        const G4String& gdmlPath) {
    ValidatePhysicalVolumeOrThrow(
        rootPhysicalVolume, "GDMLAssemblyReader::BuildAssemblyFromRoot", gdmlPath);

    auto* rootLogicalVolume = rootPhysicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(
        rootLogicalVolume, "GDMLAssemblyReader::BuildAssemblyFromRoot", gdmlPath);

    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = rootLogicalVolume->GetName();
    builtAssembly.rootPhysicalVolumeName = rootPhysicalVolume->GetName();
    CollectAvailableVolumeNames(rootPhysicalVolume, builtAssembly.availableVolumeNames);
    auto rootPart = BuildPartFromPhysicalVolumeOrThrow(rootPhysicalVolume, gdmlPath);
    rootPart.name = rootLogicalVolume->GetName();
    rootPart.translation = rootTranslation;
    rootPart.rotation = rootRotation;
    builtAssembly.parts.push_back(std::move(rootPart));

    return builtAssembly;
}

BuiltAssemblyData BuildAssemblyFromCandidates(const std::vector<RootCandidate>& rootCandidates,
                                              const G4String& rootVolumeName,
                                              const G4String& rootPhysicalVolumeName,
                                              const G4String& gdmlPath) {
    BuiltAssemblyData builtAssembly;
    builtAssembly.rootVolumeName = rootVolumeName;
    builtAssembly.rootPhysicalVolumeName = rootPhysicalVolumeName;

    for (const auto& candidate : rootCandidates) {
        auto part = BuildPartFromPhysicalVolumeOrThrow(candidate.physicalVolume, gdmlPath);
        part.translation = candidate.translation;
        part.rotation = candidate.rotation;
        builtAssembly.parts.push_back(std::move(part));
        CollectAvailableVolumeNames(candidate.physicalVolume, builtAssembly.availableVolumeNames);
    }

    if (!rootVolumeName.empty()) {
        builtAssembly.availableVolumeNames.insert(rootVolumeName);
    }
    if (!rootPhysicalVolumeName.empty()) {
        builtAssembly.availableVolumeNames.insert(rootPhysicalVolumeName);
    }

    return builtAssembly;
}

} // namespace

namespace MD1 {

GDMLImportedAssembly GDMLAssemblyReader::ReadAssembly(const G4String& gdmlPath,
                                                      const G4String& rootName) {
    GDMLImportedAssembly assembly;
    assembly.fSourcePath = ResolveGDMLPathOrThrow(gdmlPath);
    assembly.fParser = std::make_shared<G4GDMLParser>();
    assembly.fParser->Read(assembly.fSourcePath, false);

    auto* worldPhysicalVolume = assembly.fParser->GetWorldVolume();
    ValidatePhysicalVolumeOrThrow(worldPhysicalVolume,
                                  "GDMLAssemblyReader::ReadAssembly",
                                  assembly.fSourcePath);

    auto* worldLogicalVolume = worldPhysicalVolume->GetLogicalVolume();
    ValidateLogicalVolumeOrThrow(worldLogicalVolume,
                                 "GDMLAssemblyReader::ReadAssembly",
                                 assembly.fSourcePath);

    const auto structureHint = ParseGDMLStructureHint(assembly.fSourcePath);
    const auto* singleTopLevelAssemblyHint = FindSingleTopLevelAssemblyHint(structureHint);
    const auto requestedRootName = TrimName(rootName);
    if (requestedRootName.empty()) {
        if (worldLogicalVolume->GetNoDaughters() == 1) {
            auto* rootPhysicalVolume = worldLogicalVolume->GetDaughter(0);
            auto builtAssembly = BuildAssemblyFromRoot(rootPhysicalVolume,
                                                       rootPhysicalVolume->GetObjectTranslation(),
                                                       rootPhysicalVolume->GetObjectRotationValue(),
                                                       assembly.fSourcePath);
            assembly.fRootVolumeName = std::move(builtAssembly.rootVolumeName);
            assembly.fRootPhysicalVolumeName = std::move(builtAssembly.rootPhysicalVolumeName);
            assembly.fParts = std::move(builtAssembly.parts);
            assembly.fAvailableVolumeNames.assign(builtAssembly.availableVolumeNames.begin(),
                                                  builtAssembly.availableVolumeNames.end());
        } else if (singleTopLevelAssemblyHint != nullptr) {
            std::vector<RootCandidate> topLevelCandidates;
            for (G4int daughterIndex = 0; daughterIndex < worldLogicalVolume->GetNoDaughters(); ++daughterIndex) {
                auto* worldDaughter = worldLogicalVolume->GetDaughter(daughterIndex);
                topLevelCandidates.push_back(
                    RootCandidate{worldDaughter,
                                  worldDaughter->GetObjectTranslation(),
                                  worldDaughter->GetObjectRotationValue()});
            }

            auto builtAssembly = BuildAssemblyFromCandidates(topLevelCandidates,
                                                             singleTopLevelAssemblyHint->volumeRef,
                                                             singleTopLevelAssemblyHint->name,
                                                             assembly.fSourcePath);
            assembly.fRootVolumeName = std::move(builtAssembly.rootVolumeName);
            assembly.fRootPhysicalVolumeName = std::move(builtAssembly.rootPhysicalVolumeName);
            assembly.fParts = std::move(builtAssembly.parts);
            assembly.fAvailableVolumeNames.assign(builtAssembly.availableVolumeNames.begin(),
                                                  builtAssembly.availableVolumeNames.end());
        } else {
            G4Exception("GDMLAssemblyReader::ReadAssembly",
                        "ImportedGDMLInvalidTopLevel",
                        FatalException,
                        ("Imported GDML " + assembly.fSourcePath +
                         " must define exactly one top-level daughter inside the world volume "
                         "when no explicit root name is configured.").c_str());
        }
    } else {
        std::vector<RootCandidate> matches;
        for (G4int daughterIndex = 0; daughterIndex < worldLogicalVolume->GetNoDaughters(); ++daughterIndex) {
            CollectRootCandidates(worldLogicalVolume->GetDaughter(daughterIndex),
                                  requestedRootName,
                                  G4ThreeVector(),
                                  G4RotationMatrix(),
                                  matches);
        }

        if (matches.empty()) {
            if (singleTopLevelAssemblyHint != nullptr &&
                (requestedRootName == singleTopLevelAssemblyHint->volumeRef ||
                 requestedRootName == singleTopLevelAssemblyHint->name)) {
                std::vector<RootCandidate> topLevelCandidates;
                for (G4int daughterIndex = 0; daughterIndex < worldLogicalVolume->GetNoDaughters();
                     ++daughterIndex) {
                    auto* worldDaughter = worldLogicalVolume->GetDaughter(daughterIndex);
                    topLevelCandidates.push_back(
                        RootCandidate{worldDaughter,
                                      worldDaughter->GetObjectTranslation(),
                                      worldDaughter->GetObjectRotationValue()});
                }

                auto builtAssembly = BuildAssemblyFromCandidates(topLevelCandidates,
                                                                 singleTopLevelAssemblyHint->volumeRef,
                                                                 singleTopLevelAssemblyHint->name,
                                                                 assembly.fSourcePath);
                assembly.fRootVolumeName = std::move(builtAssembly.rootVolumeName);
                assembly.fRootPhysicalVolumeName = std::move(builtAssembly.rootPhysicalVolumeName);
                assembly.fParts = std::move(builtAssembly.parts);
                assembly.fAvailableVolumeNames.assign(builtAssembly.availableVolumeNames.begin(),
                                                      builtAssembly.availableVolumeNames.end());
            } else {
                const G4String assemblyHintMessage =
                    (singleTopLevelAssemblyHint != nullptr)
                        ? (" Known top-level assembly aliases: '" + singleTopLevelAssemblyHint->volumeRef +
                           "' and '" + singleTopLevelAssemblyHint->name + "'.")
                        : "";
                G4Exception("GDMLAssemblyReader::ReadAssembly",
                            "ImportedGDMLRootNotFound",
                            FatalException,
                            ("Imported GDML " + assembly.fSourcePath +
                             " does not contain a unique physical or logical volume named '" +
                             requestedRootName + "'." + assemblyHintMessage)
                                .c_str());
            }
        }

        if (!matches.empty() && matches.size() > 1) {
            G4Exception("GDMLAssemblyReader::ReadAssembly",
                        "ImportedGDMLRootAmbiguous",
                        FatalException,
                        ("Imported GDML " + assembly.fSourcePath +
                         " contains multiple physical/logical volumes named '" +
                         requestedRootName + "'. Use a unique root name.").c_str());
        }

        if (!matches.empty()) {
            const auto& selectedRoot = matches.front();
            auto builtAssembly = BuildAssemblyFromRoot(selectedRoot.physicalVolume,
                                                       selectedRoot.translation,
                                                       selectedRoot.rotation,
                                                       assembly.fSourcePath);
            assembly.fRootVolumeName = std::move(builtAssembly.rootVolumeName);
            assembly.fRootPhysicalVolumeName = std::move(builtAssembly.rootPhysicalVolumeName);
            assembly.fParts = std::move(builtAssembly.parts);
            assembly.fAvailableVolumeNames.assign(builtAssembly.availableVolumeNames.begin(),
                                                  builtAssembly.availableVolumeNames.end());
        }
    }

    if (assembly.fParts.empty()) {
        G4Exception("GDMLAssemblyReader::ReadAssembly",
                    "ImportedGDMLEmptyAssembly",
                    FatalException,
                    ("Imported GDML " + assembly.fSourcePath +
                     " did not provide any reusable passive parts.").c_str());
    }

    return assembly;
}

} // namespace MD1
