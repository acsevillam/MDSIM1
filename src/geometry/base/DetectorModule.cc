#include "geometry/base/DetectorModule.hh"

std::unique_ptr<DetectorRuntimeState> DetectorModule::CreateRuntimeState() const {
    return std::make_unique<DetectorRuntimeState>();
}

void DetectorModule::PrepareForRun(DetectorRuntimeState& /*runtimeState*/, G4bool /*isMaster*/) {}

void DetectorModule::MergeRunResults(DetectorRuntimeState& /*runtimeState*/, G4bool /*isMaster*/) {}

void DetectorModule::BeginOfEvent(const G4Event* /*event*/, DetectorRuntimeState& /*runtimeState*/) {}

std::vector<G4String> DetectorModule::GetSummaryLabels() const {
    return {GetName()};
}

G4String DetectorModule::GetSummaryLabel(G4int /*detectorID*/) const {
    return GetName();
}

void DetectorModule::PrintResults(const G4Run* /*run*/,
                                  const DetectorRuntimeState& /*runtimeState*/,
                                  const MD1::DetectorPrintContext& /*context*/) const {}
