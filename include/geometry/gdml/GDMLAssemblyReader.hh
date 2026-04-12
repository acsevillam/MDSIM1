#ifndef MDSIM1_GDML_ASSEMBLY_READER_HH
#define MDSIM1_GDML_ASSEMBLY_READER_HH

#include <memory>
#include <vector>

#include "G4GDMLParser.hh"
#include "G4RotationMatrix.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace MD1 {

struct GDMLAssemblyPart {
    G4String name;
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
    const std::vector<GDMLAssemblyPart>& GetParts() const { return fParts; }
    std::size_t GetPartCount() const { return fParts.size(); }
    const std::vector<G4String>& GetAvailableVolumeNames() const { return fAvailableVolumeNames; }

private:
    friend class GDMLAssemblyReader;

    std::shared_ptr<G4GDMLParser> fParser;
    G4String fSourcePath;
    G4String fRootVolumeName;
    G4String fRootPhysicalVolumeName;
    std::vector<GDMLAssemblyPart> fParts;
    std::vector<G4String> fAvailableVolumeNames;
};

class GDMLAssemblyReader {
public:
    static GDMLImportedAssembly ReadAssembly(const G4String& gdmlPath,
                                             const G4String& rootName = "");
};

} // namespace MD1

#endif // MDSIM1_GDML_ASSEMBLY_READER_HH
