#include "geometry/gdml/MD1GDMLReadStructure.hh"

#include <algorithm>

#include "G4AssemblyVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

namespace {

template <typename TValue>
void SortNames(std::vector<TValue>& names) {
    std::sort(names.begin(), names.end());
}

} // namespace

namespace MD1 {

void MD1GDMLReadStructure::BuildInventory(const G4VPhysicalVolume* worldVolume) {
    ClearInventory();

    if (worldVolume == nullptr) {
        return;
    }

    std::set<const G4VPhysicalVolume*> visitedPhysicals;
    std::set<G4LogicalVolume*> visitedLogicals;
    Traverse(worldVolume, G4Transform3D(), visitedPhysicals, visitedLogicals);
}

const std::vector<G4LogicalVolume*>& MD1GDMLReadStructure::FindLogicalVolumes(const G4String& name) const {
    static const std::vector<G4LogicalVolume*> kEmpty;

    const auto it = fLogicalVolumesByName.find(name);
    return (it != fLogicalVolumesByName.end()) ? it->second : kEmpty;
}

const std::vector<MD1GDMLReadStructure::PhysicalVolumeMatch>&
MD1GDMLReadStructure::FindPhysicalVolumes(const G4String& name) const {
    static const std::vector<PhysicalVolumeMatch> kEmpty;

    const auto it = fPhysicalVolumesByName.find(name);
    return (it != fPhysicalVolumesByName.end()) ? it->second : kEmpty;
}

std::vector<G4AssemblyVolume*> MD1GDMLReadStructure::FindAssemblies(const G4String& name) const {
    const auto it = assemblyMap.find(name);
    if (it == assemblyMap.end() || it->second == nullptr) {
        return {};
    }

    return {it->second};
}

std::vector<G4String> MD1GDMLReadStructure::GetLogicalVolumeNames() const {
    std::vector<G4String> names;
    names.reserve(fLogicalVolumesByName.size());
    for (const auto& [name, volumes] : fLogicalVolumesByName) {
        if (!volumes.empty()) {
            names.push_back(name);
        }
    }

    SortNames(names);
    return names;
}

std::vector<G4String> MD1GDMLReadStructure::GetPhysicalVolumeNames() const {
    std::vector<G4String> names;
    names.reserve(fPhysicalVolumesByName.size());
    for (const auto& [name, volumes] : fPhysicalVolumesByName) {
        if (!volumes.empty()) {
            names.push_back(name);
        }
    }

    SortNames(names);
    return names;
}

std::vector<G4String> MD1GDMLReadStructure::GetAssemblyNames() const {
    std::vector<G4String> names;
    names.reserve(assemblyMap.size());
    for (const auto& [name, assembly] : assemblyMap) {
        if (assembly != nullptr) {
            names.push_back(name);
        }
    }

    SortNames(names);
    return names;
}

const G4GDMLAuxListType* MD1GDMLReadStructure::FindAuxiliaryInformation(
    const G4LogicalVolume* logicalVolume) const {
    if (logicalVolume == nullptr) {
        return nullptr;
    }

    const auto it = auxMap.find(const_cast<G4LogicalVolume*>(logicalVolume));
    return (it != auxMap.end()) ? &it->second : nullptr;
}

void MD1GDMLReadStructure::ClearInventory() {
    fLogicalVolumesByName.clear();
    fPhysicalVolumesByName.clear();
}

void MD1GDMLReadStructure::RegisterLogicalVolume(G4LogicalVolume* logicalVolume) {
    if (logicalVolume == nullptr) {
        return;
    }

    auto& bucket = fLogicalVolumesByName[logicalVolume->GetName()];
    if (std::find(bucket.begin(), bucket.end(), logicalVolume) == bucket.end()) {
        bucket.push_back(logicalVolume);
    }
}

void MD1GDMLReadStructure::RegisterPhysicalVolume(const G4VPhysicalVolume* physicalVolume,
                                                  const G4Transform3D& transform) {
    if (physicalVolume == nullptr) {
        return;
    }

    auto& bucket = fPhysicalVolumesByName[physicalVolume->GetName()];
    const auto it = std::find_if(bucket.begin(), bucket.end(), [physicalVolume](const auto& match) {
        return match.physicalVolume == physicalVolume;
    });
    if (it == bucket.end()) {
        bucket.push_back(PhysicalVolumeMatch{physicalVolume, transform});
    }
}

void MD1GDMLReadStructure::Traverse(const G4VPhysicalVolume* physicalVolume,
                                    const G4Transform3D& parentTransform,
                                    std::set<const G4VPhysicalVolume*>& visitedPhysicals,
                                    std::set<G4LogicalVolume*>& visitedLogicals) {
    if (physicalVolume == nullptr) {
        return;
    }

    const G4Transform3D localTransform(physicalVolume->GetObjectRotationValue(),
                                       physicalVolume->GetObjectTranslation());
    const G4Transform3D currentTransform = parentTransform * localTransform;
    RegisterPhysicalVolume(physicalVolume, currentTransform);

    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    if (logicalVolume == nullptr) {
        return;
    }

    RegisterLogicalVolume(logicalVolume);
    if (!visitedLogicals.insert(logicalVolume).second) {
        return;
    }

    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        if (!visitedPhysicals.insert(daughter).second) {
            continue;
        }

        Traverse(daughter, currentTransform, visitedPhysicals, visitedLogicals);
    }
}

} // namespace MD1
