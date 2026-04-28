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

# Superbuild/EmbreeExternal.cmake

SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(EMBREE_GIT_REPOSITORY https://github.com/embree/embree.git)
set(EMBREE_GIT_TAG v3.13.4)  # >= 4.0.0, stable

ExternalProject_Add(Embree_external
  DEPENDS TBB_external

  GIT_REPOSITORY ${EMBREE_GIT_REPOSITORY}
  GIT_TAG        ${EMBREE_GIT_TAG}

  UPDATE_COMMAND ""
  PATCH_COMMAND
    ${CMAKE_COMMAND} -E echo "Patching Embree for macOS Clang" &&
    git apply ${SUPERBUILD_DIR}/patches/embree-macos-bezier-fix.patch


  CMAKE_CACHE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5

    -DEMBREE_TUTORIALS:BOOL=OFF
    -DEMBREE_EXAMPLES:BOOL=OFF
    -DEMBREE_TESTING:BOOL=OFF
    -DEMBREE_ISPC_SUPPORT:BOOL=OFF
    -DEMBREE_GEOMETRY_CURVE:BOOL=OFF
    -DEMBREE_GEOMETRY_SUBDIVISION:BOOL=OFF

    -DTBB_ROOT:PATH=${TBB_INSTALL_DIR}
)