/*
 * Copyright (C) 2009 International Atomic Energy Agency
 * -----------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *-----------------------------------------------------------------------------
 *
 *   AUTHORS:
 *
 *   Miguel Antonio Cortes Giraldo
 *   e-mail: miancortes -at- us.es
 *   University of Seville, Spain
 *   Dep. Fisica Atomica, Molecular y Nuclear
 *   Ap. 1065, E-41080 Seville, SPAIN
 *   Phone: +34-954550928; Fax: +34-954554445
 *
 *   Jose Manuel Quesada Molina, PhD
 *   e-mail: quesada -at- us.es
 *   University of Seville, Spain
 *   Dep. Fisica Atomica, Molecular y Nuclear
 *   Ap. 1065, E-41080 Seville, SPAIN
 *   Phone: +34-954559508; Fax: +34-954554445
 *
 *   Roberto Capote Noy, PhD
 *   e-mail: R.CapoteNoy -at- iaea.org (rcapotenoy -at- yahoo.com)
 *   International Atomic Energy Agency
 *   Nuclear Data Section, P.O.Box 100
 *   Wagramerstrasse 5, Vienna A-1400, AUSTRIA
 *   Phone: +431-260021713; Fax: +431-26007
 *
 **********************************************************************************
 * For documentation
 * see http://www-nds.iaea.org/phsp
 *
 * - 07/12/2009: public version 1.0
 *
 **********************************************************************************/


#ifndef G4IAEAphspReader_h
#define G4IAEAphspReader_h 1

#include "G4VPrimaryGenerator.hh"

#include "iaea_phsp.h"

#include <cstddef>
#include <string>
#include <vector>

#include "globals.hh"
#include "G4ThreeVector.hh"


class G4Event;


class G4IAEAphspReader :public G4VPrimaryGenerator
{

  // ========== Constructors and destructors ==========

public:

  enum class EOFPolicy
  {
    Abort,
    Restart,
    Stop,
    Synthetic
  };

  struct SyntheticParticleRecord
  {
    G4int type = 0;
    G4double energy = 0.;
    G4ThreeVector position;
    G4ThreeVector momentum;
    G4double weight = 0.;
    std::vector<G4double> extraFloats;
    std::vector<G4long> extraInts;
  };

  struct SyntheticEventRecord
  {
    G4int nStat = 1;
    std::vector<SyntheticParticleRecord> particles;
  };

  // 'filename' must include the path if needed, but NOT the extension

  G4IAEAphspReader(char* filename);
  G4IAEAphspReader(G4String filename);

private:
  G4IAEAphspReader()
  { G4Exception("Cannot use G4IAEAphspReader void constructor", "", JustWarning, ""); }

public:

  ~G4IAEAphspReader();


  // ========== Class Methods ==========

public:

  void GeneratePrimaryVertex(G4Event* evt);   // Mandatory
  static EOFPolicy ParseEOFPolicy(const G4String& policyName);

private:

  void InitializeMembers();
  void InitializeSource(char* filename);
  void ClearCurrentParticleData();
  void StoreParticleRecord(G4int type, G4double energy, const G4ThreeVector& position,
                           const G4ThreeVector& momentum, G4double weight,
                           const IAEA_Float* extraFloats, const IAEA_I32* extraInts);
  void ReadAndStoreFirstParticle();
  void PrepareThisEvent();
  void ReadThisEvent();
  void PrepareSyntheticEvents();
  void ActivateSyntheticMode();
  void LoadSyntheticEvent(std::size_t eventIndex);
  void HandleEndOfFile(G4Event* evt);
  void StopRunAtEOF(G4Event* evt);
  static const char* EOFPolicyName(EOFPolicy policy);
  void GeneratePrimaryParticles(G4Event* evt);
  void PerformRotations(G4ThreeVector& mom);
  void PerformGlobalRotations(G4ThreeVector& mom);
  void PerformHeadRotations(G4ThreeVector& mom);
  void RestartSourceFile();


  // ========== Set/Get inline methods ==========

public:

