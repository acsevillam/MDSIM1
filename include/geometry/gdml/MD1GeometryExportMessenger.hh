#ifndef MDSIM1_MD1_GEOMETRY_EXPORT_MESSENGER_HH
#define MDSIM1_MD1_GEOMETRY_EXPORT_MESSENGER_HH

#include "G4UImessenger.hh"

class G4UIcmdWithAString;
class G4UIcommand;

namespace MD1 {

class MD1GeometryExport;

class MD1GeometryExportMessenger : public G4UImessenger {
public:
    explicit MD1GeometryExportMessenger(MD1GeometryExport* geometryExport);
    ~MD1GeometryExportMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    MD1GeometryExport* fGeometryExport;
    G4UIcmdWithAString* fListPhysicalVolumesCmd;
    G4UIcmdWithAString* fListLogicalVolumesCmd;
    G4UIcommand* fWritePhysicalGDMLCmd;
    G4UIcommand* fWriteLogicalGDMLCmd;
};

} // namespace MD1

#endif // MDSIM1_MD1_GEOMETRY_EXPORT_MESSENGER_HH
