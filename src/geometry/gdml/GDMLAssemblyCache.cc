#include "geometry/gdml/GDMLAssemblyCache.hh"

#include <filesystem>

#include "G4Exception.hh"

namespace {

namespace fs = std::filesystem;

G4String NormalizeOptionalPath(const G4String& path) {
    if (path.empty()) {
        return "";
    }

    return fs::absolute(fs::path(path.c_str())).lexically_normal().string();
}

G4String RootSelectorTypeToString(const MD1::GDMLRootSelectorType type) {
    switch (type) {
        case MD1::GDMLRootSelectorType::Logical:
            return "logical";
        case MD1::GDMLRootSelectorType::Physical:
            return "physical";
        case MD1::GDMLRootSelectorType::Assembly:
            return "assembly";
        case MD1::GDMLRootSelectorType::Auto:
        default:
            return "auto";
    }
}

} // namespace

namespace MD1 {

G4String GDMLAssemblyCache::NormalizePath(const G4String& gdmlPath) {
    if (gdmlPath.empty()) {
        return gdmlPath;
    }

    return fs::absolute(fs::path(gdmlPath.c_str())).lexically_normal().string();
}

G4String GDMLAssemblyCache::BuildCacheKey(const G4String& gdmlPath,
                                          const GDMLRootSelector& rootSelector,
                                          const GDMLReadOptions& readOptions) {
    const auto normalizedPath = NormalizePath(gdmlPath);
    const auto normalizedRootName = rootSelector.name;
    const auto normalizedSchemaPath = NormalizeOptionalPath(readOptions.schemaPath);
    G4String cacheKey = normalizedPath + "::rootType=" + RootSelectorTypeToString(rootSelector.type);

    if (!normalizedRootName.empty()) {
        cacheKey += "::rootName=" + normalizedRootName;
    }
    cacheKey += "::validate=" + G4String(readOptions.validate ? "1" : "0");
    if (!normalizedSchemaPath.empty()) {
        cacheKey += "::schema=" + normalizedSchemaPath;
    }

    return cacheKey;
}

fs::file_time_type GDMLAssemblyCache::GetLastWriteTimeOrThrow(const G4String& normalizedPath) {
    const fs::path path(normalizedPath.c_str());
    if (!fs::exists(path)) {
        G4Exception("GDMLAssemblyCache::GetLastWriteTimeOrThrow",
                    "ImportedGDMLFileNotFound",
                    FatalException,
                    ("Could not find imported GDML file: " + normalizedPath).c_str());
    }

    std::error_code errorCode;
    const auto lastWriteTime = fs::last_write_time(path, errorCode);
    if (errorCode) {
        G4Exception("GDMLAssemblyCache::GetLastWriteTimeOrThrow",
                    "ImportedGDMLLastWriteTimeFailed",
                    FatalException,
                    ("Could not query the modification time for imported GDML " + normalizedPath +
                     ": " + errorCode.message()).c_str());
    }

    return lastWriteTime;
}

std::shared_ptr<const GDMLImportedAssembly> GDMLAssemblyCache::Load(
    const G4String& gdmlPath,
    const GDMLRootSelector& rootSelector,
    const GDMLReadOptions& readOptions) {
    const auto normalizedPath = NormalizePath(gdmlPath);
    const auto cacheKey = BuildCacheKey(normalizedPath, rootSelector, readOptions);
    const auto lastWriteTime = GetLastWriteTimeOrThrow(normalizedPath);

    const auto cacheIt = fEntries.find(cacheKey);
    if (cacheIt != fEntries.end() && cacheIt->second.assembly != nullptr &&
        cacheIt->second.lastWriteTime == lastWriteTime) {
        return cacheIt->second.assembly;
    }

    auto assembly =
        std::make_shared<const GDMLImportedAssembly>(
            GDMLAssemblyReader::ReadAssembly(normalizedPath, rootSelector, readOptions));
    fEntries[cacheKey] = CacheEntry{normalizedPath, lastWriteTime, assembly};
    return assembly;
}

void GDMLAssemblyCache::RetainOnly(const std::set<G4String>& referencedCacheKeys) {
    for (auto it = fEntries.begin(); it != fEntries.end();) {
        if (referencedCacheKeys.find(it->first) == referencedCacheKeys.end()) {
            it = fEntries.erase(it);
        } else {
            ++it;
        }
    }
}

void GDMLAssemblyCache::Clear() {
    fEntries.clear();
}

} // namespace MD1
