/*
 *
 * Geant4 MultiDetector Simulation
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

#ifndef CLINAC_TRUE_BEAM_MESSENGER_H
#define CLINAC_TRUE_BEAM_MESSENGER_H

// Geant4 Headers
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

// MultiDetector Headers
#include "geometry/beamline/ClinacTrueBeam.hh"

class ClinacTrueBeam;

/**
 * @class ClinacTrueBeamMessenger
 * @brief Messenger class to handle UI commands for ClinacTrueBeam.
 */
class ClinacTrueBeamMessenger : public G4UImessenger {
public:
    /**
     * @brief Constructor for ClinacTrueBeamMessenger.
     * @param clinac Pointer to the ClinacTrueBeam instance.
     */
    ClinacTrueBeamMessenger(ClinacTrueBeam* clinac);

    /**
     * @brief Destructor for ClinacTrueBeamMessenger.
     */
    ~ClinacTrueBeamMessenger() override;

    /**
     * @brief Set new values for the UI commands.
     * @param command UI command.
     * @param newValue New value for the command.
     */
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    /**
     * @brief Convert a string to three double values with unit.
     * @param paramString String containing the three double values and unit.
     * @param xval Reference to the first double value.
     * @param yval Reference to the second double value.
     * @param zval Reference to the third double value.
     */
    void ConvertToDoubleTrio(const G4String& paramString, G4double& xval, G4double& yval, G4double& zval);

    ClinacTrueBeam* fClinac; ///< Pointer to the clinac
    G4int fCurrentClinacID; ///< Current clinac ID

    G4UIcmdWithADoubleAndUnit* fSetJaw1XCmd; ///< Command to set Jaw1X apperture to the specified value
    G4UIcmdWithADoubleAndUnit* fSetJaw2XCmd; ///< Command to set Jaw2X apperture to the specified value
    G4UIcmdWithADoubleAndUnit* fSetJaw1YCmd; ///< Command to set Jaw1Y apperture to the specified value
    G4UIcmdWithADoubleAndUnit* fSetJaw2YCmd; ///< Command to set Jaw2Y apperture to the specified value
    G4UIcmdWithADoubleAndUnit* fRotateGantryToCmd; ///< Command to rotate the clinac gantry around X-axis to the specified angle
    G4UIcmdWithADoubleAndUnit* fRotateGantryCmd; ///< Command to rotate the clinac gantry around X-axis
    G4UIcmdWithADoubleAndUnit* fRotateCollimatorToCmd; ///< Command to rotate the clinac collimator around Z'-axis to the specified angle
    G4UIcmdWithADoubleAndUnit* fRotateCollimatorCmd; ///< Command to rotate the clinac collimator around Z'-axis

};

#endif // CLINAC_TRUE_BEAM_MESSENGER_H