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

# QwtExternal.cmake — build Qwt via the internal CMake wrapper (preferred over qmake)

# ----------------------------
# Single place to pin wrapper tag
# ----------------------------
# Bump this tag when you cut a new wrapper release (e.g., v0.1.1, v0.2.0)
set(qwt_WRAPPER_GIT_TAG "v0.1.1")

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

# Qwt install layout
set(QWT_INSTALL_DIR  "${_qwt_inst}")
set(QWT_INCLUDE  "${_qwt_inst}/include")
set(QWT_LIBRARY_DIR  "${_qwt_inst}/lib")

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

# Detect whether we're using a multi-config generator (e.g., Visual Studio)
get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_is_multi)
  # Forward the native config placeholder (e.g., $(Configuration))
  set(_EP_CFG "${CMAKE_CFG_INTDIR}")
else()
  # Single-config (e.g., Ninja/Unix Makefiles) → use CMAKE_BUILD_TYPE (may be empty = default)
  if(CMAKE_BUILD_TYPE)
    set(_EP_CFG "${CMAKE_BUILD_TYPE}")
  else()
    set(_EP_CFG ".")  # no named config; the placeholder is '.' for single-config
  endif()
endif()

ExternalProject_Add(Qwt_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt-cmake-wrapper.git"
  GIT_TAG        ${qwt_WRAPPER_GIT_TAG}

  # Make cloning robust for pinned tags (turn off shallow during stabilization)
  GIT_SHALLOW    0         # <-- changed from 1 to 0
  GIT_PROGRESS   1
  GIT_SUBMODULES ""        # explicit: no submodules

  SOURCE_DIR ${_qwt_src}
  BINARY_DIR ${_qwt_bin}
  INSTALL_DIR ${_qwt_inst}

  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${_qwt_inst}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    ${_qwt_extra_cmake_args}

  # For pinned tags, skip update step entirely to avoid running git in a non-git tree
  UPDATE_COMMAND ""        # <-- added: prevents Qwt_external-gitupdate.cmake

  BUILD_COMMAND   ${CMAKE_COMMAND} --build . --config ${_EP_CFG}
  INSTALL_COMMAND ${CMAKE_COMMAND} --install . --config ${_EP_CFG}

  LOG_DOWNLOAD  1
  LOG_UPDATE    1
  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# ----------------------------
# Exported Qwt library variables (config-aware)
# ----------------------------

if(WIN32)
  # Qwt wrapper uses qwtd.lib for Debug, qwt.lib for Release
  set(QWT_LIBRARY_DEBUG   "${QWT_LIBRARY_DIR}/qwtd.lib")
  set(QWT_LIBRARY_RELEASE "${QWT_LIBRARY_DIR}/qwt.lib")

  # Generator-expression aware library selection
  set(QWT_LIBRARY
    $<$<CONFIG:Debug>:${QWT_LIBRARY_DEBUG}>
    $<$<CONFIG:Release>:${QWT_LIBRARY_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>:${QWT_LIBRARY_RELEASE}>
    $<$<CONFIG:MinSizeRel>:${QWT_LIBRARY_RELEASE}>
  )

elseif(APPLE)
  set(QWT_LIBRARY "${QWT_LIBRARY_DIR}/libqwt.dylib")
else()
  set(QWT_LIBRARY "${QWT_LIBRARY_DIR}/libqwt.so")
endif()

# Export to SCIRun
# Export to parent (SCIRun superbuild)
set(QWT_INSTALL_DIR "${QWT_INSTALL_DIR}" PARENT_SCOPE)
set(QWT_INCLUDE "${QWT_INCLUDE}" PARENT_SCOPE)
set(QWT_LIBRARY_DIR "${QWT_LIBRARY_DIR}" PARENT_SCOPE)
set(QWT_LIBRARY     "${QWT_LIBRARY}"     PARENT_SCOPE)

message(STATUS "[Qwt_external] WRAPPER_TAG=${qwt_WRAPPER_GIT_TAG} ; QWT_TAG=${qwt_GIT_TAG}")
message(STATUS "[Qwt_external] INSTALL_DIR=${QWT_INSTALL_DIR}")
message(STATUS "[Qwt_external] INCLUDE=${QWT_INCLUDE}")
message(STATUS "[Qwt_external] LIBDIR=${QWT_LIBRARY_DIR}")
