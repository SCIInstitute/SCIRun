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

# QwtExternal.cmake — build Qwt via the internal CMake wrapper (preferred over qmake)

# ----------------------------
# Single place to pin wrapper tag
# ----------------------------
# Bump this tag when you cut a new wrapper release (e.g., v0.1.1, v0.2.0)
set(qwt_WRAPPER_GIT_TAG "v0.1.0")

# Keep the legacy variable for visibility; it's the upstream Qwt tag
# that your wrapper fetches internally (informational only here).
set(qwt_GIT_TAG "v6.3.0")

# Ensure ExternalProject directories are rooted under the superbuild 'ep_base'
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# ----------------------------
# Superbuild directories
# ----------------------------
set(_qwt_src  "${CMAKE_BINARY_DIR}/Externals/Source/Qwt_external")
set(_qwt_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Qwt_external")
set(_qwt_inst "${CMAKE_BINARY_DIR}/Externals/Install/Qwt_external")

# ----------------------------
# Qt discovery hints (optional)
# ----------------------------
set(_qwt_extra_cmake_args "")
if(DEFINED Qt6_DIR)
  list(APPEND _qwt_extra_cmake_args "-DQt6_DIR=${Qt6_DIR}")
endif()
if(DEFINED Qt5_DIR)
  list(APPEND _qwt_extra_cmake_args "-DQt5_DIR=${Qt5_DIR}")
endif()

include(ExternalProject)

ExternalProject_Add(Qwt_external
  # --------------
  # Your internal wrapper repo (CMake-based)
  # --------------
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt-cmake-wrapper.git"
  GIT_TAG        ${qwt_WRAPPER_GIT_TAG}
  GIT_SHALLOW    1
  GIT_PROGRESS   1
  # NOTE: leave UPDATE_DISCONNECTED OFF for the first successful fetch.
  # You can re-enable it later to avoid network hits on re-configures.
  # UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_qwt_src}
  BINARY_DIR ${_qwt_bin}
  INSTALL_DIR ${_qwt_inst}

  # ----------------------------
  # Configure the wrapper (it will fetch & build official Qwt inside)
  # ----------------------------
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${_qwt_inst}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}    # honored for single-config generators
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    # Pass Qt hints if provided:
    ${_qwt_extra_cmake_args}

  # ----------------------------
  # MSVC/VS multi-config forwarding (Debug/Release safe)
  # ----------------------------
  BUILD_COMMAND   ${CMAKE_COMMAND} --build . --config $<IF:$<CONFIG:>,$<CONFIG>,${CMAKE_BUILD_TYPE}>
  INSTALL_COMMAND ${CMAKE_COMMAND} --install . --config $<IF:$<CONFIG:>,$<CONFIG>,${CMAKE_BUILD_TYPE}>

  LOG_DOWNLOAD  1
  LOG_UPDATE    1
  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Exported variables for inner project (diagnostics/hints)
set(QWT_SOURCE_DIR  ${_qwt_src})
set(QWT_BUILD_DIR   ${_qwt_bin})
set(QWT_INSTALL_DIR ${_qwt_inst})
set(QWT_INCLUDE     "${QWT_INSTALL_DIR}/include")
set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")
# Consumers should prefer find_package(Qwt CONFIG), but we keep these for legacy:
set(QWT_LIBRARY     "qwt")

message(STATUS "[Qwt_external] WRAPPER_TAG=${qwt_WRAPPER_GIT_TAG} ; QWT_TAG=${qwt_GIT_TAG}")
message(STATUS "[Qwt_external] INSTALL_DIR=${QWT_INSTALL_DIR}")