  inline void SetTotalParallelRuns(G4int nParallelRuns)
  {
    if (nParallelRuns <= 0)
      {
        G4Exception("G4IAEAphspReader::SetTotalParallelRuns()",
                    "IAEAreaderInvalidParallelRuns",
                    FatalException,
                    "Total parallel runs must be strictly positive.");
        return;
      }
    theTotalParallelRuns = nParallelRuns;
  }
  inline void SetParallelRun(G4int parallelRun)
  {
    if (parallelRun < 1 || parallelRun > theTotalParallelRuns)
      {
        G4Exception("G4IAEAphspReader::SetParallelRun()",
                    "IAEAreaderInvalidParallelRun",
                    FatalException,
                    "Parallel run index must be within [1, total parallel runs].");
        return;
      }
    theParallelRun = parallelRun;
  }
  inline void SetTimesRecycled(G4int ntimes) {theTimesRecycled = ntimes;}

  inline void SetGlobalPhspTranslation(const G4ThreeVector & pos) {theGlobalPhspTranslation = pos;}
  inline void SetRotationOrder(G4int ord) {theRotationOrder = ord;}
  inline void SetRotationX(G4double alpha) {theAlpha = alpha;}
  inline void SetRotationY(G4double beta) {theBeta = beta;}
  inline void SetRotationZ(G4double gamma) {theGamma = gamma;}
  inline void SetIsocenterPosition(const G4ThreeVector & pos) {theIsocenterPosition = pos;}
  void SetCollimatorRotationAxis(const G4ThreeVector & axis);
  void SetGantryRotationAxis(const G4ThreeVector & axis);
  inline void SetCollimatorAngle(G4double ang) {theCollimatorAngle = ang;}
  inline void SetGantryAngle(G4double ang) {theGantryAngle = ang;}
  inline void SetEOFPolicy(EOFPolicy policy) { theEOFPolicy = policy; }
  void SetEOFPolicy(const G4String& policyName);
  inline EOFPolicy GetEOFPolicy() const { return theEOFPolicy; }
  inline G4String GetEOFPolicyName() const { return EOFPolicyName(theEOFPolicy); }
  inline void SetAbortOnNextReuseAfterEOF(G4bool abortOnReuse)
  {
    theEOFPolicy = abortOnReuse ? EOFPolicy::Abort : EOFPolicy::Restart;
  }


  inline G4String GetFileName() const {return theFileName;}
  inline G4int GetSourceReadId() const {return theSourceReadId;}
  inline G4long GetOriginalHistories() const {return theOriginalHistories;}
  inline G4long GetUsedOriginalParticles() const {return theUsedOriginalParticles;}
  inline G4long GetTotalParticles() const {return theTotalParticles;}
  inline G4int GetNumberOfExtraFloats() const {return theNumberOfExtraFloats;}
  inline G4int GetNumberOfExtraInts() const {return theNumberOfExtraInts;}
  inline std::vector<G4int>* GetExtraFloatsTypes() const {return theExtraFloatsTypes;}
  inline std::vector<G4int>* GetExtraIntsTypes() const {return theExtraIntsTypes;}
  G4long GetTotalParticlesOfType(G4String type) const;
  G4double GetConstantVariable(const G4int index) const;

  inline std::vector<G4int>* GetParticleTypeVector() const {return theParticleTypeVector;}
  inline std::vector<G4double>* GetEnergyVector() const {return theEnergyVector;}
  inline std::vector<G4ThreeVector>* GetPositionVector() const {return thePositionVector;}
  inline std::vector<G4ThreeVector>* GetMomentumVector() const {return theMomentumVector;}
  inline std::vector<G4double>* GetWeightVector() const {return theWeightVector;}
  inline std::vector< std::vector<G4double> >* GetExtraFloatsVector() const {return theExtraFloatsVector;}
  inline std::vector< std::vector<G4long> >* GetExtraIntsVector() const {return theExtraIntsVector;}

  inline G4int GetTotalParallelRuns() const {return theTotalParallelRuns;}
  inline G4int GetParallelRun() const {return theParallelRun;}
  inline G4int GetTimesRecycled() const {return theTimesRecycled;}

