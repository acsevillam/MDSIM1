#ifndef MD1_DETECTOR_PRINT_CONTEXT_HH
#define MD1_DETECTOR_PRINT_CONTEXT_HH

#include "globals.hh"

namespace MD1 {

struct DetectorPrintContext {
    G4int nofEvents = 0;
    G4int simulatedMU = 1;
    G4double scaleFactorMU = 1.;
    G4double scaleFactorMUError = 0.;
};

} // namespace MD1

#endif // MD1_DETECTOR_PRINT_CONTEXT_HH
