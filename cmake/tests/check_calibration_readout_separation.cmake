if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(digit_headers
  "include/geometry/detectors/basic/cube/readout/CubeDigit.hh"
  "include/geometry/detectors/basic/cylinder/readout/CylinderDigit.hh"
  "include/geometry/detectors/basic/sphere/readout/SphereDigit.hh"
  "include/geometry/detectors/model11/readout/Model11Digit.hh"
  "include/geometry/detectors/BB7/readout/BB7Digit.hh"
)

foreach(digit_header IN LISTS digit_headers)
  file(READ "${SOURCE_DIR}/${digit_header}" file_content)
  if(file_content MATCHES "EstimatedDoseToWater")
    message(FATAL_ERROR
      "Legacy digit interface still contains calibrated-dose fields in ${digit_header}")
  endif()
endforeach()

set(readout_parameter_headers
  "include/geometry/detectors/basic/cube/readout/CubeReadoutParameters.hh"
  "include/geometry/detectors/basic/cylinder/readout/CylinderReadoutParameters.hh"
  "include/geometry/detectors/basic/sphere/readout/SphereReadoutParameters.hh"
  "include/geometry/detectors/model11/readout/Model11ReadoutParameters.hh"
  "include/geometry/detectors/BB7/readout/BB7ReadoutParameters.hh"
)

foreach(parameter_header IN LISTS readout_parameter_headers)
  file(READ "${SOURCE_DIR}/${parameter_header}" file_content)
  if(file_content MATCHES "calibrationFactor")
    message(FATAL_ERROR
      "Readout parameters still contain calibration fields in ${parameter_header}")
  endif()
  if(parameter_header MATCHES "basic/.*/readout/" AND
     NOT file_content MATCHES "BasicDosimeterReadoutParameters")
    message(FATAL_ERROR
      "Expected shared basic/core readout parameters in ${parameter_header}")
  endif()
endforeach()

set(shared_readout_sources
  "src/geometry/detectors/basic/cube/readout/CubeReadoutModel.cc"
  "src/geometry/detectors/basic/cylinder/readout/CylinderReadoutModel.cc"
  "src/geometry/detectors/basic/sphere/readout/SphereReadoutModel.cc"
)

foreach(readout_source IN LISTS shared_readout_sources)
  file(READ "${SOURCE_DIR}/${readout_source}" file_content)
  if(NOT file_content MATCHES "BuildBasicDosimeterReadoutParameters")
    message(FATAL_ERROR
      "Expected shared basic/core readout builder usage in ${readout_source}")
  endif()
endforeach()

set(shared_digitizer_sources
  "src/geometry/detectors/basic/cube/readout/CubeDigitizer.cc"
  "src/geometry/detectors/basic/cylinder/readout/CylinderDigitizer.cc"
  "src/geometry/detectors/basic/sphere/readout/SphereDigitizer.cc"
)

foreach(digitizer_source IN LISTS shared_digitizer_sources)
  file(READ "${SOURCE_DIR}/${digitizer_source}" file_content)
  if(NOT file_content MATCHES "ComputeBasicDosimeterCollectedCharge")
    message(FATAL_ERROR
      "Expected shared collected-charge helper usage in ${digitizer_source}")
  endif()
  if(NOT file_content MATCHES "GetBasicDosimeterSensitiveVolumeMassOrThrow")
    message(FATAL_ERROR
      "Expected shared mass helper usage in ${digitizer_source}")
  endif()
endforeach()

set(shared_calibrator_sources
  "src/geometry/detectors/basic/cube/calibration/CubeDoseCalibrator.cc"
  "src/geometry/detectors/basic/cylinder/calibration/CylinderDoseCalibrator.cc"
  "src/geometry/detectors/basic/sphere/calibration/SphereDoseCalibrator.cc"
)

foreach(calibrator_source IN LISTS shared_calibrator_sources)
  file(READ "${SOURCE_DIR}/${calibrator_source}" file_content)
  if(NOT file_content MATCHES "CalibrateBasicDosimeterCollectedCharge")
    message(FATAL_ERROR
      "Expected shared calibration helper usage in ${calibrator_source}")
  endif()
endforeach()

set(module_sources
  "src/geometry/detectors/basic/cube/CubeDetectorModule.cc"
  "src/geometry/detectors/basic/cylinder/CylinderDetectorModule.cc"
  "src/geometry/detectors/basic/sphere/SphereDetectorModule.cc"
  "src/geometry/detectors/model11/Model11DetectorModule.cc"
  "src/geometry/detectors/BB7/BB7DetectorModule.cc"
)

foreach(module_source IN LISTS module_sources)
  file(READ "${SOURCE_DIR}/${module_source}" file_content)
  if(NOT file_content MATCHES "DoseCalibration")
    message(FATAL_ERROR
      "Expected a dedicated calibration ntuple in ${module_source}")
  endif()
endforeach()

file(READ "${SOURCE_DIR}/src/geometry/detectors/BB7/messenger/DetectorDualBB7Messenger.cc" bb7_messenger)
if(NOT bb7_messenger MATCHES "/MultiDetector1/detectors/BB7/setCalibrationFactor")
  message(FATAL_ERROR "BB7 messenger is missing setCalibrationFactor command")
endif()
if(NOT bb7_messenger MATCHES "/MultiDetector1/detectors/BB7/setCalibrationFactorError")
  message(FATAL_ERROR "BB7 messenger is missing setCalibrationFactorError command")
endif()

message(STATUS "Calibration/readout separation checks passed")
