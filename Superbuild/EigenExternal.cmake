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


# EigenExternal.cmake
set_property(DIRECTORY PROPERTY EP_BASE ${ep_base})

ExternalProject_Add(Eigen_external
  URL "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz"
  UPDATE_DISCONNECTED 1

  # Standard CMake configure + install (no build needed for header-only)
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
      -S <SOURCE_DIR>
      -B <BINARY_DIR>
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}

  BUILD_COMMAND ""  # header-only

  INSTALL_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --target install

  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

ExternalProject_Get_Property(Eigen_external INSTALL_DIR)
message(STATUS "[Eigen_external] INSTALL_DIR=${INSTALL_DIR}")
