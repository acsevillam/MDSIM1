#include "geometry/gdml/MD1GeometryExportMessenger.hh"

#include <sstream>

#include "G4Exception.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIparameter.hh"

#include "geometry/gdml/MD1GeometryExport.hh"

namespace MD1 {

MD1GeometryExportMessenger::MD1GeometryExportMessenger(MD1GeometryExport* geometryExport)
    : fGeometryExport(geometryExport) {
    fListPhysicalVolumesCmd =
        new G4UIcmdWithAString("/MultiDetector1/geometry/listPhysicalVolumes", this);
    fListPhysicalVolumesCmd->SetGuidance("List registered physical volumes, optionally filtering by substring.");
    fListPhysicalVolumesCmd->SetParameterName("pattern", true);
    fListPhysicalVolumesCmd->SetDefaultValue("");
    fListPhysicalVolumesCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fListLogicalVolumesCmd =
        new G4UIcmdWithAString("/MultiDetector1/geometry/listLogicalVolumes", this);
    fListLogicalVolumesCmd->SetGuidance("List registered logical volumes, optionally filtering by substring.");
    fListLogicalVolumesCmd->SetParameterName("pattern", true);
    fListLogicalVolumesCmd->SetDefaultValue("");
    fListLogicalVolumesCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fWritePhysicalGDMLCmd = new G4UIcommand("/MultiDetector1/geometry/writePhysicalGDML", this);
    fWritePhysicalGDMLCmd->SetGuidance(
        "Write a self-contained GDML for a live physical volume selected by exact name and copyNo.");
    auto* physicalNameParam = new G4UIparameter("volumeName", 's', false);
    fWritePhysicalGDMLCmd->SetParameter(physicalNameParam);
    auto* physicalCopyNoParam = new G4UIparameter("copyNo", 'i', false);
    fWritePhysicalGDMLCmd->SetParameter(physicalCopyNoParam);
    auto* physicalOutputParam = new G4UIparameter("outputPath", 's', false);
    fWritePhysicalGDMLCmd->SetParameter(physicalOutputParam);
    fWritePhysicalGDMLCmd->AvailableForStates(G4State_Idle);

    fWriteLogicalGDMLCmd = new G4UIcommand("/MultiDetector1/geometry/writeLogicalGDML", this);
    fWriteLogicalGDMLCmd->SetGuidance(
        "Write a self-contained GDML for a live logical volume selected by exact unique name.");
    auto* logicalNameParam = new G4UIparameter("volumeName", 's', false);
    fWriteLogicalGDMLCmd->SetParameter(logicalNameParam);
    auto* logicalOutputParam = new G4UIparameter("outputPath", 's', false);
    fWriteLogicalGDMLCmd->SetParameter(logicalOutputParam);
    fWriteLogicalGDMLCmd->AvailableForStates(G4State_Idle);
}

MD1GeometryExportMessenger::~MD1GeometryExportMessenger() {
    delete fListPhysicalVolumesCmd;
    delete fListLogicalVolumesCmd;
    delete fWritePhysicalGDMLCmd;
    delete fWriteLogicalGDMLCmd;
}

void MD1GeometryExportMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fListPhysicalVolumesCmd) {
        fGeometryExport->ListPhysicalVolumes(newValue);
    } else if (command == fListLogicalVolumesCmd) {
        fGeometryExport->ListLogicalVolumes(newValue);
    } else if (command == fWritePhysicalGDMLCmd) {
        std::istringstream input(newValue);
        G4String volumeName;
        G4int copyNo = 0;
        G4String outputPath;
        input >> volumeName >> copyNo >> outputPath;
        if (volumeName.empty() || outputPath.empty() || input.fail()) {
            G4Exception("MD1GeometryExportMessenger::SetNewValue",
                        "GeometryExportInvalidCommand",
                        FatalException,
                        "writePhysicalGDML expects: <volumeName> <copyNo> <outputPath>.");
        }
        fGeometryExport->WritePhysicalGDML(volumeName, copyNo, outputPath);
    } else if (command == fWriteLogicalGDMLCmd) {
        std::istringstream input(newValue);
        G4String volumeName;
        G4String outputPath;
        input >> volumeName >> outputPath;
        if (volumeName.empty() || outputPath.empty() || input.fail()) {
            G4Exception("MD1GeometryExportMessenger::SetNewValue",
                        "GeometryExportInvalidCommand",
                        FatalException,
                        "writeLogicalGDML expects: <volumeName> <outputPath>.");
        }
        fGeometryExport->WriteLogicalGDML(volumeName, outputPath);
    }
}

} // namespace MD1
