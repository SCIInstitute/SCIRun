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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# ---- VTK version ----
set(vtk_GIT_TAG "v9.6.1")

# ---- Dependencies ----
set(vtk_DEPENDENCIES "Zlib_external")

# ---- Pull dependency install dirs ----
ExternalProject_Get_Property(Zlib_external INSTALL_DIR)
set(ZLIB_INSTALL_DIR ${INSTALL_DIR})

# ---- ExternalProject ----
ExternalProject_Add(VTK_external
  DEPENDS ${vtk_DEPENDENCIES}

  GIT_REPOSITORY "https://gitlab.kitware.com/vtk/vtk.git"
  GIT_TAG ${vtk_GIT_TAG}

  GIT_SUBMODULES ""
  GIT_SUBMODULES_RECURSE OFF

  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON

    # ---- Core build options ----
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DVTK_BUILD_ALL_MODULES:BOOL=OFF
    -DVTK_GROUP_ENABLE_StandAlone:STRING=WANT
    -DVTK_GROUP_ENABLE_Rendering:STRING=WANT
    -DVTK_GROUP_ENABLE_Imaging:STRING=WANT
    -DVTK_GROUP_ENABLE_IO:STRING=WANT

    # ---- External dependency control ----
    -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:BOOL=ON

    # ---- Help VTK locate them (minimal first pass) ----
    -DZLIB_ROOT=${ZLIB_INSTALL_DIR}

    # ---- Wrapping ----
    -DVTK_WRAP_PYTHON:BOOL=OFF
    -DVTK_WRAP_JAVA:BOOL=OFF

    # ---- Superbuild hygiene ----
    -DVTK_BUILD_TESTING:BOOL=OFF
    -DVTK_BUILD_EXAMPLES:BOOL=OFF

    # ---- Install ----
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

# ---- Export build dir ----
ExternalProject_Get_Property(VTK_external BINARY_DIR)
set(VTK_BUILD_DIR ${BINARY_DIR} CACHE PATH "")

# ---- Export install dir ----
ExternalProject_Get_Property(VTK_external INSTALL_DIR)
set(VTK_INSTALL_DIR ${INSTALL_DIR} CACHE PATH "")

message(STATUS "VTK_BUILD_DIR: ${VTK_BUILD_DIR}")
message(STATUS "VTK_INSTALL_DIR: ${VTK_INSTALL_DIR}")