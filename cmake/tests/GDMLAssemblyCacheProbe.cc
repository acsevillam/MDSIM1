#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

#include "G4String.hh"

#include "geometry/gdml/GDMLAssemblyCache.hh"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: GDMLAssemblyCacheProbe <first-gdml> <second-gdml> <temp-gdml>\n";
        return 2;
    }

    const fs::path firstSource(argv[1]);
    const fs::path secondSource(argv[2]);
    const fs::path tempPath(argv[3]);

    std::error_code errorCode;
    fs::create_directories(tempPath.parent_path(), errorCode);
    if (errorCode) {
        std::cerr << "Failed to create probe directory: " << errorCode.message() << "\n";
        return 3;
    }

    MD1::GDMLAssemblyCache cache;

    fs::copy_file(firstSource, tempPath, fs::copy_options::overwrite_existing, errorCode);
    if (errorCode) {
        std::cerr << "Failed to copy first GDML: " << errorCode.message() << "\n";
        return 4;
    }

    const auto firstAssembly = cache.Load(tempPath.string());
    if (firstAssembly == nullptr) {
        std::cerr << "First GDML load returned null assembly.\n";
        return 5;
    }

    const auto firstRoot = firstAssembly->GetRootVolumeName();
    const auto firstParts = firstAssembly->GetPartCount();
    const auto firstAssemblyAddress = firstAssembly.get();
    const auto firstWriteTime = fs::last_write_time(tempPath, errorCode);
    if (errorCode) {
        std::cerr << "Failed to query first GDML mtime: " << errorCode.message() << "\n";
        return 6;
    }

    fs::copy_file(secondSource, tempPath, fs::copy_options::overwrite_existing, errorCode);
    if (errorCode) {
        std::cerr << "Failed to copy second GDML: " << errorCode.message() << "\n";
        return 7;
    }

    fs::last_write_time(tempPath, firstWriteTime + std::chrono::seconds(2), errorCode);
    if (errorCode) {
        std::cerr << "Failed to update second GDML mtime: " << errorCode.message() << "\n";
        return 8;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const auto secondAssembly = cache.Load(tempPath.string());
    if (secondAssembly == nullptr) {
        std::cerr << "Second GDML load returned null assembly.\n";
        return 9;
    }

    const auto secondRoot = secondAssembly->GetRootVolumeName();
    const auto secondParts = secondAssembly->GetPartCount();
    const auto secondAssemblyAddress = secondAssembly.get();

    if (firstRoot == secondRoot && firstParts == secondParts) {
        std::cerr << "GDML cache did not observe a geometry change after overwriting the same path.\n";
        return 10;
    }

    if (firstAssemblyAddress == secondAssemblyAddress) {
        std::cerr << "GDML cache reused the same cached assembly after the file mtime changed.\n";
        return 11;
    }

    cache.RetainOnly({});
    if (cache.Size() != 0) {
        std::cerr << "GDML cache retain-only cleanup did not remove the temporary assembly.\n";
        return 12;
    }

    std::cout << "GDML cache reload probe passed: "
              << firstRoot << " (" << firstParts << " parts) -> "
              << secondRoot << " (" << secondParts << " parts)\n";
    return 0;
}
