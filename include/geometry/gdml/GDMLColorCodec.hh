#ifndef MDSIM1_GDML_COLOR_CODEC_HH
#define MDSIM1_GDML_COLOR_CODEC_HH

#include "G4Colour.hh"
#include "G4GDMLAuxStructType.hh"
#include "G4String.hh"

class G4VisAttributes;

namespace MD1::GDMLColorCodec {

const G4GDMLAuxStructType* FindColorAuxiliary(const G4GDMLAuxListType& auxiliaries);
G4GDMLAuxListType CopyAuxiliariesWithoutColor(const G4GDMLAuxListType& auxiliaries);

G4bool TryDecodeColor(const G4String& colorValue,
                      G4Colour& color,
                      G4String* errorMessage = nullptr);
G4String EncodeColor(const G4Colour& color);

G4bool TryCreateVisAttributesFromColorAux(const G4GDMLAuxListType& auxiliaries,
                                          G4VisAttributes*& visAttributes,
                                          G4String* errorMessage = nullptr);

} // namespace MD1::GDMLColorCodec

#endif // MDSIM1_GDML_COLOR_CODEC_HH
