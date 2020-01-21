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


IF(BUILD_TESTING OR TRAVIS_BUILD)
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
    ENDFOREACH(PYTHON_TEST)
  ENDIF()

  FOREACH(test ${UNIT_TEST_LIST})
    ADD_TEST("${test}" "../${EXE_DIRPATH}/${test}")
  ENDFOREACH(test ${UNIT_TEST_LIST})
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
    ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} -E ${srn} --no_splash --regression 30 -d ${SCIRUN_TEST_RESOURCE_DIR})
  ENDFOREACH()

ENDIF()

ENDIF()
