#ifndef MDSIM1_SCINT_CUBE_DIGIT_HH
#define MDSIM1_SCINT_CUBE_DIGIT_HH

#include "G4Allocator.hh"
#include "G4TDigiCollection.hh"

#include "geometry/base/BaseDigit.hh"
#include "geometry/detectors/scintCube/readout/ScintCubePhotosensorModel.hh"

class ScintCubeDigit : public BaseDigit {
public:
    ScintCubeDigit() : BaseDigit("scintCube") {}
    ScintCubeDigit(const ScintCubeDigit&) = default;
    ~ScintCubeDigit() override = default;

    ScintCubeDigit& operator=(const ScintCubeDigit&) = default;
    G4bool operator==(const ScintCubeDigit& right) const;

    inline void* operator new(size_t);
    inline void operator delete(void* digit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetPhotosensorType(ScintCubePhotosensorType photosensorType) { fPhotosensorType = photosensorType; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetVisibleEnergy(G4double visibleEnergy) { fVisibleEnergy = visibleEnergy; }
    void SetProducedPhotons(G4double producedPhotons) { fProducedPhotons = producedPhotons; }
    void SetDetectedPhotoelectrons(G4double detectedPhotoelectrons) {
        fDetectedPhotoelectrons = detectedPhotoelectrons;
    }
    void SetMeanSignalTime(G4double meanSignalTime) { fMeanSignalTime = meanSignalTime; }

    G4int GetDetectorID() const { return fDetectorID; }
    ScintCubePhotosensorType GetPhotosensorType() const { return fPhotosensorType; }
    G4String GetPhotosensorTypeName() const { return ScintCubePhotosensorTypeToString(fPhotosensorType); }
    G4double GetEdep() const { return fEdep; }
    G4double GetVisibleEnergy() const { return fVisibleEnergy; }
    G4double GetProducedPhotons() const { return fProducedPhotons; }
    G4double GetDetectedPhotoelectrons() const { return fDetectedPhotoelectrons; }
    G4double GetMeanSignalTime() const { return fMeanSignalTime; }

private:
    G4int fDetectorID = -1;
    ScintCubePhotosensorType fPhotosensorType = ScintCubePhotosensorType::PMT;
    G4double fEdep = 0.;
    G4double fVisibleEnergy = 0.;
    G4double fProducedPhotons = 0.;
    G4double fDetectedPhotoelectrons = 0.;
    G4double fMeanSignalTime = 0.;
};

using ScintCubeDigitsCollection = G4TDigiCollection<ScintCubeDigit>;

extern G4ThreadLocal G4Allocator<ScintCubeDigit>* ScintCubeDigitAllocator;

inline void* ScintCubeDigit::operator new(size_t) {
    if (!ScintCubeDigitAllocator) {
        ScintCubeDigitAllocator = new G4Allocator<ScintCubeDigit>;
    }
    return ScintCubeDigitAllocator->MallocSingle();
}

inline void ScintCubeDigit::operator delete(void* digit) {
    if (!ScintCubeDigitAllocator) {
        ScintCubeDigitAllocator = new G4Allocator<ScintCubeDigit>;
    }
    ScintCubeDigitAllocator->FreeSingle(static_cast<ScintCubeDigit*>(digit));
}

#endif // MDSIM1_SCINT_CUBE_DIGIT_HH
