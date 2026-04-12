#include "geometry/gdml/GeometryAuxiliaryRegistry.hh"

namespace MD1 {

GeometryAuxiliaryRegistry* GeometryAuxiliaryRegistry::fInstance = nullptr;

GeometryAuxiliaryRegistry* GeometryAuxiliaryRegistry::GetInstance() {
    if (fInstance == nullptr) {
        fInstance = new GeometryAuxiliaryRegistry();
    }

    return fInstance;
}

void GeometryAuxiliaryRegistry::Kill() {
    delete fInstance;
    fInstance = nullptr;
}

void GeometryAuxiliaryRegistry::Register(const G4LogicalVolume* logicalVolume,
                                         const G4GDMLAuxListType& auxiliaries) {
    if (logicalVolume == nullptr || auxiliaries.empty()) {
        return;
    }

    fAuxiliariesByLogicalVolume[logicalVolume] = auxiliaries;
}

void GeometryAuxiliaryRegistry::Unregister(const G4LogicalVolume* logicalVolume) {
    if (logicalVolume == nullptr) {
        return;
    }

    fAuxiliariesByLogicalVolume.erase(logicalVolume);
}

const G4GDMLAuxListType* GeometryAuxiliaryRegistry::Find(const G4LogicalVolume* logicalVolume) const {
    const auto it = fAuxiliariesByLogicalVolume.find(logicalVolume);
    return (it != fAuxiliariesByLogicalVolume.end()) ? &it->second : nullptr;
}

} // namespace MD1
