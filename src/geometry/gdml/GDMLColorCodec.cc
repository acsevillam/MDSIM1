#include "geometry/gdml/GDMLColorCodec.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "G4VisAttributes.hh"

namespace {

G4bool TryParseHexByte(const std::string& hexPair, G4double& component) {
    if (hexPair.size() != 2) {
        return false;
    }

    char* end = nullptr;
    const long parsed = std::strtol(hexPair.c_str(), &end, 16);
    if (end == nullptr || *end != '\0' || parsed < 0 || parsed > 255) {
        return false;
    }

    component = static_cast<G4double>(parsed) / 255.0;
    return true;
}

G4String NormalizeColorName(const G4String& value) {
    G4String normalized = value;
    std::transform(normalized.begin(),
                   normalized.end(),
                   normalized.begin(),
                   [](unsigned char ch) { return std::tolower(ch); });
    return normalized;
}

} // namespace

namespace MD1::GDMLColorCodec {

const G4GDMLAuxStructType* FindColorAuxiliary(const G4GDMLAuxListType& auxiliaries) {
    const auto it = std::find_if(auxiliaries.begin(), auxiliaries.end(), [](const auto& auxiliary) {
        return auxiliary.type == "Color";
    });
    return (it != auxiliaries.end()) ? &(*it) : nullptr;
}

G4GDMLAuxListType CopyAuxiliariesWithoutColor(const G4GDMLAuxListType& auxiliaries) {
    G4GDMLAuxListType filteredAuxiliaries;
    filteredAuxiliaries.reserve(auxiliaries.size());
    for (const auto& auxiliary : auxiliaries) {
        if (auxiliary.type != "Color") {
            filteredAuxiliaries.push_back(auxiliary);
        }
    }
    return filteredAuxiliaries;
}

G4bool TryDecodeColor(const G4String& colorValue, G4Colour& color, G4String* errorMessage) {
    auto setError = [errorMessage](const G4String& message) {
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
    };

    if (colorValue.empty()) {
        setError("Color auxiliary is empty.");
        return false;
    }

    G4double red = 0.0;
    G4double green = 0.0;
    G4double blue = 0.0;
    G4double alpha = 1.0;

    if (colorValue[0] == '#') {
        const std::string hex = colorValue.substr(1);
        if (hex.size() == 6) {
            if (!TryParseHexByte(hex.substr(0, 2), red) ||
                !TryParseHexByte(hex.substr(2, 2), green) ||
                !TryParseHexByte(hex.substr(4, 2), blue)) {
                setError("Color hex value must use valid hexadecimal digits.");
                return false;
            }
        } else if (hex.size() == 8) {
            G4double transparency = 0.0;
            if (!TryParseHexByte(hex.substr(0, 2), red) ||
                !TryParseHexByte(hex.substr(2, 2), green) ||
                !TryParseHexByte(hex.substr(4, 2), blue) ||
                !TryParseHexByte(hex.substr(6, 2), transparency)) {
                setError("FreeCAD color hex value must use valid hexadecimal digits.");
                return false;
            }
            alpha = 1.0 - transparency;
        } else {
            setError("Color hex value must use either #RRGGBB or FreeCAD #RRGGBBTT.");
            return false;
        }
    } else {
        const auto normalizedColor = NormalizeColorName(colorValue);
        if (normalizedColor == "red") {
            red = 1.0;
        } else if (normalizedColor == "green") {
            green = 1.0;
        } else if (normalizedColor == "blue") {
            blue = 1.0;
        } else if (normalizedColor == "yellow") {
            red = 1.0;
            green = 1.0;
        } else if (normalizedColor == "cyan") {
            green = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "magenta") {
            red = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "white") {
            red = 1.0;
            green = 1.0;
            blue = 1.0;
        } else if (normalizedColor == "black") {
            red = 0.0;
            green = 0.0;
            blue = 0.0;
        } else if (normalizedColor == "gray" || normalizedColor == "grey") {
            red = 0.5;
            green = 0.5;
            blue = 0.5;
        } else {
            setError("Unsupported named color. Use a standard name or #RRGGBB / #RRGGBBTT.");
            return false;
        }
    }

    color = G4Colour(red, green, blue, alpha);
    return true;
}

G4String EncodeColor(const G4Colour& color) {
    auto encodeComponent = [](const G4double value) {
        const auto clamped = std::clamp(value, 0.0, 1.0);
        return static_cast<int>(std::lround(clamped * 255.0));
    };

    const auto transparency = 1.0 - std::clamp(color.GetAlpha(), 0.0, 1.0);

    std::ostringstream stream;
    stream << '#'
           << std::hex << std::nouppercase << std::setw(2) << std::setfill('0')
           << encodeComponent(color.GetRed())
           << std::setw(2) << encodeComponent(color.GetGreen())
           << std::setw(2) << encodeComponent(color.GetBlue())
           << std::setw(2) << encodeComponent(transparency);
    return stream.str();
}

G4bool TryCreateVisAttributesFromColorAux(const G4GDMLAuxListType& auxiliaries,
                                          G4VisAttributes*& visAttributes,
                                          G4String* errorMessage) {
    const auto* colorAuxiliary = FindColorAuxiliary(auxiliaries);
    if (colorAuxiliary == nullptr) {
        return false;
    }

    G4Colour color;
    if (!TryDecodeColor(colorAuxiliary->value, color, errorMessage)) {
        return false;
    }

    visAttributes = new G4VisAttributes(color);
    return true;
}

} // namespace MD1::GDMLColorCodec
