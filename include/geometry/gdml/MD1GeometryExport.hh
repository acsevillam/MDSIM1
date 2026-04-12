#ifndef MDSIM1_MD1_GEOMETRY_EXPORT_HH
#define MDSIM1_MD1_GEOMETRY_EXPORT_HH

#include "G4String.hh"

namespace MD1 {

class MD1GeometryExportMessenger;

class MD1GeometryExport {
public:
    static MD1GeometryExport* GetInstance();
    static void Kill();

    void ListPhysicalVolumes(const G4String& pattern) const;
    void ListLogicalVolumes(const G4String& pattern) const;
    void WritePhysicalGDML(const G4String& volumeName,
                           G4int copyNo,
                           const G4String& outputPath) const;
    void WriteLogicalGDML(const G4String& volumeName, const G4String& outputPath) const;

private:
    MD1GeometryExport();
    ~MD1GeometryExport();

    static MD1GeometryExport* fInstance;
    MD1GeometryExportMessenger* fMessenger;
};

} // namespace MD1

#endif // MDSIM1_MD1_GEOMETRY_EXPORT_HH
