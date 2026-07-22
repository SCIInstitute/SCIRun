#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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
#


IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Utils)
  ADD_SUBDIRECTORY(ModuleTestBase)
ENDIF()

IF(BUILD_TESTING)

IF(WIN32)
 # TODO: determine if VS is set to Release or Debug.
 SET(EXE_LOC ${SCIRun_BINARY_DIR}/Release/SCIRun.exe CACHE PATH "Path to SCIRun executable")
ELSE(WIN32)
 # TODO: change once testing works with app bundle
 IF(APPLE)
   SET(EXE_LOC ${SCIRun_BINARY_DIR}/${TEST_APPLICATION_NAME} CACHE PATH "Path to SCIRun executable")
 ELSE()
   SET(EXE_LOC ${SCIRun_BINARY_DIR}/${APPLICATION_NAME} CACHE PATH "Path to SCIRun executable")
 ENDIF()
ENDIF()

# On Windows, ctest launches the unit-test executables and SCIRun.exe with the
# environment it inherited from the shell, which usually lacks the Qt, Python,
# and Qwt bin directories. Without them the processes fail immediately with
# 0xc0000135 (STATUS_DLL_NOT_FOUND), and Qt cannot find its platform plugin.
# Compute the environment once here and apply it to the unit, regression, and
# import tests below so all of them run from a plain command line / CI machine.
IF(WIN32)
  get_filename_component(_python_bin_dir "${PYTHON_EXECUTABLE}" DIRECTORY)
  # Resolve Qt bin and plugin dirs from whichever Qt version was found
  foreach(_qt_ver 6 5)
    if(Qt${_qt_ver}_DIR)
      get_filename_component(_qt_bin_dir "${Qt${_qt_ver}_DIR}/../../../bin" ABSOLUTE)
      get_filename_component(_qt_plugins_dir "${Qt${_qt_ver}_DIR}/../../../plugins" ABSOLUTE)
      break()
    endif()
  endforeach()
  # Qwt is built by the Superbuild; include both the install bin/lib and the
  # in-tree config subdirs so the qwt DLL is found regardless of layout.
  string(REPLACE ";" "\\;" _escaped_env_path "$ENV{PATH}")
  set(_test_path "${_python_bin_dir}\\;${_qt_bin_dir}\\;${QWT_INSTALL_DIR}/lib\\;${QWT_INSTALL_DIR}/bin\\;${Qwt_DIR}/src/Debug\\;${Qwt_DIR}/src/Release\\;${_escaped_env_path}")
  # DLL/plugin environment every ctest-launched process needs to start at all.
  set(SCIRUN_TEST_DLL_ENVIRONMENT
    "PATH=${_test_path}"
    "PYTHONHOME=${Python_DIR}"
    "QT_QPA_PLATFORM_PLUGIN_PATH=${_qt_plugins_dir}/platforms")
  # Same, minus PYTHONHOME, for the Python tests: those are run by a python
  # interpreter of their own, which would otherwise look for its stdlib under the
  # Superbuild prefix. They only need the paths so the SCIRun.exe they spawn
  # starts. Built from _test_path directly rather than by expanding the variable
  # above, since nesting these lists strips a level of semicolon escaping.
  set(SCIRUN_TEST_PATH_ENVIRONMENT
    "PATH=${_test_path}"
    "QT_QPA_PLATFORM_PLUGIN_PATH=${_qt_plugins_dir}/platforms")
  # Unit tests run headless and must never pop a window, so pin the offscreen
  # Qt platform. The regression/import tests launch the full SCIRun app; on a
  # developer workstation ViewScene networks need the real "windows" platform
  # to get a usable OpenGL context (the offscreen platform has none and those
  # networks fail). Leave QT_QPA_PLATFORM unset for those so the default
  # platform is used locally, while headless CI can still force it via the job
  # environment.
  # Spelled out from _test_path rather than expanding SCIRUN_TEST_DLL_ENVIRONMENT:
  # an unquoted expansion strips a level of escaping from the semicolons in PATH,
  # after which ctest splits PATH into separate ENVIRONMENT entries and the test
  # starts with a truncated PATH (0xc0000135 for anything needing Qt/Qwt DLLs).
  set(SCIRUN_TEST_ENVIRONMENT
    "PATH=${_test_path}"
    "PYTHONHOME=${Python_DIR}"
    "QT_QPA_PLATFORM_PLUGIN_PATH=${_qt_plugins_dir}/platforms"
    "QT_QPA_PLATFORM=offscreen")
