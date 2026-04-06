#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(MultiDetector1)
enable_testing()

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
set(MDSIM1_DATA_DIR "${PROJECT_SOURCE_DIR}" CACHE PATH
  "Directory that contains external PHSP/runtime datasets used by MDSIM1.")
option(MDSIM1_COPY_RUNTIME_DATA
  "Copy large PHSP/reference datasets into the build tree for a self-contained local build"
  OFF)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
file(GLOB_RECURSE MULTIDETECTOR1_SOURCES CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/src/*.cc
)

file(GLOB_RECURSE MULTIDETECTOR1_HEADERS CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/include/*.hh
)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(MultiDetector1 MultiDetector1.cc ${MULTIDETECTOR1_SOURCES} ${MULTIDETECTOR1_HEADERS})
target_include_directories(MultiDetector1 PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(MultiDetector1 PRIVATE MDSIM1_DATA_DIR_DEFAULT="${MDSIM1_DATA_DIR}")
target_link_libraries(MultiDetector1 PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory
#
set(MDSIM1_RUNTIME_FILES
  mac/vis.mac
  mac/init.mac
  mac/scorers1.mac
  mac/tests/invalid_eof_policy.mac
  mac/tests/idle_detector_refresh.mac
  mac/tests/idle_detector_remove.mac
  analysis/README.md
  beam/photon6MV.txt
  beam/Co-60.txt
  beam/circularplane.txt
  beam/squareplane.txt
  src/geometry/detectors/cube/geometry/CubeCalibrationTable.dat
  input/dosimetry/Calibration.in
  input/dosimetry/templates/Calibration.intmp
  input/dosimetry/Validation.in
  input/dosimetry/templates/Validation.intmp
  input/detectors/BB7/BB7.in
  input/detectors/BB7/BB7Calibration.in
  input/detectors/BB7/templates/BB7.intmp
  input/detectors/BB7/templates/BB7Calibration.intmp
  input/detectors/cube/Cube.in
  input/detectors/cube/CubeCalibration.in
  input/detectors/cube/templates/Cube.intmp
  input/detectors/cube/templates/CubeCalibration.intmp
  input/dosimetry/DoseMap3D.in
  input/benchmark/set1.in
  input/benchmark/set2.in
  input/benchmark/set3.in
  jobs/dosimetry/launch_validation_phsp_sweep.sh
  jobs/dosimetry/run_validation_phsp_sweep.sh
  jobs/dosimetry/launch_calibration_dmax_average.sh
  jobs/dosimetry/run_calibration_dmax_average.sh
  jobs/detectors/cube/launch_calibration_charge_average_water.sh
  jobs/detectors/cube/launch_calibration_charge_average_air.sh
  jobs/detectors/cube/run_calibration_charge_average.sh
  jobs/detectors/BB7/launch_gantry_angle_sweep.sh
  jobs/detectors/BB7/run_gantry_angle_sweep.sh
  jobs/detectors/BB7/launch_collimator_angle_sweep.sh
  jobs/detectors/BB7/run_collimator_angle_sweep.sh
  jobs/detectors/BB7/launch_field_size_sweep.sh
  jobs/detectors/BB7/run_field_size_sweep.sh
  jobs/detectors/BB7/launch_depth_calibration_sweep.sh
  jobs/detectors/BB7/launch_octant_gantry_calibration_sweep.sh
  jobs/detectors/BB7/run_octant_gantry_calibration_sweep.sh
  jobs/detectors/BB7/launch_fine_gantry_calibration_sweep.sh
  jobs/detectors/BB7/run_fine_gantry_calibration_sweep.sh
)

set(MDSIM1_DATA_FILES
  beam/CyberKnife_IRIS_5mm.IAEAheader
  beam/CyberKnife_IRIS_5mm.IAEAphsp
  beam/Varian_TrueBeam6MV_01.IAEAheader
  beam/Varian_TrueBeam6MV_01.IAEAphsp
  beam/Varian_TrueBeam6MV_02.IAEAheader
  beam/Varian_TrueBeam6MV_02.IAEAphsp
  beam/Varian_TrueBeam6MV_03.IAEAheader
  beam/Varian_TrueBeam6MV_03.IAEAphsp
  beam/Varian_TrueBeam6MV_04.IAEAheader
  beam/Varian_TrueBeam6MV_04.IAEAphsp
  beam/Varian_TrueBeam6MV_05.IAEAheader
  beam/Varian_TrueBeam6MV_05.IAEAphsp
  beam/Varian_TrueBeam6MV_06.IAEAheader
  beam/Varian_TrueBeam6MV_06.IAEAphsp
  jobs/detectors/BB7/references/fine_gantry_calibration_reference.xlsx
)

foreach(_script ${MDSIM1_RUNTIME_FILES})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

if(MDSIM1_COPY_RUNTIME_DATA)
  foreach(_data_file ${MDSIM1_DATA_FILES})
    configure_file(
      ${PROJECT_SOURCE_DIR}/${_data_file}
      ${PROJECT_BINARY_DIR}/${_data_file}
      COPYONLY
    )
  endforeach()
endif()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS MultiDetector1 DESTINATION bin)

add_test(
  NAME mdsim1_invalid_eof_policy
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/invalid_eof_policy.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_invalid_eof_policy.cmake
)

add_test(
  NAME mdsim1_zero_events_skip_phsp_open
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=input/detectors/cube/Cube.in
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_zero_event_phsp_lazy.cmake
)

add_test(
  NAME mdsim1_incomplete_phsp_dataset_fails_early
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_incomplete_phsp_dataset.cmake
)

add_test(
  NAME mdsim1_idle_detector_refresh
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_detector_refresh.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_detector_refresh.cmake
)

add_test(
  NAME mdsim1_idle_detector_remove
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_detector_remove.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_detector_remove.cmake
)

add_test(
  NAME mdsim1_idle_detector_remove_mt
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_detector_remove.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_detector_remove_mt.cmake
)

set_tests_properties(
  mdsim1_invalid_eof_policy
  mdsim1_zero_events_skip_phsp_open
  mdsim1_incomplete_phsp_dataset_fails_early
  mdsim1_idle_detector_refresh
  mdsim1_idle_detector_remove
  mdsim1_idle_detector_remove_mt
  PROPERTIES
    TIMEOUT 180
)
