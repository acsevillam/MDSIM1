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

#ifndef MD1Control_h
#define MD1Control_h 1

// MD1 Headers
#include "MD1ControlMessenger.hh"
#include "G4UserTrackingAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4VSensitiveDetector.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

class MD1ControlMessenger ;

namespace MD1 {

class MD1Control {

public:
    /// Static method returning an instance of Control.
    static MD1Control* GetInstance() ;
    /// Static method killing the instance.
    static void Kill() ;

    void Setup(int argc,char** argv);

    inline G4UserTrackingAction* GetTrackingAction() const { return fTrackingAction;}

    inline void SetTrackingAction(G4UserTrackingAction* aTrackingAction){fTrackingAction=aTrackingAction;}

    inline G4UserSteppingAction* GetSteppingAction() const { return fSteppingAction;}

    inline void SetSteppingAction(G4UserSteppingAction* aSteppingAction){fSteppingAction=aSteppingAction;}

    inline G4UserSteppingAction* GetSensitiveDetectorAction() const{ return fSensitiveDetectorAction;}

    inline void SetSensitiveVolumeAction(G4UserSteppingAction* aSensitiveDetectorAction){fSensitiveDetectorAction=aSensitiveDetectorAction;}

    inline G4int GetPrimaryGeneratorType() const{ return fPrimaryGeneratorType;}

    inline void SetPrimaryGeneratorType(G4int aPrimaryGeneratorType){fPrimaryGeneratorType=aPrimaryGeneratorType;}

private:
    // Constructor
	MD1Control();
	// Destructor
	virtual ~MD1Control();

    // Singleton control instance.
    static MD1Control*			instance ;

    MD1ControlMessenger*		fMD1ControlMessenger ;

    G4UserTrackingAction*		fTrackingAction ;
    G4UserSteppingAction*    	fSteppingAction ;
    G4UserSteppingAction*    	fSensitiveDetectorAction ;

    G4int                       fPrimaryGeneratorType = 1;

};

};

#endif // MD1Control_h
