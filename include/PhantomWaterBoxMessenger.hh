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

#ifndef PHANTOM_WATER_BOX_MESSENGER_H
#define PHANTOM_WATER_BOX_MESSENGER_H

// Geant4 Headers
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

// MultiDetector Headers
#include "PhantomWaterBox.hh"

class PhantomWaterBox;

/**
 * @class PhantomWaterBoxMessenger
 * @brief Messenger class to handle UI commands for PhantomWaterBox.
 */
class PhantomWaterBoxMessenger : public G4UImessenger {
public:
    /**
     * @brief Constructor for PhantomWaterBoxMessenger.
     * @param phantom Pointer to the PhantomWaterBox instance.
     */
    PhantomWaterBoxMessenger(PhantomWaterBox* phantom);

    /**
     * @brief Destructor for PhantomWaterBoxMessenger.
     */
    ~PhantomWaterBoxMessenger() override;

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

    PhantomWaterBox* fPhantom; ///< Pointer to the phantom
    G4int fCurrentPhantomID; ///< Current phantom ID

    G4UIcmdWithAnInteger* fPhantomIDCmd; ///< Command to set the current phantom ID
    G4UIcmdWith3VectorAndUnit* fTranslateCmd; ///< Command to translate the phantom
    G4UIcmdWith3VectorAndUnit* fTranslateToCmd; ///< Command to translate the phantom to a position
    G4UIcmdWithADoubleAndUnit* fRotateXCmd; ///< Command to rotate the phantom around X-axis
    G4UIcmdWithADoubleAndUnit* fRotateYCmd; ///< Command to rotate the phantom around Y-axis
    G4UIcmdWithADoubleAndUnit* fRotateZCmd; ///< Command to rotate the phantom around Z-axis
    G4UIcommand* fRotateToCmd; ///< Command to set the rotation angles of the phantom
    G4UIcommand* fAddGeometryToCmd; ///< Command to add geometry to the phantom
	G4UIcmdWithAnInteger* fRemoveGeometryCmd; ///< Command to remove geometry from the phantom.
};

#endif // PHANTOM_WATER_BOX_MESSENGER_H