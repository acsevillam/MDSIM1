#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"

namespace {

void DumpPhysicalTree(const G4VPhysicalVolume* physicalVolume, const G4String& path) {
    if (physicalVolume == nullptr || physicalVolume->GetLogicalVolume() == nullptr) {
        return;
    }

    const auto translation = physicalVolume->GetObjectTranslation();
    std::cout << std::fixed << std::setprecision(3)
              << "node=" << path
              << " copyNo=" << physicalVolume->GetCopyNo()
              << " logical=" << physicalVolume->GetLogicalVolume()->GetName()
              << " translation_mm=("
              << translation.x() / mm << ","
              << translation.y() / mm << ","
              << translation.z() / mm << ")\n";

    auto* logicalVolume = physicalVolume->GetLogicalVolume();
    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        DumpPhysicalTree(daughter, path + "/" + daughter->GetName());
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: GDMLExportProbe <gdml-path>\n";
        return 2;
    }

    G4GDMLParser parser;
    parser.Read(argv[1], false);

    auto* worldPhysical = parser.GetWorldVolume();
    if (worldPhysical == nullptr || worldPhysical->GetLogicalVolume() == nullptr) {
        std::cerr << "Export probe could not resolve the world volume.\n";
        return 3;
    }

    auto* worldLogical = worldPhysical->GetLogicalVolume();
    std::cout << "world=" << worldLogical->GetName()
              << " daughters=" << worldLogical->GetNoDaughters() << "\n";

    for (G4int daughterIndex = 0; daughterIndex < worldLogical->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = worldLogical->GetDaughter(daughterIndex);
        if (daughter == nullptr || daughter->GetLogicalVolume() == nullptr) {
            continue;
        }

        std::cout << "daughter=" << daughter->GetName()
                  << " copyNo=" << daughter->GetCopyNo()
                  << " logical=" << daughter->GetLogicalVolume()->GetName() << "\n";
        DumpPhysicalTree(daughter, worldLogical->GetName() + "/" + daughter->GetName());
    }

    G4int colorCount = 0;
    G4int sensDetCount = 0;
    std::vector<std::pair<G4String, G4String>> colorValues;
    if (const auto* auxMap = parser.GetAuxMap(); auxMap != nullptr) {
        for (const auto& [logicalVolume, auxiliaries] : *auxMap) {
            for (const auto& auxiliary : auxiliaries) {
                if (auxiliary.type == "Color") {
                    ++colorCount;
                    if (logicalVolume != nullptr) {
                        colorValues.emplace_back(logicalVolume->GetName(), auxiliary.value);
                    }
                } else if (auxiliary.type == "SensDet") {
                    ++sensDetCount;
                }
            }
        }
    }

    std::sort(colorValues.begin(), colorValues.end(), [](const auto& lhs, const auto& rhs) {
        return (lhs.first == rhs.first) ? (lhs.second < rhs.second) : (lhs.first < rhs.first);
    });

    std::cout << "aux.color=" << colorCount
              << " aux.sensdet=" << sensDetCount << "\n";
    for (const auto& [logicalName, colorValue] : colorValues) {
        std::cout << "aux.color.value=" << logicalName << "=" << colorValue << "\n";
    }
    return 0;
}
