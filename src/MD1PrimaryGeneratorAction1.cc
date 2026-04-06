/*
 *
 * Geant4 MultiDetector Simulation v1
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

// Geant4 Headers
#include "G4AutoLock.hh"
#include "G4Exception.hh"
#include "G4GenericMessenger.hh"
#include "G4MTRunManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"

// MultiDetector Headers
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1PhspSourceConfig.hh"

#include <atomic>

namespace MD1 {

namespace {

struct SharedPhspTransformState {
  G4double gantryAngle = 0.;
  G4double collimatorAngle = 0.;
  G4ThreeVector phspShift = G4ThreeVector(0., 0., 0. * cm);
  std::atomic<G4int> version{0};
  G4Mutex mutex = G4MUTEX_INITIALIZER;
};

struct PhspTransformSnapshot {
  G4double gantryAngle = 0.;
  G4double collimatorAngle = 0.;
  G4ThreeVector phspShift = G4ThreeVector(0., 0., 0. * cm);
  G4int version = 0;
};

SharedPhspTransformState& GetSharedPhspTransformState() {
  static SharedPhspTransformState state;
  return state;
}

PhspTransformSnapshot GetTransformSnapshot() {
  auto& state = GetSharedPhspTransformState();
  G4AutoLock lock(&state.mutex);

  PhspTransformSnapshot snapshot;
  snapshot.gantryAngle = state.gantryAngle;
  snapshot.collimatorAngle = state.collimatorAngle;
  snapshot.phspShift = state.phspShift;
  snapshot.version = state.version.load(std::memory_order_relaxed);
  return snapshot;
}

G4int GetTransformVersion() {
  return GetSharedPhspTransformState().version.load(std::memory_order_acquire);
}

G4int GetWorkerIndex() {
  if (!G4Threading::IsMultithreadedApplication()) {
    return 0;
  }

  const G4int threadId = G4Threading::G4GetThreadId();
  return (threadId >= 0) ? threadId : 0;
}

G4int GetTotalWorkers() {
  if (!G4Threading::IsMultithreadedApplication()) {
    return 1;
  }

  if (const auto* masterRunManager = G4MTRunManager::GetMasterRunManager()) {
    if (masterRunManager->GetNumberOfThreads() > 0) {
      return masterRunManager->GetNumberOfThreads();
    }
  }

  if (const auto* runManager = G4RunManager::GetRunManager()) {
    if (runManager->GetNumberOfThreads() > 0) {
      return runManager->GetNumberOfThreads();
    }
  }

  return 1;
}

} // namespace

MD1PrimaryGeneratorAction1::MD1PrimaryGeneratorAction1()
  : G4VUserPrimaryGeneratorAction(),
    fHasAppliedReaderConfiguration(false),
    fAppliedSourceConfigVersion(-1),
    fAppliedTransformVersion(-1),
    fAppliedWorkerIndex(-1),
    fAppliedTotalWorkers(0),
    fRotateGantryToMessenger(nullptr),
    fRotateGantryMessenger(nullptr),
    fRotateCollimatorToMessenger(nullptr),
    fRotateCollimatorMessenger(nullptr) {

  fRotateGantryToMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/");

  G4GenericMessenger::Command& fRotateGantryToCmd =
    fRotateGantryToMessenger->DeclareMethodWithUnit("rotateGantryPhSpTo",
                                                    "deg",
                                                    &MD1PrimaryGeneratorAction1::RotateGantryTo,
                                                    "Rotate the clinac phase space around X-axis to the specified angle.");
  fRotateGantryToCmd.SetStates(G4State_Idle);

  fRotateGantryMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/");

  G4GenericMessenger::Command& fRotateGantryCmd =
    fRotateGantryMessenger->DeclareMethodWithUnit("rotateGantryPhSp",
                                                  "deg",
                                                  &MD1PrimaryGeneratorAction1::RotateGantry,
                                                  "Rotate the clinac phase space around X-axis.");
  fRotateGantryCmd.SetStates(G4State_Idle);

  fRotateCollimatorToMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/");

  G4GenericMessenger::Command& fRotateCollimatorToCmd =
    fRotateCollimatorToMessenger->DeclareMethodWithUnit("rotateCollimatorPhSpTo",
                                                        "deg",
                                                        &MD1PrimaryGeneratorAction1::RotateCollimatorTo,
                                                        "Rotate the clinac phase space around Z'-axis to the specified angle.");
  fRotateCollimatorToCmd.SetStates(G4State_Idle);

  fRotateCollimatorMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/");

  G4GenericMessenger::Command& fRotateCollimatorCmd =
    fRotateCollimatorMessenger->DeclareMethodWithUnit("rotateCollimatorPhSp",
                                                      "deg",
                                                      &MD1PrimaryGeneratorAction1::RotateCollimator,
                                                      "Rotate the clinac phase space around Z'-axis.");
  fRotateCollimatorCmd.SetStates(G4State_Idle);
}

MD1PrimaryGeneratorAction1::~MD1PrimaryGeneratorAction1() {
  delete fRotateGantryToMessenger;
  delete fRotateGantryMessenger;
  delete fRotateCollimatorToMessenger;
  delete fRotateCollimatorMessenger;
}

void MD1PrimaryGeneratorAction1::ApplyReaderConfiguration(G4IAEAphspReader& reader,
                                                          G4IAEAphspReader::EOFPolicy eofPolicy,
                                                          G4double gantryAngle,
                                                          G4double collimatorAngle,
                                                          const G4ThreeVector& phspShift,
                                                          G4int workerIndex,
                                                          G4int totalWorkers) const {
  reader.SetTotalParallelRuns(totalWorkers);
  reader.SetParallelRun(workerIndex + 1);
  reader.SetGlobalPhspTranslation(phspShift);
  reader.SetIsocenterPosition(phspShift);
  reader.SetGantryAngle(gantryAngle);
  reader.SetCollimatorAngle(collimatorAngle);
  reader.SetEOFPolicy(eofPolicy);
}

void MD1PrimaryGeneratorAction1::EnsureReadersAreSynchronized() {
  auto* sourceConfig = MD1PhspSourceConfig::GetInstance();
  const G4int sourceVersion = sourceConfig->GetVersion();
  const G4int transformVersion = GetTransformVersion();
  const G4int workerIndex = GetWorkerIndex();
  const G4int totalWorkers = GetTotalWorkers();

  if (fHasAppliedReaderConfiguration &&
      fAppliedSourceConfigVersion == sourceVersion &&
      fAppliedTransformVersion == transformVersion &&
      fAppliedWorkerIndex == workerIndex &&
      fAppliedTotalWorkers == totalWorkers) {
    return;
  }

  const auto sourceSnapshot = sourceConfig->GetSnapshot();
  const auto transformSnapshot = GetTransformSnapshot();

  if (sourceSnapshot.sourceFiles.empty()) {
    G4Exception("MD1PrimaryGeneratorAction1::EnsureReadersAreSynchronized",
                "PHSPSourceListEmpty",
                FatalException,
                "No phase-space sources were configured. Use /MultiDetector1/beamline/clinac/phsp/* commands.");
    return;
  }

  ReaderConfiguration nextConfiguration;
  nextConfiguration.eofPolicy = G4IAEAphspReader::ParseEOFPolicy(sourceSnapshot.eofPolicy);
  nextConfiguration.gantryAngle = transformSnapshot.gantryAngle;
  nextConfiguration.collimatorAngle = transformSnapshot.collimatorAngle;
  nextConfiguration.phspShift = transformSnapshot.phspShift;
  nextConfiguration.workerIndex = workerIndex;
  nextConfiguration.totalWorkers = totalWorkers;

  const G4bool needsReaderReset =
    fReaders.size() != sourceSnapshot.sourceFiles.size() ||
    fAppliedSourceFiles != sourceSnapshot.sourceFiles ||
    fAppliedWorkerIndex != workerIndex ||
    fAppliedTotalWorkers != totalWorkers;
  const G4bool needsReaderUpdate =
    needsReaderReset ||
    !fHasAppliedReaderConfiguration ||
    fAppliedSourceConfigVersion != sourceSnapshot.version ||
    fAppliedTransformVersion != transformSnapshot.version;

  if (needsReaderReset) {
    fReaders.clear();
    fReaders.resize(sourceSnapshot.sourceFiles.size());
    fAppliedSourceFiles = sourceSnapshot.sourceFiles;
  }

  if (needsReaderUpdate) {
    for (auto& reader : fReaders) {
      if (reader != nullptr) {
        ApplyReaderConfiguration(*reader,
                                 nextConfiguration.eofPolicy,
                                 nextConfiguration.gantryAngle,
                                 nextConfiguration.collimatorAngle,
                                 nextConfiguration.phspShift,
                                 nextConfiguration.workerIndex,
                                 nextConfiguration.totalWorkers);
      }
    }
  }

  fAppliedReaderConfiguration = nextConfiguration;
  fHasAppliedReaderConfiguration = true;
  fAppliedSourceConfigVersion = sourceSnapshot.version;
  fAppliedTransformVersion = transformSnapshot.version;
  fAppliedWorkerIndex = workerIndex;
  fAppliedTotalWorkers = totalWorkers;
}

G4IAEAphspReader& MD1PrimaryGeneratorAction1::GetOrCreateReader(const std::size_t sourceIndex) {
  if (!fHasAppliedReaderConfiguration || sourceIndex >= fAppliedSourceFiles.size()) {
    G4Exception("MD1PrimaryGeneratorAction1::GetOrCreateReader",
                "PHSPReaderConfigurationUnavailable",
                FatalException,
                "The requested phase-space reader is not available.");
  }

  auto& reader = fReaders[sourceIndex];
  if (reader == nullptr) {
    reader = std::make_unique<G4IAEAphspReader>(fAppliedSourceFiles[sourceIndex]);
    ApplyReaderConfiguration(*reader,
                             fAppliedReaderConfiguration.eofPolicy,
                             fAppliedReaderConfiguration.gantryAngle,
                             fAppliedReaderConfiguration.collimatorAngle,
                             fAppliedReaderConfiguration.phspShift,
                             fAppliedReaderConfiguration.workerIndex,
                             fAppliedReaderConfiguration.totalWorkers);
  }

  return *reader;
}

void MD1PrimaryGeneratorAction1::GeneratePrimaries(G4Event* anEvent) {
  EnsureReadersAreSynchronized();

  if (fReaders.empty()) {
    G4Exception("MD1PrimaryGeneratorAction1::GeneratePrimaries",
                "PHSPNoReaders",
                FatalException,
                "No phase-space readers are available.");
    return;
  }

  const std::size_t sourceIndex =
    static_cast<std::size_t>(anEvent->GetEventID()) % fReaders.size();
  GetOrCreateReader(sourceIndex).GeneratePrimaryVertex(anEvent);
}

void MD1PrimaryGeneratorAction1::RotateGantryTo(const G4double& angle) {
  auto& state = GetSharedPhspTransformState();
  G4AutoLock lock(&state.mutex);
  state.gantryAngle = angle;
  state.version.fetch_add(1, std::memory_order_release);
}

void MD1PrimaryGeneratorAction1::RotateGantry(const G4double& delta) {
  auto& state = GetSharedPhspTransformState();
  G4AutoLock lock(&state.mutex);
  state.gantryAngle += delta;
  state.version.fetch_add(1, std::memory_order_release);
}

void MD1PrimaryGeneratorAction1::RotateCollimatorTo(const G4double& angle) {
  auto& state = GetSharedPhspTransformState();
  G4AutoLock lock(&state.mutex);
  state.collimatorAngle = angle;
  state.version.fetch_add(1, std::memory_order_release);
}

void MD1PrimaryGeneratorAction1::RotateCollimator(const G4double& delta) {
  auto& state = GetSharedPhspTransformState();
  G4AutoLock lock(&state.mutex);
  state.collimatorAngle += delta;
  state.version.fetch_add(1, std::memory_order_release);
}

const G4IAEAphspReader* MD1PrimaryGeneratorAction1::GetParticleSource() const {
  if (fReaders.empty() || fReaders.front() == nullptr) {
    return nullptr;
  }
  return fReaders.front().get();
}

} // namespace MD1
