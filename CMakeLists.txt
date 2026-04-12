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
  find_package(Geant4 REQUIRED COMPONENTS gdml ui_all vis_all)
else()
  find_package(Geant4 REQUIRED COMPONENTS gdml)
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

add_executable(
  md1_gdml_cache_probe
  cmake/tests/GDMLAssemblyCacheProbe.cc
  src/geometry/gdml/MD1GDMLReadStructure.cc
  src/geometry/gdml/GDMLAssemblyReader.cc
  src/geometry/gdml/GDMLAssemblyCache.cc
)
target_include_directories(md1_gdml_cache_probe PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(md1_gdml_cache_probe PRIVATE ${Geant4_LIBRARIES})

add_executable(
  md1_gdml_export_probe
  cmake/tests/GDMLExportProbe.cc
)
target_include_directories(md1_gdml_export_probe PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(md1_gdml_export_probe PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory
#
set(MDSIM1_RUNTIME_FILES
  mac/vis.mac
  mac/init.mac
  mac/scorers1.mac
  mac/tests/invalid_eof_policy.mac
  mac/tests/invalid_primary_generator_type.mac
  mac/tests/idle_detector_refresh.mac
  mac/tests/idle_detector_remove.mac
  mac/tests/idle_cube_split_rebuild.mac
  mac/tests/idle_cylinder_split_rebuild.mac
  mac/tests/idle_sphere_split_rebuild.mac
  mac/tests/cube_signal_smoke.mac
  mac/tests/cylinder_signal_smoke.mac
  mac/tests/sphere_signal_smoke.mac
  mac/tests/scintcube_signal_smoke.mac
  mac/tests/model11_signal_smoke.mac
  mac/tests/model11_imported_geometry_smoke.mac
  mac/tests/model11_imported_geometry_passive_only_smoke.mac
  mac/tests/model11_imported_geometry_aux_sensdet.mac
  mac/tests/model11_imported_geometry_validate_smoke.mac
  mac/tests/model11_imported_geometry_validate_invalid.mac
  mac/tests/model11_imported_geometry_missing_gdml.mac
  mac/tests/model11_imported_geometry_split_unsupported.mac
  mac/tests/geometry_export_smoke.mac
  mac/tests/bb7_signal_smoke.mac
  analysis/README.md
  beam/photon6MV.txt
  beam/Co-60.txt
  beam/circularplane.txt
  beam/squareplane.txt
  src/geometry/detectors/basic/cube/geometry/CubeCalibrationTable.dat
  src/geometry/detectors/basic/cylinder/geometry/CylinderCalibrationTable.dat
  src/geometry/detectors/basic/sphere/geometry/SphereCalibrationTable.dat
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
  input/detectors/cylinder/Cylinder.in
  input/detectors/cylinder/CylinderCalibration.in
  input/detectors/cylinder/templates/Cylinder.intmp
  input/detectors/cylinder/templates/CylinderCalibration.intmp
  input/detectors/scintCube/ScintCube.in
  input/detectors/scintCube/templates/ScintCube.intmp
  input/detectors/model11/Model11.in
  input/detectors/model11/templates/Model11.intmp
  models/detectors/model11/gdml/model11.gdml
  cmake/tests/data/model11/imported_geometry.gdml
  cmake/tests/data/model11/imported_geometry_aux.gdml
  cmake/tests/data/model11/imported_geometry_validate.gdml
  cmake/tests/data/model11/imported_geometry_invalid_schema.gdml
  cmake/tests/data/model11/imported_geometry_reloaded.gdml
  input/detectors/sphere/Sphere.in
  input/detectors/sphere/SphereCalibration.in
  input/detectors/sphere/templates/Sphere.intmp
  input/detectors/sphere/templates/SphereCalibration.intmp
  input/dosimetry/DoseMap3D.in
  input/benchmark/set1.in
  input/benchmark/set2.in
  input/benchmark/set3.in
  jobs/lib/md1_job_helpers.sh
  jobs/dosimetry/launch_validation.sh
  jobs/dosimetry/run_validation.sh
  jobs/dosimetry/launch_calibration_dmax.sh
  jobs/dosimetry/run_calibration_dmax.sh
  jobs/detectors/cube/launch_calibration_water.sh
  jobs/detectors/cube/launch_calibration_air.sh
  jobs/detectors/cube/launch_calibration_water_pmma_2p5mm.sh
  jobs/detectors/cube/launch_calibration_air_pmma_2p5mm.sh
  jobs/detectors/cube/run_calibration.sh
  jobs/detectors/cube/launch_pdd_interface_water.sh
  jobs/detectors/cube/launch_pdd_interface_air.sh
  jobs/detectors/cube/launch_pdd_interface_water_pmma_2p5mm.sh
  jobs/detectors/cube/launch_pdd_interface_air_pmma_2p5mm.sh
  jobs/detectors/cube/run_pdd_interface.sh
  jobs/detectors/cylinder/launch_calibration_water.sh
  jobs/detectors/cylinder/launch_calibration_air.sh
  jobs/detectors/cylinder/launch_calibration_water_pmma_2p5mm.sh
  jobs/detectors/cylinder/launch_calibration_air_pmma_2p5mm.sh
  jobs/detectors/cylinder/run_calibration.sh
  jobs/detectors/cylinder/launch_pdd_interface_water.sh
  jobs/detectors/cylinder/launch_pdd_interface_air.sh
  jobs/detectors/cylinder/launch_pdd_interface_water_pmma_2p5mm.sh
  jobs/detectors/cylinder/launch_pdd_interface_air_pmma_2p5mm.sh
  jobs/detectors/cylinder/run_pdd_interface.sh
  jobs/detectors/sphere/launch_calibration_water.sh
  jobs/detectors/sphere/launch_calibration_air.sh
  jobs/detectors/sphere/launch_calibration_water_pmma_2p5mm.sh
  jobs/detectors/sphere/launch_calibration_air_pmma_2p5mm.sh
  jobs/detectors/sphere/run_calibration.sh
  jobs/detectors/sphere/launch_pdd_interface_water.sh
  jobs/detectors/sphere/launch_pdd_interface_air.sh
  jobs/detectors/sphere/launch_pdd_interface_water_pmma_2p5mm.sh
  jobs/detectors/sphere/launch_pdd_interface_air_pmma_2p5mm.sh
  jobs/detectors/sphere/run_pdd_interface.sh
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
  NAME mdsim1_invalid_primary_generator_type
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/invalid_primary_generator_type.mac
          -DEXPECTED_REGEX=InvalidPrimaryGeneratorType
          -DFAILURE_LABEL=invalid_primary_generator_type
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_expected_failure.cmake
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

add_test(
  NAME mdsim1_idle_cube_split_rebuild_mt
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_cube_split_rebuild.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_cube_split_rebuild_mt.cmake
)

add_test(
  NAME mdsim1_idle_cylinder_split_rebuild_mt
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_cylinder_split_rebuild.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_cylinder_split_rebuild_mt.cmake
)

add_test(
  NAME mdsim1_idle_sphere_split_rebuild_mt
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/idle_sphere_split_rebuild.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_idle_sphere_split_rebuild_mt.cmake
)

add_test(
  NAME mdsim1_cube_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/cube_signal_smoke.mac
          -DDETECTOR_SUMMARY_LABEL=cube\\[0\\]
          -DDETECTOR_RESULTS_TITLE=Cube
          -DDETECTOR_NAME=cube
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_detector_signal_smoke.cmake
)

add_test(
  NAME mdsim1_cylinder_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/cylinder_signal_smoke.mac
          -DDETECTOR_SUMMARY_LABEL=cylinder\\[0\\]
          -DDETECTOR_RESULTS_TITLE=Cylinder
          -DDETECTOR_NAME=cylinder
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_detector_signal_smoke.cmake
)

add_test(
  NAME mdsim1_sphere_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/sphere_signal_smoke.mac
          -DDETECTOR_SUMMARY_LABEL=sphere\\[0\\]
          -DDETECTOR_RESULTS_TITLE=Sphere
          -DDETECTOR_NAME=sphere
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_detector_signal_smoke.cmake
)

add_test(
  NAME mdsim1_scintcube_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/scintcube_signal_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_scintcube_signal_smoke.cmake
)

add_test(
  NAME mdsim1_model11_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_signal_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_signal_smoke.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_imported_geometry_smoke.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_passive_only_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_passive_only_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_imported_geometry_passive_only_smoke.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_aux_sensdet
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_aux_sensdet.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_imported_geometry_aux_sensdet.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_validate_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_validate_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_imported_geometry_passive_only_smoke.cmake
)

add_test(
  NAME mdsim1_bb7_signal_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/bb7_signal_smoke.mac
          -DDETECTOR_SUMMARY_LABEL=BB7\\[0\\]
          -DDETECTOR_RESULTS_TITLE=BB7
          -DDETECTOR_NAME=BB7
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_detector_signal_smoke.cmake
)

add_test(
  NAME mdsim1_calibration_readout_separation
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_calibration_readout_separation.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_missing_gdml
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_missing_gdml.mac
          -DEXPECTED_REGEX=ImportedGDMLFileNotFound
          -DFAILURE_LABEL=model11_missing_gdml
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_expected_failure.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_split_unsupported
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_split_unsupported.mac
          -DEXPECTED_REGEX=DetectorModel11ImportedGeometrySplitUnsupported
          -DFAILURE_LABEL=model11_split_with_gdml
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_expected_failure.cmake
)

add_test(
  NAME mdsim1_model11_imported_geometry_validate_invalid
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/model11_imported_geometry_validate_invalid.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_model11_imported_geometry_validate_invalid.cmake
)

add_test(
  NAME mdsim1_gdml_import_cleanup
  COMMAND ${CMAKE_COMMAND}
          -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_gdml_import_cleanup.cmake
)

add_test(
  NAME mdsim1_gdml_cache_reload
  COMMAND ${CMAKE_COMMAND}
          -DPROBE_EXECUTABLE=$<TARGET_FILE:md1_gdml_cache_probe>
          -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_gdml_cache_reload.cmake
)

add_test(
  NAME mdsim1_geometry_export_smoke
  COMMAND ${CMAKE_COMMAND}
          -DEXECUTABLE=$<TARGET_FILE:MultiDetector1>
          -DPROBE_EXECUTABLE=$<TARGET_FILE:md1_gdml_export_probe>
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -DMACRO_PATH=mac/tests/geometry_export_smoke.mac
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_geometry_export_smoke.cmake
)

add_test(
  NAME mdsim1_validation_summary_axes
  COMMAND ${CMAKE_COMMAND}
          -DWORKING_DIRECTORY=${PROJECT_BINARY_DIR}
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/check_validation_summary_axes.cmake
)

set_tests_properties(
  mdsim1_invalid_eof_policy
  mdsim1_invalid_primary_generator_type
  mdsim1_zero_events_skip_phsp_open
  mdsim1_incomplete_phsp_dataset_fails_early
  mdsim1_idle_detector_refresh
  mdsim1_idle_detector_remove
  mdsim1_idle_detector_remove_mt
  mdsim1_idle_cube_split_rebuild_mt
  mdsim1_idle_cylinder_split_rebuild_mt
  mdsim1_idle_sphere_split_rebuild_mt
  mdsim1_cube_signal_smoke
  mdsim1_cylinder_signal_smoke
  mdsim1_sphere_signal_smoke
  mdsim1_scintcube_signal_smoke
  mdsim1_model11_signal_smoke
  mdsim1_model11_imported_geometry_smoke
  mdsim1_model11_imported_geometry_passive_only_smoke
  mdsim1_model11_imported_geometry_missing_gdml
  mdsim1_model11_imported_geometry_split_unsupported
  mdsim1_bb7_signal_smoke
  mdsim1_gdml_cache_reload
  mdsim1_gdml_import_cleanup
  mdsim1_validation_summary_axes
  PROPERTIES
    TIMEOUT 180
)

set_tests_properties(
  mdsim1_invalid_eof_policy
  mdsim1_invalid_primary_generator_type
  mdsim1_zero_events_skip_phsp_open
  mdsim1_incomplete_phsp_dataset_fails_early
  PROPERTIES
    LABELS "phsp_quick;quick"
)

set_tests_properties(
  mdsim1_idle_detector_refresh
  mdsim1_idle_detector_remove
  mdsim1_idle_detector_remove_mt
  mdsim1_idle_cube_split_rebuild_mt
  mdsim1_idle_cylinder_split_rebuild_mt
  mdsim1_idle_sphere_split_rebuild_mt
  mdsim1_model11_imported_geometry_missing_gdml
  mdsim1_model11_imported_geometry_split_unsupported
  mdsim1_gdml_cache_reload
  mdsim1_gdml_import_cleanup
  mdsim1_validation_summary_axes
  PROPERTIES
    LABELS "detectors_quick;geometry_quick;quick"
)

set_tests_properties(
  mdsim1_cube_signal_smoke
  mdsim1_cylinder_signal_smoke
  mdsim1_sphere_signal_smoke
  mdsim1_scintcube_signal_smoke
  mdsim1_model11_signal_smoke
  mdsim1_model11_imported_geometry_smoke
  mdsim1_model11_imported_geometry_passive_only_smoke
  mdsim1_bb7_signal_smoke
  PROPERTIES
    LABELS "detectors_quick;signal_quick;quick"
)

add_custom_target(
  detector-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L detectors_quick
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running quick detector-focused CTest suite"
)

add_custom_target(
  geometry-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L geometry_quick
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running quick geometry-focused CTest suite"
)

add_custom_target(
  signal-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L signal_quick
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running quick detector signal CTest suite"
)

add_custom_target(
  phsp-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L phsp_quick
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running quick PHSP-focused CTest suite"
)

add_custom_target(
  quick-tests
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L quick
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running full quick CTest suite"
)

add_custom_target(
  nightly-tests
  COMMAND ${CMAKE_COMMAND}
          -DCTEST_COMMAND=${CMAKE_CTEST_COMMAND}
          -DBUILD_DIR=${PROJECT_BINARY_DIR}
          -DLABEL=nightly
          -DDESCRIPTION=nightly_or_heavy_suite
          -P ${PROJECT_SOURCE_DIR}/cmake/tests/run_labeled_ctest.cmake
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running nightly/heavy CTest suite if present"
)
