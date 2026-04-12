#ifndef MDSIM1_GDML_ASSEMBLY_CACHE_HH
#define MDSIM1_GDML_ASSEMBLY_CACHE_HH

#include <filesystem>
#include <map>
#include <memory>
#include <set>

#include "G4String.hh"

#include "geometry/gdml/GDMLAssemblyReader.hh"

namespace MD1 {

class GDMLAssemblyCache {
public:
    std::shared_ptr<const GDMLImportedAssembly> Load(const G4String& gdmlPath,
                                                     const G4String& rootName = "");
    void RetainOnly(const std::set<G4String>& referencedCacheKeys);
    void Clear();
    std::size_t Size() const { return fEntries.size(); }

    static G4String NormalizePath(const G4String& gdmlPath);
    static G4String BuildCacheKey(const G4String& gdmlPath, const G4String& rootName);

private:
    struct CacheEntry {
        G4String normalizedPath;
        std::filesystem::file_time_type lastWriteTime;
        std::shared_ptr<const GDMLImportedAssembly> assembly;
    };

    static std::filesystem::file_time_type GetLastWriteTimeOrThrow(const G4String& normalizedPath);

    std::map<G4String, CacheEntry> fEntries;
};

} // namespace MD1

#endif // MDSIM1_GDML_ASSEMBLY_CACHE_HH
