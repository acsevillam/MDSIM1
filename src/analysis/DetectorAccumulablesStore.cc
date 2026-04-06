#include "analysis/DetectorAccumulablesStore.hh"

#include <algorithm>
#include <mutex>

#include "G4Threading.hh"

namespace MD1 {

namespace {

std::mutex gDetectorAccumulablesMutex;
std::size_t gDetectorAccumulablesGeneration = 0;
std::vector<DetectorRunAccumulables> gMergedDetectorAccumulables;

DetectorRunAccumulables* FindDetectorAccumulablesByName(
    std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const G4String& detectorName) {
    const auto it = std::find_if(detectorAccumulables.begin(),
                                 detectorAccumulables.end(),
                                 [&](const auto& accumulators) {
                                     return accumulators.name == detectorName;
                                 });
    return (it != detectorAccumulables.end()) ? &(*it) : nullptr;
}

const DetectorRunAccumulables* FindDetectorAccumulablesByName(
    const std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const G4String& detectorName) {
    const auto it = std::find_if(detectorAccumulables.begin(),
                                 detectorAccumulables.end(),
                                 [&](const auto& accumulators) {
                                     return accumulators.name == detectorName;
                                 });
    return (it != detectorAccumulables.end()) ? &(*it) : nullptr;
}

DetectorRunAccumulables& FindOrCreateDetectorAccumulablesByName(
    std::vector<DetectorRunAccumulables>& detectorAccumulables,
    const G4String& detectorName) {
    if (auto* accumulators = FindDetectorAccumulablesByName(detectorAccumulables, detectorName)) {
        return *accumulators;
    }

    detectorAccumulables.emplace_back(detectorName);
    return detectorAccumulables.back();
}

std::size_t ResetMergedDetectorAccumulables() {
    std::lock_guard<std::mutex> lock(gDetectorAccumulablesMutex);
    ++gDetectorAccumulablesGeneration;
    gMergedDetectorAccumulables.clear();
    return gDetectorAccumulablesGeneration;
}

std::size_t GetMergedDetectorAccumulablesGeneration() {
    std::lock_guard<std::mutex> lock(gDetectorAccumulablesMutex);
    return gDetectorAccumulablesGeneration;
}

void MergeDetectorAccumulablesIntoSharedStore(
    std::size_t generation,
    const std::vector<DetectorRunAccumulables>& detectorAccumulables) {
    if (detectorAccumulables.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(gDetectorAccumulablesMutex);
    if (generation != gDetectorAccumulablesGeneration) {
        return;
    }

    for (const auto& detectorAccumulablesEntry : detectorAccumulables) {
        auto& mergedDetectorAccumulables = FindOrCreateDetectorAccumulablesByName(
            gMergedDetectorAccumulables,
            detectorAccumulablesEntry.name);
        mergedDetectorAccumulables.Merge(detectorAccumulablesEntry);
    }
}

std::vector<DetectorRunAccumulables> SnapshotMergedDetectorAccumulables(std::size_t generation) {
    std::lock_guard<std::mutex> lock(gDetectorAccumulablesMutex);
    if (generation != gDetectorAccumulablesGeneration) {
        return {};
    }
    return gMergedDetectorAccumulables;
}

} // namespace

CalibratedDoseToWaterRunAccumulables::CalibratedDoseToWaterRunAccumulables(
    const G4String& /*detectorName*/) {
    value.reset();
    calibrationError.reset();
}

void CalibratedDoseToWaterRunAccumulables::Fill(const CalibratedDoseToWaterData& data) {
    value.fill(data.value);
    calibrationError.fill(data.calibrationError);
}

void CalibratedDoseToWaterRunAccumulables::Merge(
    const CalibratedDoseToWaterRunAccumulables& other) {
    value.add(&other.value);
    calibrationError.add(&other.calibrationError);
}

DetectorRunAccumulables::DetectorRunAccumulables(const G4String& detectorName)
    : name(detectorName),
      estimatedDoseToWater(detectorName) {
    totalEdep.reset();
    collectedCharge.reset();
    dose.reset();
}

void DetectorRunAccumulables::Merge(const DetectorRunAccumulables& other) {
    events += other.events;
    totalEdep.add(&other.totalEdep);
    collectedCharge.add(&other.collectedCharge);
    dose.add(&other.dose);
    estimatedDoseToWater.Merge(other.estimatedDoseToWater);
}

void DetectorAccumulablesStore::PrepareForRun(G4bool isMaster) {
    fLocalAccumulables.clear();
    if (!G4Threading::IsMultithreadedApplication()) {
        return;
    }

    if (isMaster) {
        fGeneration = ResetMergedDetectorAccumulables();
    } else {
        fGeneration = GetMergedDetectorAccumulablesGeneration();
    }
}

void DetectorAccumulablesStore::MergeWorkerResults(G4bool isMaster) {
    if (!G4Threading::IsMultithreadedApplication() || isMaster) {
        return;
    }
    MergeDetectorAccumulablesIntoSharedStore(fGeneration, fLocalAccumulables);
}

std::vector<DetectorRunAccumulables> DetectorAccumulablesStore::GetAccumulablesForSummary(
    G4bool isMaster) const {
    if (G4Threading::IsMultithreadedApplication() && isMaster) {
        return SnapshotMergedDetectorAccumulables(fGeneration);
    }
    return fLocalAccumulables;
}

DetectorRunAccumulables& DetectorAccumulablesStore::FindOrCreate(const G4String& detectorName) {
    return FindOrCreateDetectorAccumulablesByName(fLocalAccumulables, detectorName);
}

} // namespace MD1
