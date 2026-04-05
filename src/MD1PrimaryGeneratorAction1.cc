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
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4GenericMessenger.hh"
#include "G4Exception.hh"

// MultiDetector Headers
#include "MD1PrimaryGeneratorAction1.hh"
#include "MD1Control.hh"
#include "MD1PhspSourceConfig.hh"

#include <memory>
#include <vector>

namespace MD1 {

namespace {

struct SharedPhspSourceState {
  std::vector<std::unique_ptr<G4IAEAphspReader>> readers;
  std::vector<G4String> sourceFiles;
  std::size_t nextReaderIndex = 0;
  G4double gantryAngle = 0.;
  G4double collimatorAngle = 0.;
  G4ThreeVector phspShift = G4ThreeVector(0., 0., 0. * cm);
  G4bool initialized = false;
  G4Mutex mutex = G4MUTEX_INITIALIZER;
};

SharedPhspSourceState& GetSharedPhspSourceState() {
  static SharedPhspSourceState state;
  return state;
}

void ApplySharedTransforms(SharedPhspSourceState& state) {
  for (auto& reader : state.readers) {
    reader->SetGlobalPhspTranslation(state.phspShift);
    reader->SetIsocenterPosition(state.phspShift);
    reader->SetGantryAngle(state.gantryAngle);
    reader->SetCollimatorAngle(state.collimatorAngle);
    reader->SetAbortOnNextReuseAfterEOF(true);
  }
}

void InitializeReadersLocked(SharedPhspSourceState& state) {
  if (state.initialized) {
    return;
  }

  state.sourceFiles = MD1PhspSourceConfig::GetInstance()->ResolveSourceBaseNames();

  if (state.sourceFiles.empty()) {
    G4Exception("MD1PrimaryGeneratorAction1::InitializeReadersLocked",
                "PHSPSourceListEmpty",
                FatalException,
                "No phase-space sources were configured. Use /MultiDetector1/beamline/clinac/phsp/* commands.");
    return;
  }

  state.readers.clear();
  state.readers.reserve(state.sourceFiles.size());

  for (const auto& file : state.sourceFiles) {
    state.readers.push_back(std::make_unique<G4IAEAphspReader>(file));
    G4cout << "IAEA Reader initialized with file: " << file << G4endl;
  }

  ApplySharedTransforms(state);
  state.nextReaderIndex = 0;
  state.initialized = true;
}

} // namespace

MD1PrimaryGeneratorAction1::MD1PrimaryGeneratorAction1()
  : G4VUserPrimaryGeneratorAction(),
    fGantryAngle(0.),
    fCollimatorAngle(0.) {

  fRotateGantryToMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/" );

  G4GenericMessenger::Command& fRotateGantryToCmd =
    fRotateGantryToMessenger->DeclareMethodWithUnit("rotateGantryPhSpTo", "deg",
                                                    &MD1PrimaryGeneratorAction1::RotateGantryTo,
                                                    "Rotate the clinac phase space around X-axis to the specified angle.");
  fRotateGantryToCmd.SetStates(G4State_Idle);

  fRotateGantryMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/" );

  G4GenericMessenger::Command& fRotateGantryCmd =
    fRotateGantryMessenger->DeclareMethodWithUnit("rotateGantryPhSp", "deg",
                                                  &MD1PrimaryGeneratorAction1::RotateGantry,
                                                  "Rotate the clinac phase space around X-axis.");
  fRotateGantryCmd.SetStates(G4State_Idle);

  fRotateCollimatorToMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/" );

  G4GenericMessenger::Command& fRotateCollimatorToCmd =
    fRotateCollimatorToMessenger->DeclareMethodWithUnit("rotateCollimatorPhSpTo", "deg",
                                                        &MD1PrimaryGeneratorAction1::RotateCollimatorTo,
                                                        "Rotate the clinac phase space around Z'-axis to the specified angle.");
  fRotateCollimatorToCmd.SetStates(G4State_Idle);

  fRotateCollimatorMessenger =
    new G4GenericMessenger(this, "/MultiDetector1/beamline/clinac/" );

  G4GenericMessenger::Command& fRotateCollimatorCmd =
    fRotateCollimatorMessenger->DeclareMethodWithUnit("rotateCollimatorPhSp", "deg",
                                                      &MD1PrimaryGeneratorAction1::RotateCollimator,
                                                      "Rotate the clinac phase space around Z'-axis.");
  fRotateCollimatorCmd.SetStates(G4State_Idle);

  G4cout << "MD1PrimaryGeneratorAction1 constructor called on thread "
         << G4Threading::G4GetThreadId() << G4endl;

  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  InitializeReadersLocked(state);
}

MD1PrimaryGeneratorAction1::~MD1PrimaryGeneratorAction1() {
  delete fRotateGantryToMessenger;
  delete fRotateGantryMessenger;
  delete fRotateCollimatorToMessenger;
  delete fRotateCollimatorMessenger;
}

void MD1PrimaryGeneratorAction1::GeneratePrimaries(G4Event* anEvent) {
  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  InitializeReadersLocked(state);

  if (state.readers.empty()) {
    G4Exception("MD1PrimaryGeneratorAction1::GeneratePrimaries",
                "PHSPNoReaders",
                FatalException,
                "No phase-space readers are available.");
    return;
  }

  auto& reader = state.readers[state.nextReaderIndex];
  reader->GeneratePrimaryVertex(anEvent);
  state.nextReaderIndex = (state.nextReaderIndex + 1) % state.readers.size();
}

void MD1PrimaryGeneratorAction1::RotateGantryTo(const G4double& angle) {
  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  state.gantryAngle = angle;
  ApplySharedTransforms(state);
}

void MD1PrimaryGeneratorAction1::RotateGantry(const G4double& delta) {
  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  state.gantryAngle += delta;
  ApplySharedTransforms(state);
}

void MD1PrimaryGeneratorAction1::RotateCollimatorTo(const G4double& angle) {
  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  state.collimatorAngle = angle;
  ApplySharedTransforms(state);
}

void MD1PrimaryGeneratorAction1::RotateCollimator(const G4double& delta) {
  auto& state = GetSharedPhspSourceState();
  G4AutoLock l(&state.mutex);
  state.collimatorAngle += delta;
  ApplySharedTransforms(state);
}

const G4IAEAphspReader* MD1PrimaryGeneratorAction1::GetParticleSource() const {
  auto& state = GetSharedPhspSourceState();
  if (state.readers.empty()) {
    return nullptr;
  }
  return state.readers.front().get();
}

} // namespace MD1
