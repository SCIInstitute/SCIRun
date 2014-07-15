
#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#

ADD_SUBDIRECTORY(Utils)
ADD_SUBDIRECTORY(ModuleTestBase)

########################################################################
# Configure test support

IF(RUN_UNIT_TESTS)
  FOREACH(test ${UNIT_TEST_LIST})
    ADD_TEST("${test}" "../${EXE_DIRPATH}/${test}")
  ENDFOREACH(test ${UNIT_TEST_LIST})
ENDIF()


IF(RUN_BASIC_REGRESSION_TESTS)

   IF(WIN32)
     SET(EXE_LOC ${SCIRun_BINARY_DIR}/${BUILD_TYPE}/SCIRun.exe CACHE PATH "Path to SCIRun executable")
   ELSE(WIN32)
     # TODO: change once testing works with app bundle
     IF(APPLE)
       SET(EXE_LOC ${SCIRun_BINARY_DIR}/${TEST_APPLICATION_NAME} CACHE PATH "Path to SCIRun executable")
     ELSE()
       SET(EXE_LOC ${SCIRun_BINARY_DIR}/${APPLICATION_NAME} CACHE PATH "Path to SCIRun executable")
     ENDIF()
   ENDIF()
   MESSAGE(STATUS "Using executable: ${EXE_LOC} to test networks")
   
  # Run all the possible .srn files in src/ExampleNets
  FILE(GLOB_RECURSE scirun_srns "${SCIRun_SOURCE_DIR}/ExampleNets/regression/*.srn5")
  FOREACH(srn ${scirun_srns})
    GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
    #SET(SRN_ROOT_NAMES "${SRN_ROOT_NAMES} ${srn_name}" CACHE INTERNAL "List of root net names that correspond with image names")
    ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} -E ${srn} --no_splash --regression)
    #FILE(READ ${srn} temp_srn)
    #STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
    #IF(contains_ViewScene)
      #SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
    #ENDIF()
  ENDFOREACH()
ENDIF()