  inline G4ThreeVector GetGlobalPhspTranslation() const {return theGlobalPhspTranslation;}
  inline G4int GetRotationOrder() const {return theRotationOrder;}
  inline G4double GetRotationX() const {return theAlpha;}
  inline G4double GetRotationY() const {return theBeta;}
  inline G4double GetRotationZ() const {return theGamma;}
  inline G4ThreeVector GetIsocenterPosition() const {return theIsocenterPosition;}
  inline G4double GetCollimatorAngle() const {return theCollimatorAngle;}
  inline G4double GetGantryAngle() const {return theGantryAngle;}
  inline G4ThreeVector GetCollimatorRotationAxis() const {return theCollimatorRotAxis;}
  inline G4ThreeVector GetGantryRotationAxis() const {return theGantryRotAxis;}


  // ========== Data members ==========

private:

  // ----------------------
  // FILE GLOBAL PROPERTIES
  // ----------------------

  G4String theFileName;
  // Must include the path, but NOT the IAEA extension

  G4int theSourceReadId;
  // The Id the file source has for the IAEA routines

  static const G4int theAccessRead = 1;
  // A value needed to open the file in the IAEA codes

  G4long theOriginalHistories;
  // Number of original histories which generated the phase space file

  G4long theTotalParticles;
  // Number of particles stored in the phase space file

  G4int theNumberOfExtraFloats, theNumberOfExtraInts;
  // Number of extra variables stored for each particle

  std::vector<G4int>* theExtraFloatsTypes; 
  std::vector<G4int>* theExtraIntsTypes;
  // Identification to classify the different extra variables

  // ---------------------
  // PARTICLE PROPERTIES
  // ---------------------

  std::vector<G4int>* theParticleTypeVector;
  std::vector<G4double>* theEnergyVector;
  std::vector<G4ThreeVector>* thePositionVector;
  std::vector<G4ThreeVector>* theMomentumVector;
  std::vector<G4double>* theWeightVector;
  std::vector< std::vector<G4double> >* theExtraFloatsVector;
  std::vector< std::vector<G4long> >* theExtraIntsVector;

  // -------------------
  // COUNTERS AND FLAGS
  // -------------------

  G4int theTotalParallelRuns;
  // For parallel runs, number of fragments in which the PSF is divided into

  G4int theParallelRun;
  // Sets the fragment of PSF where the particles must be read from

  G4int theTimesRecycled;
  // Set the number of times that each particle is recycled (not repeated)

  G4int theNStat;
  // Decides how many events should pass before throwing a new particle

  G4long theUsedOriginalParticles;
  // Variable that stores the number of particles readed so far

  G4long theCurrentParticle;
  // Number to store the current particle position in PSF

  G4bool theEndOfFile;
  // Flag active when the file has reached the end

  G4bool theLastGenerated;
  // Flag active only when the last particle has been simulated

  EOFPolicy theEOFPolicy;
  // Policy applied when the phase-space source reaches EOF

  G4bool theSyntheticModeActive;
  // True once the reader switches from physical PHSP reads to synthetic sampling

  G4bool theSyntheticModeLogged;
  // Avoid repeating the EOF-to-synthetic transition log for the same source

  G4long theSyntheticEventsGenerated;
  // Number of synthetic events already sampled after EOF

  std::vector<SyntheticEventRecord> theSyntheticEventPool;
  // Empirical event pool used to resample complete PHSP histories

  G4bool theHasGeneratedAtLeastOneEvent;
  // Tracks whether the source has already generated at least one event

  // ------------------------
  // SPATIAL TRANSFORMATIONS
  // ------------------------

  G4ThreeVector theGlobalPhspTranslation;
  // Global translation performed to particles

  G4int theRotationOrder;
  // Variable to decide first, second and third rotations
  // For example, 132 means rotations using X, Z and Y global axis

  G4double theAlpha, theBeta, theGamma;
  // Angles of rotations around global axis

  G4ThreeVector theIsocenterPosition;
  // Position of the isocenter if needed

  G4double theCollimatorAngle, theGantryAngle;
  G4ThreeVector theCollimatorRotAxis, theGantryRotAxis;
  // Angles and axis of isocentric rotations in the machine
  // The collimator ALWAYS rotates first.

};

#endif
