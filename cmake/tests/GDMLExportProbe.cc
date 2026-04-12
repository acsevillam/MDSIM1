#include <iostream>

#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

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
    }

    G4int colorCount = 0;
    G4int sensDetCount = 0;
    if (const auto* auxMap = parser.GetAuxMap(); auxMap != nullptr) {
        for (const auto& [logicalVolume, auxiliaries] : *auxMap) {
            (void)logicalVolume;
            for (const auto& auxiliary : auxiliaries) {
                if (auxiliary.type == "Color") {
                    ++colorCount;
                } else if (auxiliary.type == "SensDet") {
                    ++sensDetCount;
                }
            }
        }
    }

    std::cout << "aux.color=" << colorCount
              << " aux.sensdet=" << sensDetCount << "\n";
    return 0;
}
