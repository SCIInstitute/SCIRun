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

# TnyExternal.cmake
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Superbuild directories (match your pattern—optional but helps clarity)
set(_tny_inst "${CMAKE_BINARY_DIR}/Externals/Install/Tny_external")

ExternalProject_Add(Tny_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Tny.git"
  GIT_TAG        "origin/master"
  PATCH_COMMAND  ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""

  # Copy only the headers into a conventional layout:
  #   <INSTALL_DIR>/include/tny/tny.hpp
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E make_directory "${_tny_inst}/include/tny" &&
    ${CMAKE_COMMAND} -E copy_directory "<SOURCE_DIR>/tny/src/tny" "${_tny_inst}/include/tny"

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Export variables for SCIRun consumption
ExternalProject_Get_Property(Tny_external SOURCE_DIR)
set(TNY_SOURCE_DIR  "${SOURCE_DIR}")
set(TNY_INSTALL_DIR "${_tny_inst}")
set(TNY_INCLUDE     "${TNY_INSTALL_DIR}/include")
message(STATUS "[Tny_external] INSTALL_DIR=${TNY_INSTALL_DIR}")