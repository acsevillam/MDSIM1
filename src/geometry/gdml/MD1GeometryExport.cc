#include "geometry/gdml/MD1GeometryExport.hh"

#include <algorithm>
#include <array>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4NistManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4StateManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VSolid.hh"
#include "G4VisAttributes.hh"

#include "geometry/gdml/GDMLColorCodec.hh"
#include "geometry/gdml/GeometryAuxiliaryRegistry.hh"
#include "geometry/gdml/MD1GeometryExportMessenger.hh"

namespace {

namespace fs = std::filesystem;

struct Bounds3D {
    G4ThreeVector min;
    G4ThreeVector max;
    G4bool valid = false;
};

G4String TrimValue(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

G4bool MatchesPattern(const G4String& value, const G4String& pattern) {
    return pattern.empty() || value.find(pattern) != G4String::npos;
}

G4String NormalizeOutputPath(const G4String& outputPath) {
    const fs::path requestedPath(outputPath.c_str());
    return fs::absolute(requestedPath).lexically_normal().string();
}

void EnsureDirectoryExists(const G4String& outputPath) {
    const fs::path path(outputPath.c_str());
    const auto parentPath = path.parent_path();
    if (parentPath.empty()) {
        return;
    }

    std::error_code errorCode;
    fs::create_directories(parentPath, errorCode);
    if (errorCode) {
        G4Exception("MD1GeometryExport::EnsureDirectoryExists",
                    "GeometryExportOutputDirectoryFailed",
                    FatalException,
                    ("Could not create GDML export directory " + parentPath.string() + ": " +
                     errorCode.message())
                        .c_str());
    }
}

void ExpandBounds(Bounds3D& bounds, const G4ThreeVector& point) {
    if (!bounds.valid) {
        bounds.min = point;
        bounds.max = point;
        bounds.valid = true;
        return;
    }

    bounds.min.setX(std::min(bounds.min.x(), point.x()));
    bounds.min.setY(std::min(bounds.min.y(), point.y()));
    bounds.min.setZ(std::min(bounds.min.z(), point.z()));
    bounds.max.setX(std::max(bounds.max.x(), point.x()));
    bounds.max.setY(std::max(bounds.max.y(), point.y()));
    bounds.max.setZ(std::max(bounds.max.z(), point.z()));
}

void ExpandBoundsWithSolid(const G4VSolid* solid,
                           const G4Transform3D& transform,
                           Bounds3D& bounds) {
    if (solid == nullptr) {
        return;
    }

    G4ThreeVector localMin;
    G4ThreeVector localMax;
    solid->BoundingLimits(localMin, localMax);

    const std::array<G4double, 2> xs = {localMin.x(), localMax.x()};
    const std::array<G4double, 2> ys = {localMin.y(), localMax.y()};
    const std::array<G4double, 2> zs = {localMin.z(), localMax.z()};
    for (const auto x : xs) {
        for (const auto y : ys) {
            for (const auto z : zs) {
                ExpandBounds(bounds,
                             transform.getTranslation() +
                                 transform.getRotation() * G4ThreeVector(x, y, z));
            }
        }
    }
}

void AccumulateSubtreeBounds(const G4LogicalVolume* logicalVolume,
                             const G4Transform3D& transform,
                             Bounds3D& bounds) {
    if (logicalVolume == nullptr) {
        return;
    }

    ExpandBoundsWithSolid(logicalVolume->GetSolid(), transform, bounds);
    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        const G4Transform3D daughterTransform(daughter->GetObjectRotationValue(),
                                              daughter->GetObjectTranslation());
        AccumulateSubtreeBounds(daughter->GetLogicalVolume(), transform * daughterTransform, bounds);
    }
}

G4double ComputeHalfLength(const G4double minValue, const G4double maxValue) {
    const G4double span = std::max(0.0, maxValue - minValue);
    const G4double margin = std::max(1.0 * mm, 0.05 * span);
    return std::max(1.0 * mm, 0.5 * span + margin);
}

G4bool HasAuxiliaryType(const G4GDMLAuxListType& auxiliaries, const G4String& type) {
    return std::any_of(auxiliaries.begin(), auxiliaries.end(), [&type](const auto& aux) {
        return aux.type == type;
    });
}

G4GDMLAuxListType BuildExportAuxiliaries(const G4LogicalVolume* logicalVolume) {
    G4GDMLAuxListType auxiliaries;
    if (const auto* registeredAuxiliaries =
            MD1::GeometryAuxiliaryRegistry::GetInstance()->Find(logicalVolume);
        registeredAuxiliaries != nullptr) {
        auxiliaries = MD1::GDMLColorCodec::CopyAuxiliariesWithoutColor(*registeredAuxiliaries);
    }

    if (const auto* visAttributes = logicalVolume->GetVisAttributes(); visAttributes != nullptr) {
        auxiliaries.push_back(
            G4GDMLAuxStructType{
                "Color", MD1::GDMLColorCodec::EncodeColor(visAttributes->GetColour()), "", nullptr});
    }

    if (const auto* sensitiveDetector = logicalVolume->GetSensitiveDetector();
        sensitiveDetector != nullptr && !HasAuxiliaryType(auxiliaries, "SensDet")) {
        auxiliaries.push_back(
            G4GDMLAuxStructType{"SensDet", sensitiveDetector->GetName(), "", nullptr});
    }

    return auxiliaries;
}

void RegisterAuxiliariesRecursive(G4GDMLParser& parser,
                                  const G4LogicalVolume* logicalVolume,
                                  std::set<const G4LogicalVolume*>& visitedLogicals) {
    if (logicalVolume == nullptr || !visitedLogicals.insert(logicalVolume).second) {
        return;
    }

    const auto auxiliaries = BuildExportAuxiliaries(logicalVolume);
    for (const auto& auxiliary : auxiliaries) {
        parser.AddVolumeAuxiliary(auxiliary, const_cast<G4LogicalVolume*>(logicalVolume));
    }

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter != nullptr) {
            RegisterAuxiliariesRecursive(parser, daughter->GetLogicalVolume(), visitedLogicals);
        }
    }
}

