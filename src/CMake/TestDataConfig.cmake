#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

MACRO(TEST_DATA)
  # master should be always be publicly released version
  SET(GIT_TAG "origin/master")

  SET(test_data_DIR "${CMAKE_BINARY_DIR}/SCIRunTestData")
  SET(test_data_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/download/SCIRunTestData")

  # separate test data files from the rest of the external project files
  SET(ep_base "${test_data_DOWNLOAD_DIR}")
  SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

  # If CMake ever allows overriding the checkout command or adding flags,
  # git checkout -q will silence message about detached head (harmless).
  ExternalProject_Add(SCIRunTestData_external
    GIT_REPOSITORY "https://github.com/CIBC-Internal/SCIRunTestData.git"
    GIT_TAG ${GIT_TAG}
    BUILD_IN_SOURCE ON
    SOURCE_DIR ${test_data_DIR}
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ""
    ${DISABLED_UPDATE}
    INSTALL_COMMAND ""
  )

  # test data location for tests
  SET(SCIRUN_TEST_RESOURCE_DIR ${test_data_DIR} CACHE PATH "Root for all github-based testing files")

ENDMACRO()