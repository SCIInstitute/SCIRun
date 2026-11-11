#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2026 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

# EXTERNALS_FROM=<superbuild bin dir>: build only the inner SCIRun project,
# linking against the externals another, already built Superbuild produced.
#
# Paths are read from that build's inner SCIRun/CMakeCache.txt, which exists
# only once every external is built (SCIRun_external DEPENDS on all of them)
# and is one file per build dir under every generator.

# The externals build records a hash of each Superbuild/ file that decides how
# externals are built (ExternalsManifest.txt, written once they all finish);
# a build whose Superbuild/ hashes differently is refused. These files are
# left out: they hold options and inner-build plumbing, not external recipes.
SET(_ef_untracked Superbuild.cmake CMakeLists.txt ExternalsFrom.cmake
  DeprecatedFlags.cmake CMakePresets.json CMakeUserPresets.json
  CMakeUserPresets.json.example scripts/check_dependency_versions.py)

FUNCTION(scirun_externals_manifest superbuild_dir out_var)
  FILE(GLOB_RECURSE _files RELATIVE "${superbuild_dir}" "${superbuild_dir}/*")
  LIST(SORT _files)
  SET(_m "")
  FOREACH(_f ${_files})
    IF(_f IN_LIST _ef_untracked OR _f MATCHES "^deprecated/|(^|/)\\.|__pycache__")
      CONTINUE()
    ENDIF()
    FILE(SHA256 "${superbuild_dir}/${_f}" _h)
    STRING(APPEND _m "${_h}  ${_f}\n")
  ENDFOREACH()
  SET(${out_var} "${_m}" PARENT_SCOPE)
ENDFUNCTION()

# Normal (non-EXTERNALS_FROM) builds: publish the manifest only after every
# external has built, via a SCIRun_external step ahead of its configure.
MACRO(scirun_externals_record)
  scirun_externals_manifest("${SUPERBUILD_DIR}" _ef_m)
  FILE(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/ExternalsManifest.pending.txt" CONTENT "${_ef_m}" @ONLY)
  ExternalProject_Add_Step(SCIRun_external record_externals
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_BINARY_DIR}/ExternalsManifest.pending.txt"
      "${CMAKE_BINARY_DIR}/ExternalsManifest.txt"
    DEPENDS "${CMAKE_BINARY_DIR}/ExternalsManifest.pending.txt"
    DEPENDEES patch
    DEPENDERS configure)
ENDMACRO()

# Options that change which externals exist or how they were compiled.
SET(_ef_config_vars
  WITH_PYTHON WITH_TETGEN WITH_OSPRAY PREBUILT_OSPRAY WITH_VTK PREBUILT_VTK
  WITH_GUI Qt_PATH SCIRUN_QT_MIN_VERSION)

# Called before any OPTION(): adopt the other build's settings as defaults,
# so -DEXTERNALS_FROM=<dir> alone gives a compatible configuration.
MACRO(scirun_externals_from_seed)
  GET_FILENAME_COMPONENT(EXTERNALS_FROM "${EXTERNALS_FROM}" ABSOLUTE)
  SET(EXTERNALS_FROM "${EXTERNALS_FROM}" CACHE PATH "" FORCE)
  GET_FILENAME_COMPONENT(_ef_self "${CMAKE_BINARY_DIR}" REALPATH)
  GET_FILENAME_COMPONENT(_ef_other "${EXTERNALS_FROM}" REALPATH)
  IF(_ef_self STREQUAL _ef_other)
    MESSAGE(FATAL_ERROR "EXTERNALS_FROM points at this build directory; give it another Superbuild's.")
  ENDIF()
  IF(NOT EXISTS "${EXTERNALS_FROM}/CMakeCache.txt")
    MESSAGE(FATAL_ERROR "EXTERNALS_FROM=${EXTERNALS_FROM} has no CMakeCache.txt; "
      "point it at a Superbuild binary dir (the one holding Externals/ and SCIRun/).")
  ENDIF()
  LOAD_CACHE("${EXTERNALS_FROM}" READ_WITH_PREFIX _ef_
    CMAKE_PROJECT_NAME CMAKE_HOME_DIRECTORY CMAKE_BUILD_TYPE CMAKE_CXX_COMPILER CMAKE_GENERATOR
    CMAKE_OSX_DEPLOYMENT_TARGET ${_ef_config_vars})
  IF(NOT _ef_CMAKE_PROJECT_NAME STREQUAL "Superbuild")
    MESSAGE(FATAL_ERROR "EXTERNALS_FROM=${EXTERNALS_FROM} is a '${_ef_CMAKE_PROJECT_NAME}' build, "
      "not a SCIRun Superbuild. Point it at the outer build dir, not its SCIRun/ subdir.")
  ENDIF()
  IF(NOT EXISTS "${EXTERNALS_FROM}/SCIRun/CMakeCache.txt")
    MESSAGE(FATAL_ERROR "EXTERNALS_FROM=${EXTERNALS_FROM} has not finished building its externals "
      "(no SCIRun/CMakeCache.txt yet). Build that Superbuild first.")
  ENDIF()

  FOREACH(_v ${_ef_config_vars})
    IF(DEFINED _ef_${_v} AND NOT DEFINED CACHE{${_v}})
      IF(_v MATCHES "^(WITH|PREBUILT)_")
        SET(_t BOOL)
      ELSEIF(_v STREQUAL "Qt_PATH")
        SET(_t PATH)
      ELSE()
        SET(_t STRING)
      ENDIF()
      SET(${_v} "${_ef_${_v}}" CACHE ${_t} "Taken from EXTERNALS_FROM")
    ENDIF()
  ENDFOREACH()
  # Toolkits are example data, not build inputs; the shared build has a copy.
  IF(NOT DEFINED CACHE{DOWNLOAD_TOOLKITS})
    SET(DOWNLOAD_TOOLKITS OFF CACHE BOOL "Download toolkit repositories.")
  ENDIF()
  # PROJECT() has already cached an empty CMAKE_BUILD_TYPE.
  IF(NOT CMAKE_BUILD_TYPE AND _ef_CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "${_ef_CMAKE_BUILD_TYPE}" CACHE STRING "Taken from EXTERNALS_FROM" FORCE)
  ENDIF()
