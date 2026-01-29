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


# ZlibExternal.cmake (modernized + consistent)

# ZlibExternal.cmake  —  installs zlib and exports an installed prefix for consumers

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(zlib_GIT_TAG "v1.2.8")

# Common CMake args for the external
# NOTE: We intentionally DO NOT redirect output directories here,
# so that `cmake --install` produces a normal <INSTALL_DIR> layout.
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

  # If your zlib source honors BUILD_SHARED_LIBS, uncomment to force static:
  # -DBUILD_SHARED_LIBS=OFF
)

# Single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Superbuild directories
set(_zlib_src  "${CMAKE_BINARY_DIR}/Externals/Source/Zlib_external")
set(_zlib_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Zlib_external")
set(_zlib_inst "${CMAKE_BINARY_DIR}/Externals/Install/Zlib_external")

ExternalProject_Add(Zlib_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/zlib.git"
  GIT_TAG        ${zlib_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_zlib_src}
  BINARY_DIR ${_zlib_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}

  # Run the install step for this external (required for Fix B)
  INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --target install

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# ----------------------------------------------------------------------
# Export variables for downstream projects (FreeType, SCIRun, etc.)
# Expect a standard install layout:
#   <INSTALL_DIR>/include/{zlib.h,zconf.h}
#   <INSTALL_DIR>/lib/{z.lib|libz.a|libz.so|libz.dylib}
# ----------------------------------------------------------------------
set(ZLIB_SOURCE_DIR  ${_zlib_src})
set(ZLIB_INSTALL_DIR ${_zlib_inst})

# Public headers for consumers (FreeType only needs zlib.h)
set(ZLIB_INCLUDE     ${ZLIB_INSTALL_DIR}/include)

# Library directory
set(ZLIB_LIBRARY_DIR ${ZLIB_INSTALL_DIR}/lib)

# Pick the most likely library filename for each platform
if(WIN32)
  # Your custom zlib typically names the static lib 'z.lib'.
  set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/z.lib")
elseif(APPLE)
  # Prefer shared if present, otherwise static
  if(EXISTS "${ZLIB_LIBRARY_DIR}/libz.dylib")
    set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/libz.dylib")
  else()
    set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/libz.a")
  endif()
else()
  # Linux/*nix: prefer shared if present, otherwise static
  if(EXISTS "${ZLIB_LIBRARY_DIR}/libz.so")
    set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/libz.so")
  else()
    set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/libz.a")
  endif()
endif()

message(STATUS "[Zlib_external] INSTALL_DIR=${ZLIB_INSTALL_DIR}")
message(STATUS "[Zlib_external] INCLUDE=${ZLIB_INCLUDE}")
message(STATUS "[Zlib_external] LIB=${ZLIB_LIBRARY}")