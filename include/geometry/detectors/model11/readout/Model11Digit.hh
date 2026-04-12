#ifndef MDSIM1_MODEL11_DIGIT_HH
#define MDSIM1_MODEL11_DIGIT_HH

#include "G4Allocator.hh"
#include "G4TDigiCollection.hh"

#include "geometry/base/BaseDigit.hh"
#include "geometry/detectors/model11/readout/Model11PhotosensorModel.hh"

class Model11Digit : public BaseDigit {
public:
    Model11Digit() : BaseDigit("model11") {}
    Model11Digit(const Model11Digit&) = default;
    ~Model11Digit() override = default;

    Model11Digit& operator=(const Model11Digit&) = default;
    G4bool operator==(const Model11Digit& right) const;

    inline void* operator new(size_t);
    inline void operator delete(void* digit);

    void Draw() override {}
    void Print() override;

    void SetDetectorID(G4int detectorID) { fDetectorID = detectorID; }
    void SetPhotosensorType(Model11PhotosensorType photosensorType) { fPhotosensorType = photosensorType; }
    void SetEdep(G4double edep) { fEdep = edep; }
    void SetVisibleEnergy(G4double visibleEnergy) { fVisibleEnergy = visibleEnergy; }
    void SetProducedPhotons(G4double producedPhotons) { fProducedPhotons = producedPhotons; }
    void SetDetectedPhotoelectrons(G4double detectedPhotoelectrons) {
        fDetectedPhotoelectrons = detectedPhotoelectrons;
    }
    void SetMeanSignalTime(G4double meanSignalTime) { fMeanSignalTime = meanSignalTime; }

    G4int GetDetectorID() const { return fDetectorID; }
    Model11PhotosensorType GetPhotosensorType() const { return fPhotosensorType; }
    G4String GetPhotosensorTypeName() const { return Model11PhotosensorTypeToString(fPhotosensorType); }
    G4double GetEdep() const { return fEdep; }
    G4double GetVisibleEnergy() const { return fVisibleEnergy; }
    G4double GetProducedPhotons() const { return fProducedPhotons; }
    G4double GetDetectedPhotoelectrons() const { return fDetectedPhotoelectrons; }
    G4double GetMeanSignalTime() const { return fMeanSignalTime; }

private:
    G4int fDetectorID = -1;
    Model11PhotosensorType fPhotosensorType = Model11PhotosensorType::PMT;
    G4double fEdep = 0.;
    G4double fVisibleEnergy = 0.;
    G4double fProducedPhotons = 0.;
    G4double fDetectedPhotoelectrons = 0.;
    G4double fMeanSignalTime = 0.;
};

using Model11DigitsCollection = G4TDigiCollection<Model11Digit>;

extern G4ThreadLocal G4Allocator<Model11Digit>* Model11DigitAllocator;

inline void* Model11Digit::operator new(size_t) {
    if (!Model11DigitAllocator) {
        Model11DigitAllocator = new G4Allocator<Model11Digit>;
    }
    return Model11DigitAllocator->MallocSingle();
}

inline void Model11Digit::operator delete(void* digit) {
    if (!Model11DigitAllocator) {
        Model11DigitAllocator = new G4Allocator<Model11Digit>;
    }
    Model11DigitAllocator->FreeSingle(static_cast<Model11Digit*>(digit));
}

#endif // MDSIM1_MODEL11_DIGIT_HH