ENDMACRO()

# Called where the externals would be added: refuse a mismatched build, then
# set the variables the *External.cmake files would have set.
MACRO(scirun_externals_from_import)
  SET(_ef_bad)
  FOREACH(_v ${_ef_config_vars} CMAKE_OSX_DEPLOYMENT_TARGET)
    # Compare as booleans where both sides are, so ON matches TRUE.
    SET(_mine "${${_v}}")
    SET(_theirs "${_ef_${_v}}")
    IF(_mine MATCHES "^(ON|OFF|TRUE|FALSE|YES|NO|0|1)?$" AND _theirs MATCHES "^(ON|OFF|TRUE|FALSE|YES|NO|0|1)?$")
      IF((_mine AND NOT _theirs) OR (_theirs AND NOT _mine))
        LIST(APPEND _ef_bad "${_v}: this build '${_mine}', externals '${_theirs}'")
      ENDIF()
    ELSEIF(NOT _mine STREQUAL _theirs)
      LIST(APPEND _ef_bad "${_v}: this build '${_mine}', externals '${_theirs}'")
    ENDIF()
  ENDFOREACH()
  # Empty means a multi-config generator picks it at build time.
  IF(CMAKE_BUILD_TYPE AND _ef_CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL _ef_CMAKE_BUILD_TYPE)
    LIST(APPEND _ef_bad "CMAKE_BUILD_TYPE: this build '${CMAKE_BUILD_TYPE}', externals '${_ef_CMAKE_BUILD_TYPE}'")
  ENDIF()
  # A compiler update or another single-config generator is usually harmless,
  # so these only warn.
  FOREACH(_v CMAKE_CXX_COMPILER CMAKE_GENERATOR)
    IF(NOT ${_v} STREQUAL _ef_${_v})
      MESSAGE(WARNING "EXTERNALS_FROM: ${_v} differs (this build '${${_v}}', "
        "externals '${_ef_${_v}}'). Link or runtime errors are likely if the ABI changed.")
    ENDIF()
  ENDFOREACH()
  IF(_ef_bad)
    LIST(JOIN _ef_bad "\n  " _ef_bad)
    MESSAGE(FATAL_ERROR "EXTERNALS_FROM=${EXTERNALS_FROM} was configured differently:\n  ${_ef_bad}\n"
      "Drop the conflicting -D options (they default to the externals' values) "
      "or use a matching Superbuild.")
  ENDIF()

  scirun_externals_from_check_superbuild()

  # Inner-cache name -> Superbuild variable that SCIRUN_CACHE_ARGS reads.
  SET(_ef_map
    Eigen_DIR Eigen_DIR   Zlib_DIR Zlib_DIR   SQLite_DIR SQLite_DIR
    Boost_DIR Boost_DIR   Teem_DIR Teem_DIR   Freetype_DIR Freetype_DIR
    GLM_DIR GLM_DIR       SPDLOG_DIR SPDLOG_DIR   TNY_DIR TNY_DIR
    GLEW_DIR Glew_DIR     LODEPNG_DIR LODEPNG_DIR   CLEAVER2_DIR CLEAVER2_DIR
    LibXML2_DIR LibXML2_DIR
    Python_DIR Python_DIR   PYTHON_EXECUTABLE SCI_PYTHON_EXE
    USER_PYTHON_VERSION USER_PYTHON_VERSION
    USER_PYTHON_VERSION_MAJOR USER_PYTHON_VERSION_MAJOR
    USER_PYTHON_VERSION_MINOR USER_PYTHON_VERSION_MINOR
    Tetgen_DIR Tetgen_DIR
    Ospray_External_Dir OSPRAY_BUILD_DIR   VTK_External_Dir VTK_INSTALL_DIR
    QWT_INCLUDE QWT_INCLUDE   QWT_LIBRARY_DIR QWT_LIBRARY_DIR
    QWT_LIBRARY QWT_LIBRARY   QWT_INSTALL_DIR QWT_INSTALL_DIR
    SCIRUN_TEST_RESOURCE_DIR _ef_test_dir)
  SET(_ef_inner_names)
  SET(_i 0)
  LIST(LENGTH _ef_map _n)
  WHILE(_i LESS _n)
    LIST(GET _ef_map ${_i} _inner)
    LIST(APPEND _ef_inner_names ${_inner})
    MATH(EXPR _i "${_i} + 2")
  ENDWHILE()
  LOAD_CACHE("${EXTERNALS_FROM}/SCIRun" READ_WITH_PREFIX _efi_ ${_ef_inner_names})
  SET(_i 0)
  WHILE(_i LESS _n)
    LIST(GET _ef_map ${_i} _inner)
    MATH(EXPR _j "${_i} + 1")
    LIST(GET _ef_map ${_j} _outer)
    SET(${_outer} "${_efi_${_inner}}")
    MATH(EXPR _i "${_i} + 2")
  ENDWHILE()

  IF(BUILD_TESTING AND NOT SCIRUN_TEST_RESOURCE_DIR AND _ef_test_dir)
    SET(SCIRUN_TEST_RESOURCE_DIR "${_ef_test_dir}" CACHE PATH "Root for all github-based testing files" FORCE)
  ENDIF()
  MESSAGE(STATUS "Using externals from ${EXTERNALS_FROM}; not building any.")