ENDIF()

IF(RUN_UNIT_TESTS)

  IF(BUILD_WITH_PYTHON)
    EXECUTE_PROCESS(COMMAND "python" ${CMAKE_SOURCE_DIR}/Testing/Python/list_tests.py
      ${CMAKE_SOURCE_DIR}/Testing/Python/
      OUTPUT_VARIABLE STR_TESTS
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE)

    SEPARATE_ARGUMENTS(PYTHON_TEST_LIST UNIX_COMMAND ${STR_TESTS})

    IF(APPLE)
      SET(PYTHON_TEST_EXE ${SCIRun_BINARY_DIR}/../Externals/Install/Python_external/bin/python3)
    ELSE(APPLE)
      SET(PYTHON_TEST_EXE "python")
    ENDIF(APPLE)

    FOREACH(PYTHON_TEST ${PYTHON_TEST_LIST})
      #MESSAGE(${PYTHON_TEST})
      ADD_TEST(NAME ${PYTHON_TEST}
        COMMAND ${CMAKE_COMMAND} -E env "SCIRUN_EXECUTABLE_PATH_FOR_PYTHON_TESTS=${EXE_LOC}" "${PYTHON_TEST_EXE}" "-m" "unittest" "-v" ${PYTHON_TEST}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Testing/Python/)
      # These tests spawn SCIRun.exe as a subprocess, which needs the Qt/Python/Qwt
      # DLLs on PATH or it exits immediately with 0xc0000135 (STATUS_DLL_NOT_FOUND).
      IF(WIN32)
        SET_TESTS_PROPERTIES("${PYTHON_TEST}" PROPERTIES ENVIRONMENT "${SCIRUN_TEST_PATH_ENVIRONMENT}")
      ENDIF()
    ENDFOREACH(PYTHON_TEST)
  ENDIF()

  FOREACH(test ${UNIT_TEST_LIST})
    ADD_TEST("${test}" "../${EXE_DIRPATH}/${test}")
  ENDFOREACH(test ${UNIT_TEST_LIST})

  IF(WIN32 AND UNIT_TEST_LIST)
    FOREACH(test ${UNIT_TEST_LIST})
      # Quote the expansion: unquoted, CMake strips a level of escaping from the
      # semicolons in PATH, so ctest splits it into separate ENVIRONMENT entries
      # and the test starts with a truncated PATH (0xc0000135 for anything that
      # needs the Qt or Qwt DLLs).
      set_property(TEST "${test}" PROPERTY ENVIRONMENT "${SCIRUN_TEST_ENVIRONMENT}")
    ENDFOREACH()
  ENDIF()
ENDIF()

