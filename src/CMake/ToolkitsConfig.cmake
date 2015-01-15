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

# TODO: Would CMake's ExternalData commands work here? Look into it...

MACRO(EXTERNAL_TOOLKITS)
  SET(ep_base "${CMAKE_BINARY_DIR}/Toolkits/BrainStimulator")
  SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

  # BrainStimulator
  # master should be publicly released version
  SET(toolkit_GIT_TAG "origin/master")

  SET(toolkit_DIR "${CMAKE_CURRENT_BINARY_DIR}/Toolkits/BrainStimulator")
  SET(toolkit_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/BrainStimulator")

  # If CMake ever allows overriding the checkout command or adding flags,
  # git checkout -q will silence message about detached head (harmless).
  ExternalProject_Add(BrainStimulator_external
    GIT_REPOSITORY "https://github.com/SCIInstitute/BrainStimulator.git"
    GIT_TAG ${toolkit_GIT_TAG}
    BUILD_IN_SOURCE ON
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ""
    ${DISABLED_UPDATE}
    INSTALL_COMMAND ""
  )
ENDMACRO()