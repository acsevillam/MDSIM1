#ifndef MDSIM1_SCINT_CUBE_READOUT_PARAMETERS_HH
#define MDSIM1_SCINT_CUBE_READOUT_PARAMETERS_HH

#include "G4SystemOfUnits.hh"
#include "globals.hh"

#include "geometry/detectors/scintCube/readout/ScintCubePhotosensorModel.hh"

struct ScintCubeReadoutParameters {
    G4double scintillationYield = 10000.0 / MeV;
    G4double birksConstant = 0.0 * mm / MeV;
    G4double lightCollectionEfficiency = 0.20;
    G4double decayTime = 2.0 * ns;
    G4double transportDelay = 0.5 * ns;
    G4double timeJitter = 0.1 * ns;
    G4double resolutionScale = 1.0;
    ScintCubePhotosensorType photosensorType = ScintCubePhotosensorType::PMT;
    ScintCubePMTParameters pmtParameters;
    ScintCubeSiPMParameters sipmParameters;
};

#endif // MDSIM1_SCINT_CUBE_READOUT_PARAMETERS_HH
