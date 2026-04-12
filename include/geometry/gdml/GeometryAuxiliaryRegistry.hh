#ifndef MDSIM1_GEOMETRY_AUXILIARY_REGISTRY_HH
#define MDSIM1_GEOMETRY_AUXILIARY_REGISTRY_HH

#include <map>

#include "G4GDMLAuxStructType.hh"

class G4LogicalVolume;

namespace MD1 {

class GeometryAuxiliaryRegistry {
public:
    static GeometryAuxiliaryRegistry* GetInstance();
    static void Kill();

    void Register(const G4LogicalVolume* logicalVolume, const G4GDMLAuxListType& auxiliaries);
    void Unregister(const G4LogicalVolume* logicalVolume);
    const G4GDMLAuxListType* Find(const G4LogicalVolume* logicalVolume) const;

private:
    GeometryAuxiliaryRegistry() = default;
    ~GeometryAuxiliaryRegistry() = default;

    static GeometryAuxiliaryRegistry* fInstance;
    std::map<const G4LogicalVolume*, G4GDMLAuxListType> fAuxiliariesByLogicalVolume;
};

} // namespace MD1

#endif // MDSIM1_GEOMETRY_AUXILIARY_REGISTRY_HH
