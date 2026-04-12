if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

set(removed_paths
  "include/geometry/imported/ImportedGeometryAsset.hh"
  "include/geometry/imported/ImportedGeometryReader.hh"
  "src/geometry/imported/ImportedGeometryReader.cc"
  "tools/freecad/freecad_asset_exporter.py"
  "input/detectors/model11/assets/sample_imported_geometry/geometry.asset"
  "input/detectors/model11/assets/sample_imported_geometry/meshes/A1SLbodyforcnc2.stl"
  "input/detectors/model11/assets/sample_imported_geometry/meshes/A1SLcapforcnc2.stl"
)

foreach(removed_path IN LISTS removed_paths)
  if(EXISTS "${SOURCE_DIR}/${removed_path}")
    message(FATAL_ERROR "Legacy imported-geometry path still exists: ${removed_path}")
  endif()
endforeach()

set(removed_directories
  "src/geometry/imported"
  "input/detectors/model11/assets"
  "input/detectors/model11/assets/sample_imported_geometry"
  "input/detectors/model11/assets/sample_imported_geometry/meshes"
)

foreach(removed_directory IN LISTS removed_directories)
  if(IS_DIRECTORY "${SOURCE_DIR}/${removed_directory}")
    message(FATAL_ERROR "Legacy imported-geometry directory still exists: ${removed_directory}")
  endif()
endforeach()

set(text_files_to_check
  "README.md"
  "CMakeLists.txt"
  "src/geometry/detectors/model11/messenger/DetectorModel11Messenger.cc"
  "mac/tests/model11_imported_geometry_smoke.mac"
  "mac/tests/model11_imported_geometry_passive_only_smoke.mac"
  "mac/tests/model11_imported_geometry_missing_gdml.mac"
  "mac/tests/model11_imported_geometry_split_unsupported.mac"
  "input/detectors/model11/Model11.in"
  "input/detectors/model11/templates/Model11.intmp"
)

set(legacy_patterns
  "geometry\\.asset"
  "ImportedGeometryReader"
  "freecad_asset_exporter"
  "setImportedGeometryAsset"
  "setImportedGeometryDefaultMaterial"
  "setImportedGeometryEnabled"
  "setImportedGeometryMode"
  "sample_imported_geometry"
)

foreach(text_file IN LISTS text_files_to_check)
  file(READ "${SOURCE_DIR}/${text_file}" file_content)

  if(text_file STREQUAL "src/geometry/detectors/model11/messenger/DetectorModel11Messenger.cc")
    if(NOT file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryGDML")
      message(FATAL_ERROR "model11 messenger is missing setImportedGeometryGDML")
    endif()
    if(NOT file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryRootLogical")
      message(FATAL_ERROR "model11 messenger is missing setImportedGeometryRootLogical")
    endif()
    if(NOT file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryRootPhysical")
      message(FATAL_ERROR "model11 messenger is missing setImportedGeometryRootPhysical")
    endif()
    if(NOT file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryRootAssembly")
      message(FATAL_ERROR "model11 messenger is missing setImportedGeometryRootAssembly")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryRoot\", this")
      message(FATAL_ERROR "Legacy model11 setImportedGeometryRoot command is still present")
    endif()
    if(NOT file_content MATCHES "/MultiDetector1/detectors/model11/addSensitiveVolume")
      message(FATAL_ERROR "model11 messenger is missing addSensitiveVolume")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setDiameter")
      message(FATAL_ERROR "Legacy model11 setDiameter command is still present")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setHeight")
      message(FATAL_ERROR "Legacy model11 setHeight command is still present")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setMaterial")
      message(FATAL_ERROR "Legacy model11 setMaterial command is still present")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryEnabled")
      message(FATAL_ERROR "Legacy model11 setImportedGeometryEnabled command is still present")
    endif()
    if(file_content MATCHES "/MultiDetector1/detectors/model11/setImportedGeometryMode")
      message(FATAL_ERROR "Legacy model11 setImportedGeometryMode command is still present")
    endif()
  endif()

  foreach(legacy_pattern IN LISTS legacy_patterns)
    if(file_content MATCHES "${legacy_pattern}")
      message(FATAL_ERROR
        "Legacy imported-geometry reference '${legacy_pattern}' still appears in ${text_file}")
    endif()
  endforeach()
endforeach()

message(STATUS "GDML import cleanup checks passed")
