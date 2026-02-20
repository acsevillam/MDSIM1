#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(MultiDetector1)

# ROOT
# include(FindROOT.cmake)

# set(INCLUDE_DIRECTORIES
# ${ROOT_INCLUDE_DIR} 
# )
# include_directories( ${INCLUDE_DIRECTORIES})
# set(LINK_DIRECTORIES
# ${ROOT_LIBRARY_DIR}
# )
# link_directories( ${LINK_DIRECTORIES})

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Get examples sources from common and shared
#
#list(APPEND sources 
#  ${PROJECT_SOURCE_DIR}/../SciFW1/src/geometry/AcceleratorClinac2100.cc)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(MultiDetector1 MultiDetector1.cc ${sources} ${headers})
target_link_libraries(MultiDetector1 ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${G4IAEAphsp_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build MultiDetector1. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(MultiDetector1_SCRIPTS
  mac/vis.mac
  mac/init.mac
  mac/scorers1.mac
  analysis/README.md
  beam/photon6MV.txt
  beam/Co-60.txt
  beam/circularplane.txt
  beam/squareplane.txt
  beam/CyberKnife_IRIS_5mm.IAEAheader
  beam/CyberKnife_IRIS_5mm.IAEAphsp
  beam/Varian_TrueBeam6MV_01.IAEAheader
  beam/Varian_TrueBeam6MV_01.IAEAphsp
  input/Calibration.in
  input/Calibration.intmp
  input/Validation.in
  input/Validation.intmp
  input/BB7.in
  input/BB7Calibration.in
  input/BB7Calibration.intmp
  #input/DetRotX0deg/BB7Calibration0.in
  #input/DetRotX0deg/BB7Calibration2.in
  #input/DetRotX0deg/BB7Calibration4.in
  #input/DetRotX0deg/BB7Calibration6.in
  #input/DetRotX0deg/BB7Calibration8.in
  #input/DetRotX0deg/BB7Calibration10.in
  #input/DetRotX0deg/BB7Calibration15.in
  #input/DetRotX0deg/BB7Calibration20.in
  #input/DetRotX0deg/BB7Calibration25.in
  #input/DetRotX0deg/BB7Calibration30.in
  #input/DetRotX0deg/BB7Calibration35.in
  input/DoseMap3D.in
  input/benchmark/set1.in
  input/benchmark/set2.in
  input/benchmark/set3.in
  jobs/job_launcher1.sh
  jobs/job1.sh
  jobs/job_launcher2a.sh
  jobs/job2a.sh
  jobs/job_launcher2b.sh
  jobs/job2b.sh
  jobs/job_launcher2c.sh
  jobs/job2c.sh
  jobs/job_launcher3.sh
  jobs/job_launcher4a.sh
  jobs/job4a.sh
  jobs/job_launcher4b.sh
  jobs/job4b.sh
  )

foreach(_script ${MultiDetector1_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
#add_custom_target(MultiDetector1 DEPENDS MultiDetector1)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS MultiDetector1 DESTINATION bin)