const G4VPhysicalVolume* ResolveUniquePhysicalVolume(const G4String& volumeName, const G4int copyNo) {
    auto* physicalStore = G4PhysicalVolumeStore::GetInstance();
    if (!physicalStore->IsMapValid()) {
        physicalStore->UpdateMap();
    }

    const auto& volumeMap = physicalStore->GetMap();
    const auto volumeIt = volumeMap.find(volumeName);
    if (volumeIt == volumeMap.end()) {
        G4Exception("MD1GeometryExport::ResolveUniquePhysicalVolume",
                    "GeometryExportPhysicalVolumeNotFound",
                    FatalException,
                    ("No physical volume named '" + volumeName + "' is currently registered.")
                        .c_str());
    }

    std::vector<const G4VPhysicalVolume*> matchingVolumes;
    for (const auto* physicalVolume : volumeIt->second) {
        if (physicalVolume != nullptr && physicalVolume->GetCopyNo() == copyNo) {
            matchingVolumes.push_back(physicalVolume);
        }
    }

    if (matchingVolumes.empty()) {
        std::ostringstream detail;
        detail << "No physical volume named '" << volumeName << "' with copyNo " << copyNo
               << " is currently registered.";
        if (!volumeIt->second.empty()) {
            detail << " Available copyNos:";
            for (const auto* physicalVolume : volumeIt->second) {
                if (physicalVolume != nullptr) {
                    detail << " " << physicalVolume->GetCopyNo();
                }
            }
        }
        G4Exception("MD1GeometryExport::ResolveUniquePhysicalVolume",
                    "GeometryExportPhysicalVolumeNotFound",
                    FatalException,
                    detail.str().c_str());
    }

    if (matchingVolumes.size() > 1) {
        std::ostringstream detail;
        detail << "Physical volume selection '" << volumeName << "' copyNo " << copyNo
               << " is still ambiguous. Matching logicals:";
        for (const auto* physicalVolume : matchingVolumes) {
            detail << " " << physicalVolume->GetLogicalVolume()->GetName();
        }
        G4Exception("MD1GeometryExport::ResolveUniquePhysicalVolume",
                    "GeometryExportPhysicalVolumeAmbiguous",
                    FatalException,
                    detail.str().c_str());
    }

    return matchingVolumes.front();
}

