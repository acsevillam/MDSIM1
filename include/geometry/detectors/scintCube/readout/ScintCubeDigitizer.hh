#ifndef MDSIM1_SCINT_CUBE_DIGITIZER_HH
#define MDSIM1_SCINT_CUBE_DIGITIZER_HH

#include <map>
#include <memory>

#include "geometry/base/BaseDigitizer.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeDigit.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeHit.hh"
#include "geometry/detectors/scintCube/readout/ScintCubePhotosensorModel.hh"
#include "geometry/detectors/scintCube/readout/ScintCubeReadoutParameters.hh"

class ScintCubeDigitizer : public BaseDigitizer {
public:
    ScintCubeDigitizer(const G4String& name,
                       std::map<G4int, ScintCubeReadoutParameters> readoutParametersByDetector);
    ~ScintCubeDigitizer() override;

    void Digitize() override;

private:
    const ScintCubeHitsCollection* fHitsCollection;
    ScintCubeDigitsCollection* fDigitsCollection;
    G4int fDCID;
    std::map<G4int, ScintCubeReadoutParameters> fReadoutParametersByDetector;
    std::map<G4int, std::unique_ptr<ScintCubePhotosensorModel>> fPhotosensorModels;
};

#endif // MDSIM1_SCINT_CUBE_DIGITIZER_HH
