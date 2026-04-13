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

#include "globals.hh"

namespace MD1 {

class MD1ControlMessenger;

class MD1Control {

public:
    /// Static method returning an instance of Control.
    static MD1Control* GetInstance() ;
    /// Static method killing the instance.
    static void Kill() ;

    inline G4int GetPrimaryGeneratorType() const{ return fPrimaryGeneratorType;}

    void SetPrimaryGeneratorType(G4int aPrimaryGeneratorType);
    void CenterViewOnZAxis();
    void ToggleFocusAxes();

private:
    // Constructor
	MD1Control();
	// Destructor
	virtual ~MD1Control();

    // Singleton control instance.
    static MD1Control*			instance ;

    MD1ControlMessenger*		fMD1ControlMessenger ;

    G4int                       fPrimaryGeneratorType = 1;

};

};

#endif // MD1Control_h
