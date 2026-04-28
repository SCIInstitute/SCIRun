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

# Superbuild/TBBExternal.cmake

SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(TBB_GIT_REPOSITORY "https://github.com/oneapi-src/oneTBB.git")
set(TBB_GIT_TAG "v2021.11.0")  # Known-compatible with rkcommon 1.11

ExternalProject_Add(TBB_external
  GIT_REPOSITORY ${TBB_GIT_REPOSITORY}
  GIT_TAG        ${TBB_GIT_TAG}

  UPDATE_COMMAND ""
  PATCH_COMMAND ""

  CMAKE_CACHE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DTBB_TEST:BOOL=OFF
    -DTBB_STRICT:BOOL=OFF
    -DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5
)