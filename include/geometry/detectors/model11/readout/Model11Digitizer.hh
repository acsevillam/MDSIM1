#ifndef MDSIM1_MODEL11_DIGITIZER_HH
#define MDSIM1_MODEL11_DIGITIZER_HH

#include <map>
#include <memory>

#include "geometry/base/BaseDigitizer.hh"
#include "geometry/detectors/model11/readout/Model11Digit.hh"
#include "geometry/detectors/model11/readout/Model11Hit.hh"
#include "geometry/detectors/model11/readout/Model11PhotosensorModel.hh"
#include "geometry/detectors/model11/readout/Model11ReadoutParameters.hh"

class Model11Digitizer : public BaseDigitizer {
public:
    Model11Digitizer(const G4String& name,
                       std::map<G4int, Model11ReadoutParameters> readoutParametersByDetector);
    ~Model11Digitizer() override;

    void Digitize() override;

private:
    const Model11HitsCollection* fHitsCollection;
    Model11DigitsCollection* fDigitsCollection;
    G4int fDCID;
    std::map<G4int, Model11ReadoutParameters> fReadoutParametersByDetector;
    std::map<G4int, std::unique_ptr<Model11PhotosensorModel>> fPhotosensorModels;
};

#endif // MDSIM1_MODEL11_DIGITIZER_HH
