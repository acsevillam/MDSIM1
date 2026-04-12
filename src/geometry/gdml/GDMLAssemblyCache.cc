#include "geometry/gdml/GDMLAssemblyCache.hh"

#include <filesystem>

#include "G4Exception.hh"

namespace {

namespace fs = std::filesystem;

G4String TrimName(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

} // namespace

namespace MD1 {

G4String GDMLAssemblyCache::NormalizePath(const G4String& gdmlPath) {
    if (gdmlPath.empty()) {
        return gdmlPath;
    }

    return fs::absolute(fs::path(gdmlPath.c_str())).lexically_normal().string();
}

G4String GDMLAssemblyCache::BuildCacheKey(const G4String& gdmlPath, const G4String& rootName) {
    const auto normalizedPath = NormalizePath(gdmlPath);
    const auto normalizedRootName = TrimName(rootName);
    if (normalizedRootName.empty()) {
        return normalizedPath;
    }

    return normalizedPath + "::root=" + normalizedRootName;
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

std::shared_ptr<const GDMLImportedAssembly> GDMLAssemblyCache::Load(const G4String& gdmlPath,
                                                                    const G4String& rootName) {
    const auto normalizedPath = NormalizePath(gdmlPath);
    const auto cacheKey = BuildCacheKey(normalizedPath, rootName);
    const auto lastWriteTime = GetLastWriteTimeOrThrow(normalizedPath);

    const auto cacheIt = fEntries.find(cacheKey);
    if (cacheIt != fEntries.end() && cacheIt->second.assembly != nullptr &&
        cacheIt->second.lastWriteTime == lastWriteTime) {
        return cacheIt->second.assembly;
    }

    auto assembly =
        std::make_shared<const GDMLImportedAssembly>(GDMLAssemblyReader::ReadAssembly(normalizedPath, rootName));
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