ENDMACRO()

MACRO(scirun_externals_from_check_superbuild)
  SET(_ef_manifest_file "${EXTERNALS_FROM}/ExternalsManifest.txt")
  UNSET(_ef_theirs)
  IF(EXISTS "${_ef_manifest_file}")
    FILE(READ "${_ef_manifest_file}" _ef_theirs)
    # A refresh of the shared externals re-runs this check on the next build.
    SET_PROPERTY(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_ef_manifest_file}")
  ELSEIF(IS_DIRECTORY "${_ef_CMAKE_HOME_DIRECTORY}")
    MESSAGE(WARNING "EXTERNALS_FROM predates ExternalsManifest.txt; comparing with the current "
      "Superbuild/ of its checkout (${_ef_CMAKE_HOME_DIRECTORY}), which may have moved on since "
      "the externals were built. Rebuild it once to record a manifest.")
    scirun_externals_manifest("${_ef_CMAKE_HOME_DIRECTORY}" _ef_theirs)
  ELSE()
    MESSAGE(WARNING "EXTERNALS_FROM has no ExternalsManifest.txt and its source checkout is gone; "
      "cannot tell whether this Superbuild/ matches the externals.")
  ENDIF()

  IF(DEFINED _ef_theirs)
    scirun_externals_manifest("${SUPERBUILD_DIR}" _ef_mine)
    IF(NOT _ef_mine STREQUAL _ef_theirs)
      STRING(REPLACE "\n" ";" _ef_mine_l "${_ef_mine}")
      STRING(REPLACE "\n" ";" _ef_theirs_l "${_ef_theirs}")
      SET(_ef_diff)
      FOREACH(_l IN LISTS _ef_mine_l _ef_theirs_l)
        IF(_l AND NOT (_l IN_LIST _ef_mine_l AND _l IN_LIST _ef_theirs_l))
          STRING(REGEX REPLACE "^[0-9a-f]+  " "" _p "${_l}")
          LIST(APPEND _ef_diff "${_p}")
        ENDIF()
      ENDFOREACH()
      LIST(REMOVE_DUPLICATES _ef_diff)
      LIST(JOIN _ef_diff "\n  " _ef_diff)
      SET(_ef_msg "This checkout's Superbuild/ differs from the one the externals in "
        "${EXTERNALS_FROM} were built from:\n  ${_ef_diff}\n")
      IF(EXTERNALS_FROM_IGNORE_CHANGES)
        MESSAGE(WARNING ${_ef_msg} "Continuing because EXTERNALS_FROM_IGNORE_CHANGES is ON.")
      ELSE()
        MESSAGE(FATAL_ERROR ${_ef_msg} "Do a full Superbuild, refresh the shared externals to match, "
          "or set EXTERNALS_FROM_IGNORE_CHANGES=ON if the change cannot affect the externals.")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO()
