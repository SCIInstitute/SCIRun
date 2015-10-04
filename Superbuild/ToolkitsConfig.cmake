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

# Macro to include toolkit repos that contain files that do not need to be compiled
# i.e. networks, data, documentation etc.
MACRO(EXTERNAL_TOOLKIT name)
  SET(toolkit_ExternalProject_name "${name}_external")
  # toolkit master should be always be publicly released version
  SET(toolkit_GIT_TAG "origin/master")

  SET(toolkit_DIR "${CMAKE_BINARY_DIR}/Toolkits/${name}")
  SET(toolkit_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/download/${name}")

  # separate toolkit source from the rest of the external project files
  SET(ep_base "${toolkit_DOWNLOAD_DIR}")
  SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

  # If CMake ever allows overriding the checkout command or adding flags,
  # git checkout -q will silence message about detached head (harmless).
  ExternalProject_Add(${toolkit_ExternalProject_name}
    GIT_REPOSITORY "https://github.com/SCIInstitute/${name}.git"
    GIT_TAG ${toolkit_GIT_TAG}
    BUILD_IN_SOURCE ON
    SOURCE_DIR ${toolkit_DIR}
    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ""
    ${DISABLED_UPDATE}
    INSTALL_COMMAND ""
  )
ENDMACRO()