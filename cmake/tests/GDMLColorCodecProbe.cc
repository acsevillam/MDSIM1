#include <iomanip>
#include <iostream>

#include "geometry/gdml/GDMLColorCodec.hh"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: GDMLColorCodecProbe <color-value>\n";
        return 2;
    }

    G4Colour color;
    G4String errorMessage;
    if (!MD1::GDMLColorCodec::TryDecodeColor(argv[1], color, &errorMessage)) {
        std::cerr << "error=" << errorMessage << "\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(6)
              << "alpha=" << color.GetAlpha()
              << " encoded=" << MD1::GDMLColorCodec::EncodeColor(color) << "\n";
    return 0;
}