G4LogicalVolume* ResolveUniqueLogicalVolume(const G4String& volumeName) {
    auto* logicalStore = G4LogicalVolumeStore::GetInstance();
    if (!logicalStore->IsMapValid()) {
        logicalStore->UpdateMap();
    }

    const auto& volumeMap = logicalStore->GetMap();
    const auto volumeIt = volumeMap.find(volumeName);
    if (volumeIt == volumeMap.end() || volumeIt->second.empty()) {
        G4Exception("MD1GeometryExport::ResolveUniqueLogicalVolume",
                    "GeometryExportLogicalVolumeNotFound",
                    FatalException,
                    ("No logical volume named '" + volumeName + "' is currently registered.")
                        .c_str());
    }

    if (volumeIt->second.size() > 1) {
        G4Exception("MD1GeometryExport::ResolveUniqueLogicalVolume",
                    "GeometryExportLogicalVolumeAmbiguous",
                    FatalException,
                    ("Logical volume selection '" + volumeName +
                     "' is not unique. Use /MultiDetector1/geometry/writePhysicalGDML instead.")
                        .c_str());
    }

    return volumeIt->second.front();
}

void WriteGDMLForRoot(G4LogicalVolume* rootLogicalVolume,
                      const G4String& rootPhysicalName,
                      const G4int rootCopyNo,
                      const G4String& outputPath) {
    if (rootLogicalVolume == nullptr) {
        G4Exception("MD1GeometryExport::WriteGDMLForRoot",
                    "GeometryExportNullRoot",
                    FatalException,
                    "Cannot export a null logical volume.");
    }

    Bounds3D bounds;
    AccumulateSubtreeBounds(rootLogicalVolume, G4Transform3D(), bounds);
    if (!bounds.valid) {
        bounds.min = G4ThreeVector(-1.0 * mm, -1.0 * mm, -1.0 * mm);
        bounds.max = G4ThreeVector(1.0 * mm, 1.0 * mm, 1.0 * mm);
        bounds.valid = true;
    }

    const G4double halfX = ComputeHalfLength(bounds.min.x(), bounds.max.x());
    const G4double halfY = ComputeHalfLength(bounds.min.y(), bounds.max.y());
    const G4double halfZ = ComputeHalfLength(bounds.min.z(), bounds.max.z());

    const auto normalizedOutputPath = NormalizeOutputPath(outputPath);
    EnsureDirectoryExists(normalizedOutputPath);

    auto* galactic = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    auto* worldSolid = new G4Box("MD1ExportWorldSolid", halfX, halfY, halfZ);
    auto* worldLogical = new G4LogicalVolume(worldSolid, galactic, "MD1ExportWorldLogical");
    auto* worldPhysical = new G4PVPlacement(nullptr,
                                            G4ThreeVector(),
                                            worldLogical,
                                            "MD1ExportWorldPhysical",
                                            nullptr,
                                            false,
                                            0,
                                            false);
    auto* rootPlacement = new G4PVPlacement(nullptr,
                                            G4ThreeVector(),
                                            rootLogicalVolume,
                                            rootPhysicalName,
                                            worldLogical,
                                            false,
                                            rootCopyNo,
                                            false);

    G4GDMLParser parser;
    parser.SetOutputFileOverwrite(true);
    parser.SetAddPointerToName(false);
    parser.SetSDExport(true);

    std::set<const G4LogicalVolume*> visitedLogicals;
    RegisterAuxiliariesRecursive(parser, rootLogicalVolume, visitedLogicals);
    parser.Write(normalizedOutputPath, worldLogical);

    delete rootPlacement;
    delete worldPhysical;
    delete worldLogical;
    delete worldSolid;

    G4cout << "GDML export written to " << normalizedOutputPath
           << " with root physical '" << rootPhysicalName
           << "' copyNo=" << rootCopyNo << G4endl;
}

} // namespace

