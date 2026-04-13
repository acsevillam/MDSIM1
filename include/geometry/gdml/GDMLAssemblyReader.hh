#ifndef MDSIM1_GDML_ASSEMBLY_READER_HH
#define MDSIM1_GDML_ASSEMBLY_READER_HH

#include <memory>
#include <map>
#include <set>
#include <vector>

#include "G4GDMLAuxStructType.hh"
#include "G4GDMLParser.hh"
#include "G4RotationMatrix.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4GDMLParser;

namespace MD1 {

struct GDMLDetachedParserScaffold {
    std::vector<G4VPhysicalVolume*> physicalVolumes;
    std::vector<G4LogicalVolume*> logicalVolumes;

    ~GDMLDetachedParserScaffold();
};

class MD1GDMLReadStructure;

enum class GDMLRootSelectorType {
    Auto,
    Logical,
    Physical,
    Assembly
};

struct GDMLRootSelector {
    GDMLRootSelectorType type = GDMLRootSelectorType::Auto;
    G4String name;
};

struct GDMLReadOptions {
    G4bool validate = false;
    G4String schemaPath;
};


struct GDMLAssemblyPart {
    G4String name;
    G4String runtimePhysicalName;
    G4LogicalVolume* logicalVolume = nullptr;
    const G4VPhysicalVolume* physicalVolume = nullptr;
    G4ThreeVector translation;
    G4RotationMatrix rotation;
};

class GDMLImportedAssembly {
public:
    GDMLImportedAssembly() = default;

    const G4String& GetSourcePath() const { return fSourcePath; }
    const G4String& GetRootVolumeName() const { return fRootVolumeName; }
    const G4String& GetRootPhysicalVolumeName() const { return fRootPhysicalVolumeName; }
    const GDMLRootSelector& GetRootSelector() const { return fRootSelector; }
    const std::vector<GDMLAssemblyPart>& GetParts() const { return fParts; }
    std::size_t GetPartCount() const { return fParts.size(); }
    const std::vector<G4String>& GetAvailableVolumeNames() const { return fAvailableVolumeNames; }
    const std::vector<G4String>& GetAvailableLogicalVolumeNames() const {
        return fAvailableLogicalVolumeNames;
    }
    const std::vector<G4String>& GetAvailablePhysicalVolumeNames() const {
        return fAvailablePhysicalVolumeNames;
    }
    const std::vector<G4String>& GetAvailableAssemblyNames() const {
        return fAvailableAssemblyNames;
    }
    std::vector<G4String> GetAuxSensitiveVolumeNames() const;
    const G4GDMLAuxListType* GetAuxiliaryInfo(const G4LogicalVolume* logicalVolume) const;

private:
    friend class GDMLAssemblyReader;

    std::shared_ptr<G4GDMLParser> fParser;
    MD1GDMLReadStructure* fReader = nullptr;
    G4String fSourcePath;
    G4String fRootVolumeName;
    G4String fRootPhysicalVolumeName;
    GDMLRootSelector fRootSelector;
    std::vector<GDMLAssemblyPart> fParts;
    std::vector<G4String> fAvailableVolumeNames;
    std::vector<G4String> fAvailableLogicalVolumeNames;
    std::vector<G4String> fAvailablePhysicalVolumeNames;
    std::vector<G4String> fAvailableAssemblyNames;
    std::map<const G4LogicalVolume*, G4GDMLAuxListType> fAuxiliaryInfoByLogicalVolume;
    std::shared_ptr<GDMLDetachedParserScaffold> fDetachedParserScaffold;
};

class GDMLAssemblyReader {
public:
    static GDMLImportedAssembly ReadAssembly(const G4String& gdmlPath,
                                             const GDMLRootSelector& rootSelector = {},
                                             const GDMLReadOptions& readOptions = {});
};

} // namespace MD1

#endif // MDSIM1_GDML_ASSEMBLY_READER_HH
