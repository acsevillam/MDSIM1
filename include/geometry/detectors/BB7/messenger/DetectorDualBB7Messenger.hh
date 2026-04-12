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

#ifndef DETECTOR_DUAL_BB7_MESSENGER_H
#define DETECTOR_DUAL_BB7_MESSENGER_H

// Geant4 Headers
#include "G4UImessenger.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

// MultiDetector Headers
#include "geometry/detectors/BB7/geometry/DetectorDualBB7.hh"

class DetectorDualBB7;

/**
 * @class DetectorDualBB7Messenger
 * @brief Messenger class to handle UI commands for DetectorDualBB7.
 */
class DetectorDualBB7Messenger : public G4UImessenger {
public:
    /**
     * @brief Constructor for DetectorDualBB7Messenger.
     * @param detector Pointer to the DetectorDualBB7 instance.
     */
    DetectorDualBB7Messenger(DetectorDualBB7* detector);

    /**
     * @brief Destructor for DetectorDualBB7Messenger.
     */
    ~DetectorDualBB7Messenger() override;

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

    DetectorDualBB7* fDetector; ///< Pointer to the detector
    G4int fCurrentDetectorID; ///< Current detector ID

    G4UIcmdWithAnInteger* fDetectorIDCmd; ///< Command to set the current detector ID
    G4UIcmdWith3VectorAndUnit* fTranslateCmd; ///< Command to translate the detector
    G4UIcmdWith3VectorAndUnit* fTranslateToCmd; ///< Command to translate the detector to a position
    G4UIcmdWithADoubleAndUnit* fRotateXCmd; ///< Command to rotate the detector around X-axis
    G4UIcmdWithADoubleAndUnit* fRotateYCmd; ///< Command to rotate the detector around Y-axis
    G4UIcmdWithADoubleAndUnit* fRotateZCmd; ///< Command to rotate the detector around Z-axis
    G4UIcommand* fRotateToCmd; ///< Command to set the rotation angles of the detector
    G4UIcmdWithADouble* fSetCalibrationFactorCmd; ///< Command to set calibration factor
    G4UIcmdWithADouble* fSetCalibrationFactorErrorCmd; ///< Command to set calibration factor error
    G4UIcommand* fAddGeometryToCmd; ///< Command to add geometry to the detector
	G4UIcmdWithAnInteger* fRemoveGeometryCmd; ///< Command to remove geometry from the detector.
};

#endif // DETECTOR_DUAL_BB7_MESSENGER_H
