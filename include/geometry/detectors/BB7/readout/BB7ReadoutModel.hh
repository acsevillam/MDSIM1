/*
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

#ifndef BB7_READOUT_MODEL_H
#define BB7_READOUT_MODEL_H

#include "geometry/detectors/BB7/readout/BB7ReadoutParameters.hh"

class BB7ReadoutModel {
public:
    static BB7ReadoutParameters Build();
};

#endif // BB7_READOUT_MODEL_H
