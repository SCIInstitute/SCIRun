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

set(ospray_DEPENDENCIES)
set(ospray_DEPENDENCIES)
LIST(APPEND ospray_DEPENDENCIES
  GLM_external
  rkcommon_external
  Embree_external
)

ExternalProject_Get_Property(rkcommon_external INSTALL_DIR)
set(RKCOMMON_INSTALL_DIR ${INSTALL_DIR})

ExternalProject_Get_Property(TBB_external INSTALL_DIR)
set(TBB_INSTALL_DIR ${INSTALL_DIR})

ExternalProject_Get_Property(Embree_external INSTALL_DIR)
set(EMBREE_INSTALL_DIR ${INSTALL_DIR})

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Ospray_external
  DEPENDS ${ospray_DEPENDENCIES}
  GIT_REPOSITORY ${OSPRAY_GIT_URL}
  GIT_TAG ${OSPRAY_GIT_TAG}

  GIT_SUBMODULES ""
  GIT_SUBMODULES_RECURSE OFF

  # EP_UPDATE_DISCONNECTED emits update and update_disconnected as siblings; under
  # -j they race on git lock files. Missed by c552399bb.
  UPDATE_COMMAND ""

  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DENABLE_OSPRAY_SUPERBUILD:BOOL=ON
    -DBUILD_ISA_AVX512:BOOL=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
    -Drkcommon_DIR:PATH=${RKCOMMON_INSTALL_DIR}/lib/cmake/rkcommon-1.11.0
    -DTBB_ROOT:PATH=${TBB_INSTALL_DIR}
    -Dembree_DIR:PATH=${EMBREE_INSTALL_DIR}/lib/cmake/embree-3.13.4
    -DEMBREE_VERSION_REQUIRED:STRING=3.13.0
    -DOSPRAY_ENABLE_ISPC:BOOL=OFF
    -Dglm_DIR:PATH=${GLM_DIR}/cmake/glm
)

ExternalProject_Get_Property(Ospray_external BINARY_DIR)
SET(OSPRAY_BUILD_DIR ${BINARY_DIR} CACHE PATH "")

MESSAGE(STATUS "OSPRAY_BUILD_DIR: ${OSPRAY_BUILD_DIR}")
