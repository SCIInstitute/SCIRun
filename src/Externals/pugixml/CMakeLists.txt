cmake_minimum_required(VERSION 3.4)
project(pugixml VERSION 1.12 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
#include(GNUInstallDirs)
#include(CTest)

cmake_dependent_option(PUGIXML_USE_VERSIONED_LIBDIR
  "Use a private subdirectory to install the headers and libraries" OFF
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

cmake_dependent_option(PUGIXML_USE_POSTFIX
  "Use separate postfix for each configuration to make sure you can install multiple build outputs" OFF
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

cmake_dependent_option(PUGIXML_STATIC_CRT
  "Use static MSVC RT libraries" OFF
  "MSVC" OFF)

cmake_dependent_option(PUGIXML_BUILD_TESTS
  "Build pugixml tests" OFF
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

# Custom build defines
set(PUGIXML_BUILD_DEFINES CACHE STRING "Build defines for custom options")
separate_arguments(PUGIXML_BUILD_DEFINES)

# Technically not needed for this file. This is builtin CMAKE global variable.
#option(BUILD_SHARED_LIBS "Build shared instead of static library" OFF)

# Expose option to build PUGIXML as static as well when the global BUILD_SHARED_LIBS variable is set
#cmake_dependent_option(PUGIXML_BUILD_SHARED_AND_STATIC_LIBS
#  "Build both shared and static libraries" OFF
#  "BUILD_SHARED_LIBS" OFF)

# Expose options from the pugiconfig.hpp
option(PUGIXML_WCHAR_MODE "Enable wchar_t mode" OFF)
option(PUGIXML_COMPACT "Enable compact mode" OFF)

# Advanced options from pugiconfig.hpp
option(PUGIXML_NO_XPATH "Disable XPath" OFF)
option(PUGIXML_NO_STL "Disable STL" OFF)
option(PUGIXML_NO_EXCEPTIONS "Disable Exceptions" OFF)
mark_as_advanced(PUGIXML_NO_XPATH PUGIXML_NO_STL PUGIXML_NO_EXCEPTIONS)

# Policy configuration
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW) # Enables use of MSVC_RUNTIME_LIBRARY
endif()

set(PUGIXML_PUBLIC_DEFINITIONS
  $<$<BOOL:${PUGIXML_WCHAR_MODE}>:PUGIXML_WCHAR_MODE>
  $<$<BOOL:${PUGIXML_COMPACT}>:PUGIXML_COMPACT>
  $<$<BOOL:${PUGIXML_NO_XPATH}>:PUGIXML_NO_XPATH>
  $<$<BOOL:${PUGIXML_NO_STL}>:PUGIXML_NO_STL>
  $<$<BOOL:${PUGIXML_NO_EXCEPTIONS}>:PUGIXML_NO_EXCEPTIONS>)

if (BUILD_SHARED_LIBS)
  SCIRUN_ADD_LIBRARY(pugixml SHARED
    #${PROJECT_SOURCE_DIR}/scripts/pugixml_dll.rc
    ${PROJECT_SOURCE_DIR}/pugixml.cpp)
endif()

IF(BUILD_SHARED_LIBS)
	ADD_DEFINITIONS(-DBUILD_pugixml)
ENDIF(BUILD_SHARED_LIBS)
