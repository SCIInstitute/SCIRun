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

# Borrowed CMake code from the MaidSafe Boost CMake build
# found at https://github.com/maidsafe/MaidSafe/blob/master/cmake_modules/add_boost.cmake
# and code borrowed from ITK4 HDFMacros.cmake

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# disable auto linking
# also set in Seg3D?
SET(boost_CXX_Flags "-DBOOST_ALL_NO_LIB=1")
IF(APPLE)
  LIST(APPEND boost_CXX_Flag "-DBOOST_LCAST_NO_WCHAR_T" "-DBOOST_THREAD_DONT_USE_ATOMIC")
ENDIF()
IF(WIN32)
  LIST(APPEND boost_CXX_Flag "-DBOOST_BIND_ENABLE_STDCALL")
ENDIF()

SET( boost_DEPENDENCIES )

# explicitly set library list
SET(boost_Libraries
  "atomic"
  "chrono"
  "date_time"
  "exception"
  "filesystem"
  "program_options"
  "regex"
  "serialization"
  #"system"
  "thread"
  CACHE INTERNAL "Boost library name.")

IF(BUILD_WITH_PYTHON)
  ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB=1)
  LIST(APPEND boost_Libraries python)
  LIST(APPEND boost_DEPENDENCIES Python_external)
  LIST(APPEND boost_CXX_Flag "-DBOOST_PYTHON_STATIC_MODULE" "-DBOOST_PYTHON_STATIC_LIB")
ENDIF()

# for travis clang builds--need a narrower test
IF(UNIX)
  ADD_DEFINITIONS(-DBOOST_NO_CXX11_ALLOCATOR)
ENDIF()

# TODO: set up 64-bit build detection
# Boost Jam needs to have 64-bit build explicitly configured
IF(WIN32)
  SET(FORCE_64BIT_BUILD ON)
ENDIF()


set(_boost_git_url "https://github.com/CIBC-Internal/boost.git")
set(_boost_git_tag "v1.90.0")

# TODO: fix install step
#
# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Boost_external
  DEPENDS ${boost_DEPENDENCIES}
  GIT_REPOSITORY ${_boost_git_url}
  GIT_TAG ${_boost_git_tag}
  BUILD_IN_SOURCE ON
  PATCH_COMMAND ""
  INSTALL_COMMAND ""        # We manage install manually
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
    -DPython_DIR:PATH=${Python_DIR}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DFORCE_64BIT_BUILD:BOOL=${FORCE_64BIT_BUILD}
    -DSCI_BOOST_LIBRARIES:STATIC=${boost_Libraries}
    -DSCI_BOOST_CXX_FLAGS:STRING=${boost_CXX_Flags}
)

ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)

if(WIN32)
  set(_B2_CMD ${SOURCE_DIR}/b2.exe)
  set(_B2_BOOTSTRAP_CMD bootstrap.bat)
else()
  set(_B2_CMD ${SOURCE_DIR}/b2)
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
endif()

# bootstrap b2
ExternalProject_Add_Step(Boost_external bootstrap_b2
  COMMAND ${_B2_BOOTSTRAP_CMD}
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES update
  COMMENT "Bootstrapping b2"
)

# Generate Boost symlinked headers
ExternalProject_Add_Step(Boost_external stage_headers
  COMMAND ${_B2_CMD} headers
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES bootstrap_b2
  COMMENT "Running b2 headers"
)

# Install full header tree: <INSTALL_DIR>/include/boost/*
ExternalProject_Add_Step(Boost_external install_full_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${INSTALL_DIR}/include/boost
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/boost ${INSTALL_DIR}/include/boost
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  COMMENT "Installing full Boost headers"
)

# Build required Boost libraries
ExternalProject_Add_Step(Boost_external build_libs
  COMMAND ${_B2_CMD}
          --with-atomic
          --with-chrono
          --with-date_time
          --with-filesystem
          --with-program_options
          --with-regex
          --with-serialization
          --with-thread
          $<$<BOOL:${BUILD_WITH_PYTHON}>:--with-python>
          link=static
          runtime-link=static
          variant=release
          threading=multi
          cxxflags=-fPIC
          stage
  WORKING_DIRECTORY ${SOURCE_DIR}
  DEPENDEES stage_headers
  COMMENT "Building Boost static libraries (including Python if enabled)"
)

SET(SCI_BOOST_INCLUDE ${SOURCE_DIR})
SET(SCI_BOOST_LIBRARY_DIR ${SOURCE_DIR}/stage/lib)
SET(SCI_BOOST_USE_FILE ${INSTALL_DIR}/UseBoost.cmake)

SET(BOOST_PREFIX "boost_")
SET(THREAD_POSTFIX "")

SET(SCI_BOOST_LIBRARY)

FOREACH(lib ${boost_Libraries})
  SET(LIB_NAME "${BOOST_PREFIX}${lib}${THREAD_POSTFIX}")
  LIST(APPEND SCI_BOOST_LIBRARY ${LIB_NAME})
ENDFOREACH()

# Boost is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/BoostConfig.cmake.in ${INSTALL_DIR}/BoostConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UseBoost.cmake ${SCI_BOOST_USE_FILE} COPYONLY)

SET(Boost_DIR ${INSTALL_DIR} CACHE PATH "")

MESSAGE(STATUS "Boost_DIR: ${Boost_DIR}")