namespace MD1 {

MD1GeometryExport* MD1GeometryExport::fInstance = nullptr;

MD1GeometryExport* MD1GeometryExport::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new MD1GeometryExport();
    }

    return fInstance;
}

void MD1GeometryExport::Kill() {
    delete fInstance;
    fInstance = nullptr;
}

MD1GeometryExport::MD1GeometryExport()
    : fMessenger(new MD1GeometryExportMessenger(this)) {}

MD1GeometryExport::~MD1GeometryExport() {
    delete fMessenger;
    fMessenger = nullptr;
}

void MD1GeometryExport::ListPhysicalVolumes(const G4String& pattern) const {
    auto* physicalStore = G4PhysicalVolumeStore::GetInstance();
    if (!physicalStore->IsMapValid()) {
        physicalStore->UpdateMap();
    }

    const auto filter = TrimValue(pattern);
    G4cout << "Registered physical volumes:" << G4endl;
    for (const auto& [volumeName, physicalVolumes] : physicalStore->GetMap()) {
        if (!MatchesPattern(volumeName, filter)) {
            continue;
        }

        for (const auto* physicalVolume : physicalVolumes) {
            if (physicalVolume == nullptr) {
                continue;
            }

            const auto* motherLogical = physicalVolume->GetMotherLogical();
            G4cout << " - " << physicalVolume->GetName()
                   << " copyNo=" << physicalVolume->GetCopyNo()
                   << " logical=" << physicalVolume->GetLogicalVolume()->GetName()
                   << " mother=" << ((motherLogical != nullptr) ? motherLogical->GetName() : "none")
                   << G4endl;
        }
    }
}

void MD1GeometryExport::ListLogicalVolumes(const G4String& pattern) const {
    auto* logicalStore = G4LogicalVolumeStore::GetInstance();
    if (!logicalStore->IsMapValid()) {
        logicalStore->UpdateMap();
    }

    const auto filter = TrimValue(pattern);
    G4cout << "Registered logical volumes:" << G4endl;
    for (const auto& [volumeName, logicalVolumes] : logicalStore->GetMap()) {
        if (!MatchesPattern(volumeName, filter)) {
            continue;
        }

        G4cout << " - " << volumeName << " count=" << logicalVolumes.size() << G4endl;
    }
}

void MD1GeometryExport::WritePhysicalGDML(const G4String& volumeName,
                                          G4int copyNo,
                                          const G4String& outputPath) const {
    const auto trimmedName = TrimValue(volumeName);
    const auto trimmedOutputPath = TrimValue(outputPath);
    const auto* physicalVolume = ResolveUniquePhysicalVolume(trimmedName, copyNo);
    WriteGDMLForRoot(physicalVolume->GetLogicalVolume(),
                     physicalVolume->GetName(),
                     physicalVolume->GetCopyNo(),
                     trimmedOutputPath);
}

void MD1GeometryExport::WriteLogicalGDML(const G4String& volumeName,
                                         const G4String& outputPath) const {
    const auto trimmedName = TrimValue(volumeName);
    const auto trimmedOutputPath = TrimValue(outputPath);
    auto* logicalVolume = ResolveUniqueLogicalVolume(trimmedName);
    WriteGDMLForRoot(logicalVolume, logicalVolume->GetName(), 0, trimmedOutputPath);
}

} // namespace MD1
