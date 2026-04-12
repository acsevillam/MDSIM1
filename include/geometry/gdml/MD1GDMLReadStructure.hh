#ifndef MDSIM1_MD1_GDML_READ_STRUCTURE_HH
#define MDSIM1_MD1_GDML_READ_STRUCTURE_HH

#include <map>
#include <set>
#include <vector>

#include "G4GDMLReadStructure.hh"
#include "G4String.hh"
#include "G4Transform3D.hh"

class G4AssemblyVolume;
class G4LogicalVolume;
class G4VPhysicalVolume;

namespace MD1 {

class MD1GDMLReadStructure : public G4GDMLReadStructure {
public:
    struct PhysicalVolumeMatch {
        const G4VPhysicalVolume* physicalVolume = nullptr;
        G4Transform3D transform;
    };

    MD1GDMLReadStructure() = default;
    ~MD1GDMLReadStructure() override = default;

    void BuildInventory(const G4VPhysicalVolume* worldVolume);

    const std::vector<G4LogicalVolume*>& FindLogicalVolumes(const G4String& name) const;
    const std::vector<PhysicalVolumeMatch>& FindPhysicalVolumes(const G4String& name) const;
    std::vector<G4AssemblyVolume*> FindAssemblies(const G4String& name) const;
    std::vector<G4String> GetLogicalVolumeNames() const;
    std::vector<G4String> GetPhysicalVolumeNames() const;
    std::vector<G4String> GetAssemblyNames() const;
    const G4GDMLAuxListType* FindAuxiliaryInformation(const G4LogicalVolume* logicalVolume) const;
    const G4GDMLAssemblyMapType& GetAssemblyMap() const { return assemblyMap; }

private:
    void ClearInventory();
    void RegisterLogicalVolume(G4LogicalVolume* logicalVolume);
    void RegisterPhysicalVolume(const G4VPhysicalVolume* physicalVolume,
                                const G4Transform3D& transform);
    void Traverse(const G4VPhysicalVolume* physicalVolume,
                  const G4Transform3D& parentTransform,
                  std::set<const G4VPhysicalVolume*>& visitedPhysicals,
                  std::set<G4LogicalVolume*>& visitedLogicals);

    std::map<G4String, std::vector<G4LogicalVolume*>> fLogicalVolumesByName;
    std::map<G4String, std::vector<PhysicalVolumeMatch>> fPhysicalVolumesByName;
};

} // namespace MD1

#endif // MDSIM1_MD1_GDML_READ_STRUCTURE_HH