IF(RUN_BASIC_REGRESSION_TESTS)
  MESSAGE(STATUS "Using executable: ${EXE_LOC} to test networks")

  # Run all the possible .srn5 files in src/ExampleNets
  FILE(GLOB_RECURSE scirun_srns_modules "${SCIRun_SOURCE_DIR}/ExampleNets/regression/Modules/*.srn5")
  FILE(GLOB_RECURSE scirun_srns_network "${SCIRun_SOURCE_DIR}/ExampleNets/regression/Network/*.srn5")
  FILE(GLOB_RECURSE scirun_srns_io "${SCIRun_SOURCE_DIR}/ExampleNets/regression/ImportExport/*.srn5")
  set(scirun_srns ${scirun_srns_modules} ${scirun_srns_network} ${scirun_srns_io})

  IF(NOT BUILD_HEADLESS)
    FILE(GLOB_RECURSE scirun_srns_renderer "${SCIRun_SOURCE_DIR}/ExampleNets/regression/Renderer/*.srn5")
    set(scirun_srns ${scirun_srns} ${scirun_srns_renderer})
  ENDIF()

  IF(BUILD_WITH_PYTHON)
    FILE(GLOB_RECURSE scirun_srns_python "${SCIRun_SOURCE_DIR}/ExampleNets/regression/Python/*.srn5")
    set(scirun_srns ${scirun_srns} ${scirun_srns_python})
  ENDIF()

  IF(WITH_OSPRAY)
    FILE(GLOB_RECURSE scirun_srns_ospray "${SCIRun_SOURCE_DIR}/ExampleNets/regression/Ospray/*.srn5")
    set(scirun_srns ${scirun_srns} ${scirun_srns_ospray})
  ENDIF()

  FOREACH(srn ${scirun_srns})
    GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
    SET(test_name ".Test.ExampleNetwork.${srn_name}_srn")
    # In-app regression timeout (seconds). 60 gives the heaviest networks
    # headroom when the machine is under parallel load; genuine hangs are still
    # bounded by the ctest --timeout passed in CI.
    ADD_TEST("${test_name}" ${EXE_LOC} -E ${srn} --no_splash --regression 60 -d ${SCIRUN_TEST_RESOURCE_DIR})

    IF(WIN32)
      SET_TESTS_PROPERTIES("${test_name}" PROPERTIES ENVIRONMENT "${SCIRUN_TEST_DLL_ENVIRONMENT}")
    ENDIF()

    # Networks that instantiate a ViewScene render through an offscreen GL
    # context in regression mode. macOS serializes GPU command submission
    # across contexts, so running several ViewScene networks concurrently can
    # slow rendering enough to trip the in-app regression timeout. Give these
    # tests a shared RESOURCE_LOCK so CTest never schedules two of them at once,
    # while CPU-only tests still parallelize freely.
    FILE(READ "${srn}" srn_contents)
    STRING(FIND "${srn_contents}" "ViewScene" srn_viewscene_pos)
    IF(NOT srn_viewscene_pos EQUAL -1)
      SET_TESTS_PROPERTIES("${test_name}" PROPERTIES RESOURCE_LOCK gpu)
    ENDIF()
    UNSET(srn_contents)
  ENDFOREACH()

ENDIF()

IF(RUN_IMPORT_TESTS)
  FILE(GLOB_RECURSE scirun_legacy_srns "${SCIRUN_TEST_RESOURCE_DIR}/**/*.srn")
  # The glob is broad; drop files that aren't legacy v4 networks: serialization
  # test artifacts (TransientOutput/), scratch outputs (*_TMP.srn), and any
  # converted networks left in the data dir (*_imported.srn5 won't match *.srn,
  # but guard against *_imported.srn just in case).
  LIST(FILTER scirun_legacy_srns EXCLUDE REGEX "(/TransientOutput/|_TMP\\.srn$|_imported)")
  FOREACH(srn ${scirun_legacy_srns})
    # Name tests by path relative to the data dir, not just the basename:
    # several legacy networks share a basename across subdirectories, which
    # would collide into a single CTest test name.
    FILE(RELATIVE_PATH srn_rel "${SCIRUN_TEST_RESOURCE_DIR}" "${srn}")
    STRING(REGEX REPLACE "[/\\\\]" "." srn_rel "${srn_rel}")
    STRING(REGEX REPLACE "\\.srn$" "" srn_rel "${srn_rel}")
    SET(import_test_name ".Test.ImportNetwork.${srn_rel}")
    # Run in GUI mode (no -x): import goes through the GuiCommandFactory's
    # FileImportCommand. The headless console path has no import command.
    ADD_TEST("${import_test_name}" ${EXE_LOC} --no_splash --regression 30 --import ${srn})
    # Bound a hung import at the ctest level.
    SET_TESTS_PROPERTIES("${import_test_name}" PROPERTIES TIMEOUT 120)
    IF(WIN32)
      SET_TESTS_PROPERTIES("${import_test_name}" PROPERTIES ENVIRONMENT "${SCIRUN_TEST_DLL_ENVIRONMENT}")
    ENDIF()
  ENDFOREACH()
ENDIF()

ENDIF()
