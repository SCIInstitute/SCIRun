#
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

IF(UNIX)
  # temporary - needed to make Travis CI build work
  # TODO: revisit and delete when CMake 2.8.11 or greater is available
  # or switch to another CI tool
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
ELSE()
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)
ENDIF()

IF(CMAKE_VERSION VERSION_GREATER 3.0)
  # TODO: when support for versions older than 2.8.12 is dropped,
  # enable new policy and get rid of version check
  #
  # see https://cmake.org/cmake/help/v3.0/policy/CMP0042.html
  CMAKE_POLICY(SET CMP0042 OLD)
ENDIF()

PROJECT(SCIRun)

###############################################################################
# Don't allow the user to build in the src directory...
#

IF(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
  MESSAGE( "" )
  MESSAGE( "Building SCIRun in the source directory is not allowed." )
  MESSAGE( "(In order to clean up, you now need to run: rm -rf CMakeFiles CMakeCache.txt" )
  MESSAGE( "" )
  MESSAGE( FATAL_ERROR "Goodbye." )
ENDIF()

################################################################################
# If building on Windows, make sure that the path to the bin directory is short.
# TODO: Revisit if CPM module names get shorter.

IF(WIN32)
  STRING(LENGTH ${CMAKE_BINARY_DIR} BIN_DIR_LENGTH)
  IF(${BIN_DIR_LENGTH} GREATER 15)
    MESSAGE(WARNING "Binary directory is too long, the build may fail due to long path names.")
  ENDIF()
ENDIF()

########################################################################
# Build shared libraries by default

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)

########################################################################
# Headless build

OPTION(BUILD_HEADLESS "Build headless version, without Qt" OFF)
MARK_AS_ADVANCED(BUILD_HEADLESS)


########################################################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds

IF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)

########################################################################
# Git revision details

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules ${CMAKE_MODULE_PATH})

# Make a version file containing the current version from git.
include(GetGitRevisionDescription)
git_describe(VERSION_TAG --tag)
git_describe(VERSION_BRANCH --all)
get_git_head_revision(VERSION_REFSPEC VERSION_HASHVAR)

MESSAGE(STATUS "Git version tag: " "${VERSION_TAG}")
MESSAGE(STATUS "Git version branch: " "${VERSION_BRANCH}")
MESSAGE(STATUS "Git version commit hash: " "${VERSION_HASHVAR}")

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION_TAG}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION_TAG}")
#TODO: make generic
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\-(alpha.[A-Za-z0-9_]+)\\-?.*" "\\1" VERSION_PATCH "${VERSION_TAG}")
#TODO: parse SHA
#string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
#set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

#MESSAGE(STATUS "Git version major: " "${VERSION_MAJOR}")
#MESSAGE(STATUS "Git version minor: " "${VERSION_MINOR}")
#MESSAGE(STATUS "Git version patch name: " "${VERSION_PATCH}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/Version.cc.in
 ${CMAKE_CURRENT_SOURCE_DIR}/Core/Application/Version.cc)
set(version_file "${CMAKE_CURRENT_SOURCE_DIR}/Core/Application/Version.cc")


########################################################################
# SCIRUN version number.

SET(SCIRUN_VERSION_MAJOR "${VERSION_MAJOR}")
SET(SCIRUN_VERSION_MINOR "${VERSION_MINOR}")
SET(SCIRUN_VERSION_PATCH "${VERSION_PATCH}")

SET(SCIRUN_VERSION_STRING "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


########################################################################
# APPLICATION Information (used by CPack)
#
# Based on Seg3D settings...

SET(APPLICATION_NAME "SCIRun" CACHE INTERNAL "Application name" FORCE)
# TODO: temporary hack to allow testing
SET(TEST_APPLICATION_NAME "SCIRun_test" CACHE INTERNAL "Application name" FORCE)
# add others?
SET(APPLICATION_EXTENSION ".srn5" CACHE INTERNAL "Network file extension" FORCE)
SET(APPLICATION_URL "www.scirun.org" CACHE INTERNAL "Project URL" FORCE)
SET(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Vendor information" FORCE)
SET(APPLICATION_CONTACT "scirun-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
SET(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\scirun5_32.ico" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_WINDOWS_RC "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\SCIRun.rc" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Resources/Icons/scirun5.icns" CACHE INTERNAL "OSX icon" FORCE)
SET(APPLICATION_OSX_ICON "scirun5.icns" CACHE INTERNAL "OSX icon" FORCE)

########################################################################
# Standard system and compiler tests

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(ExternalProject)

IF(WIN32)
  ADD_DEFINITIONS("/wd4251 /wd4275 -D_CRT_SECURE_NO_WARNINGS")
ENDIF(WIN32)

########################################################################
# External project utilities

OPTION(DISABLE_ExternalProject_UPDATE "Turn off ExternalProject UPDATE_COMMAND property (if not connected to the internet, for example" ON)
MARK_AS_ADVANCED(DISABLE_ExternalProject_UPDATE)
IF(DISABLE_ExternalProject_UPDATE)
  # do nothing commands...
  IF(WIN32)
    SET(DISABLED_UPDATE "UPDATE_COMMAND" "rem")
  ELSE()
    SET(DISABLED_UPDATE "UPDATE_COMMAND" "echo")
  ENDIF()
ENDIF()

########################################################################
# Configure test support

INCLUDE( CTest )

# ON by default
IF(BUILD_TESTING)
  ENABLE_TESTING()

  # Hack BUILDNAME to differentiate from regular SCIRun builds
  # TODO: remove when transitioning to production code
  SET(BUILDNAME "${BUILDNAME_SCIRun}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)

  #INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/TestDataConfig.cmake)
  #TEST_DATA()

  OPTION(RUN_UNIT_TESTS "Run gtest unit tests" ON)
  OPTION(RUN_BASIC_REGRESSION_TESTS "Run basic regression tests" ON)

  IF(NOT EXISTS "${SCIRUN_TEST_RESOURCE_DIR}")
    MESSAGE( WARNING "Test resource path does not exist. Please set it correctly to run all the unit and regression tests. Clone this github repo to get all the files: https://github.com/CIBC-Internal/SCIRunTestData" )
  ENDIF()

  ADD_DEFINITIONS(-DTEST_RESOURCE_ROOT_DIR="${SCIRUN_TEST_RESOURCE_DIR}" -DBUILD_TESTING)
ENDIF()

#########################################################################
## Python
#
#OPTION(BUILD_WITH_PYTHON "Build with python support." OFF)
#
#IF(BUILD_WITH_PYTHON)
#  MESSAGE(STATUS "Configuring Python")
#  SET(PythonLibs_FIND_VERSION "3.2")
#  FIND_PACKAGE(PythonLibs REQUIRED)
#  IF(NOT PYTHONLIBS_FOUND)
#    MESSAGE(FATAL_ERROR "Python 3.3 or 3.2 not found. Set PYTHON_INCLUDE_DIR and PYT#HON_LIBRARY to the correct Python installation.")
#  ENDIF()
#
#  ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB)
#  ADD_DEFINITIONS(-DBUILD_WITH_PYTHON)
#ENDIF(BUILD_WITH_PYTHON)


FIND_PACKAGE(Zlib CONFIGS ZlibConfig.cmake HINTS ${Zlib_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
IF(NOT Zlib_FOUND)
  MESSAGE(FATAL_ERROR "Zlib library not found in ${Zlib_DIR}")
ENDIF()
INCLUDE(${ZLIB_USE_FILE})

FIND_PACKAGE(LibPNG CONFIGS LibPNGConfig.cmake HINTS ${LibPNG_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
IF(NOT LibPNG_FOUND)
  MESSAGE(FATAL_ERROR "Png library not found in ${LibPNG_DIR}")
ENDIF()
INCLUDE(${PNG_USE_FILE})

FIND_PACKAGE(SQLite CONFIGS SQLiteConfig.cmake HINTS ${SQLite_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
IF(NOT SQLite_FOUND)
  MESSAGE(FATAL_ERROR "SQLite library not found in ${SQLite_DIR}")
ENDIF()
INCLUDE(${SQLITE_USE_FILE})

FIND_PACKAGE(Teem CONFIGS TeemConfig.cmake HINTS ${Teem_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
IF(NOT Teem_FOUND)
  MESSAGE(FATAL_ERROR "Teem library not found in ${Teem_DIR}")
ENDIF()
INCLUDE(${Teem_USE_FILE})

IF(BUILD_WITH_PYTHON)
  FIND_PACKAGE(Python CONFIGS PythonConfig.cmake HINTS ${Python_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
  IF(NOT Python_FOUND)
    MESSAGE(FATAL_ERROR "Python library not found in ${Python_DIR}")
  ENDIF()
  INCLUDE(${SCI_PYTHON_USE_FILE})
ENDIF()

FIND_PACKAGE(Boost CONFIGS BoostConfig.cmake HINTS ${Boost_DIR} NO_SYSTEM_ENVIRONMENT_PATH)
IF(NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Boost library not found in ${Boost_DIR}")
ENDIF()
INCLUDE(${SCI_BOOST_USE_FILE})

########################################################################
# Custom compiler flags and definitions

# TODO: Better compiler check for OS X?
# Try CMAKE_<LANG>_COMPILER_ID MATCHES Clang? Or check for Intel, GNU?
IF(UNIX)
  SET(CXX_LANGUAGE_STANDARD_FLAG "-std=c++11")
  CHECK_CXX_COMPILER_FLAG(${CXX_LANGUAGE_STANDARD_FLAG} USE_CXX11_FLAG)
  IF(NOT USE_CXX11_FLAG)
    SET(CXX_LANGUAGE_STANDARD_FLAG "-std=c++0x")
    CHECK_CXX_COMPILER_FLAG(${CXX_LANGUAGE_STANDARD_FLAG} USE_CXX0x_FLAG)
    IF(NOT USE_CXX0x_FLAG)
      SET(CXX_LANGUAGE_STANDARD_FLAG "")
    ENDIF()
  ENDIF()

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LANGUAGE_STANDARD_FLAG} -Wall")
  IF(NOT APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl -lrt")
  ENDIF()
ENDIF()

IF(APPLE)
  SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -ftemplate-depth=256")
  SET(CMAKE_CXX_FLAGS_DEBUG "-Wshorten-64-to-32 ${CMAKE_CXX_FLAGS_DEBUG}")

  ADD_DEFINITIONS(-DGTEST_USE_OWN_TR1_TUPLE=1)
ENDIF()

########################################################################
# Find and configure Git

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
ENDIF()

########################################################################
# Find and configure Awk & Sed  {(OS X only), needed to build App bundle

# should be installed by default, searching system paths only
IF(APPLE)
  FIND_PROGRAM(MACDEPLOYQT
    NAMES macdeployqt
  )
  MARK_AS_ADVANCED(MACDEPLOYQT)

  SET(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING "Set macdeployqt output level (0-3)")
  MARK_AS_ADVANCED(MACDEPLOYQT_OUTPUT_LEVEL)

  FIND_PROGRAM(SED
    NAMES sed
  )
  MARK_AS_ADVANCED(SED)

  FIND_PROGRAM(AWK
    NAMES awk
  )
  MARK_AS_ADVANCED(AWK)
ENDIF()


########################################################################
# Configure documentation

OPTION(BUILD_DOXYGEN_DOCUMENTATION "Generate Doxygen-based documentation." OFF)
MARK_AS_ADVANCED(BUILD_DOXYGEN_DOCUMENTATION)

IF(BUILD_DOXYGEN_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)

  IF(DOXYGEN_FOUND)
    OPTION(COPY_DOXYGEN_TO_SERVER "Copy Doxygen documentation to server." OFF)
    MARK_AS_ADVANCED(COPY_DOXYGEN_TO_SERVER)
    ADD_SUBDIRECTORY(Documentation/Doxygen)
  ELSE()
    SET(BUILD_DOXYGEN_DOCUMENTATION OFF CACHE BOOL "Generate doxygen-based documentation." FORCE)
  ENDIF()
ENDIF()

# TODO: uncomment when LaTeX files are available
# OPTION(BUILD_LATEX_DOCUMENTATION "Compile SCIRun LaTeX manuals and tutorials to PDF." OFF)
# MARK_AS_ADVANCED(BUILD_LATEX_DOCUMENTATION)
#
# IF(BUILD_LATEX_DOCUMENTATION)
#   FIND_PACKAGE(LATEX)
#   IF(PDFLATEX_COMPILER)
#     IF(BUILD_DOCUMENTATION)
#     ADD_SUBDIRECTORY(Documentation/Manuals)
#   ENDIF()
# ENDIF()

########################################################################
# Download external data sources

#OPTION(DOWNLOAD_TOOLKITS "Download toolkit repositories." ON)
#MARK_AS_ADVANCED(DOWNLOAD_TOOLKITS)
#IF(DOWNLOAD_TOOLKITS)
#  INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/ToolkitsConfig.cmake)
#  EXTERNAL_TOOLKIT(BrainStimulator)
#  EXTERNAL_TOOLKIT(FwdInvToolkit)
#ENDIF()
#
#########################################################################
## Configure zlib external project
#
#INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/ZlibConfig.cmake)
#EXTERNAL_ZLIB_LIBRARY(GIT)
#
#
##########################################################################
## Configure Boost external project
#
#INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/BoostConfig.cmake)
#EXTERNAL_BOOST_LIBRARY(GIT)
#
#
#########################################################################
## Configure teem external project
#
#INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/TeemConfig.cmake)
#EXTERNAL_TEEM_LIBRARY(GIT)


########################################################################
# Configure tetgen external project

OPTION(WITH_TETGEN "Build Tetgen." ON)
MARK_AS_ADVANCED(WITH_TETGEN)
IF(WITH_TETGEN)
  ADD_DEFINITIONS(-DWITH_TETGEN)
ENDIF(WITH_TETGEN)

########################################################################
# Configure CPM and Spire - http://github.com/iauns/cpm
# Configure CPM and Spire - http://github.com/CIBC-Internal/cpm

MESSAGE(STATUS "Configuring CPMs")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# CPM Setup.
#set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/cpm-packages" CACHE TYPE STRING)
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "CPM requires Git.")
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
  message(STATUS "Cloning repo (https://github.com/CIBC-Internal/cpm)")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/CIBC-Internal/cpm ${CPM_DIR}
    RESULT_VARIABLE error_code
    OUTPUT_QUIET ERROR_QUIET)
  if(error_code)
    message(FATAL_ERROR "CPM failed to get the hash for HEAD")
  endif()
endif()
include(${CPM_DIR}/CPM.cmake)

# TODO: Add tags for *all* of the CPM modules. This will allow upstream
# flexibility. Once all versions are appropriately tagged, I will be able
# to untag upstream and keep it moving forward.

# ++ MODULE: GL Platform
CPM_AddModule("gl_platform"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-gl-platform"
  GIT_TAG "origin/master")

# ++ MODULE: Arc-Look-At (Autoview Camera)
CPM_AddModule("look_at"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-arc-look-at"
  GIT_TAG "origin/SCIRun")

# ++ MODULE: Entity System
CPM_AddModule("es"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-entity-system"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Entity system component serialization
CPM_AddModule("es_cereal"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-cereal"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Cereal support for GLM.
CPM_AddModule("cereal_glm"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-cereal-glm"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: ES Systems
CPM_AddModule("es_systems"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-systems"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Async filesystem support
CPM_AddModule("es_fs"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-fs"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: General transforms and components
CPM_AddModule("es_general"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-general"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Entity System Rendering.
CPM_AddModule("es_render"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-render"
  GIT_TAG "origin/SCIRun"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Acorn
CPM_AddModule("es_acorn"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-es-acorn"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: Var buffer
CPM_AddModule("var_buffer"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-var-buffer"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

# ++ MODULE: BSerialize
CPM_AddModule("bserialize"
  GIT_REPOSITORY "https://github.com/CIBC-Internal/cpm-bserialize"
  GIT_TAG "origin/master"
  USE_EXISTING_VER TRUE)

CPM_Finish()

MESSAGE(STATUS "Done configuring CPMs")

########################################################################
# Copy Spire-SCIRun specific assets and shaders

set(OUTPUT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
set(OUTPUT_ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/Assets)

function (CopyShaders src_shader_dir dest_shader_dir)
  if (NOT dest_shader_dir STREQUAL "")
    file(COPY ${src_shader_dir}/ DESTINATION ${dest_shader_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

function (CopyAssets src_asset_dir dest_asset_dir)
  if (NOT dest_asset_dir STREQUAL "")
    file(COPY ${src_asset_dir}/ DESTINATION ${dest_asset_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

if (DEFINED OUTPUT_SHADER_DIR)
  CopyShaders(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/shaders ${OUTPUT_SHADER_DIR})
else()
  message(WARNING "Not copying shader files. OUTPUT_SHADER_DIR variable not found.")
endif()

if (DEFINED OUTPUT_ASSET_DIR)
  CopyAssets(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/assets ${OUTPUT_ASSET_DIR})
else()
  message(WARNING "Not copying asset files. OUTPUT_ASSET_DIR variable not found.")
endif()

########################################################################
# Find and configure OpenGL

FIND_PACKAGE(OpenGL REQUIRED)

########################################################################
# Find and configure Qt

# TODO: exclude for headless
IF(NOT BUILD_HEADLESS)
  FIND_PACKAGE(Qt4 COMPONENTS QtMain QtCore QtGui QtNetwork QtOpenGL REQUIRED)

  IF(QT_FOUND)
    INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
  ELSE()
    MESSAGE(FATAL_ERROR "QT 4.8 or greater is required to build SCIRun")
  ENDIF()

  ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
ENDIF()

########################################################################
# Find and configure Qwt custom widgets

OPTION(WITH_QWT_WIDGETS "Qwt Custom Widgets" OFF)
MARK_AS_ADVANCED(WITH_QWT_WIDGETS)

IF(WITH_QWT_WIDGETS)
  SET(QWT_INCLUDE_DIR CACHE PATH "Qwt include directory")
  IF(NOT QWT_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "QWT_INCLUDE_DIR should be set to path like 'qwt-6.0.1/include'")
  ENDIF()
  INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIR})
  SET(QWT_LIBRARY CACHE FILEPATH "Qwt library")
  IF(NOT QWT_LIBRARY)
  	MESSAGE(FATAL_ERROR "QWT_LIBRARY should be set to a filepath like 'qwt-6.0.1/lib/libqwt.so'")
  ENDIF()
  SET(qwt_libraries ${QWT_LIBRARY} )
ENDIF()

########################################################################
# Find and configure dl library

IF(UNIX)
  FIND_LIBRARY(DL_LIBRARY dl)
ENDIF()


########################################################################
# Includes

SET(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SCIRun_SOURCE_DIR}/
  ${SCIRun_SOURCE_DIR}/include
  ${SCIRun_SOURCE_DIR}/Externals/gmock/gtest/include
  ${SCIRun_SOURCE_DIR}/Externals/gmock/include
  ${SCIRun_SOURCE_DIR}/Externals/libxml2/include
  ${CMAKE_CURRENT_BINARY_DIR}/Externals/libxml2/include
  ${SCIRun_SOURCE_DIR}/Externals/
  ${SCIRun_BINARY_DIR}/
  ${SCIRun_BINARY_DIR}/include
  ${SCIRun_SOURCE_DIR}/Externals/eigen
  # TODO: set up include variable in log4cpp project
  ${SCIRun_SOURCE_DIR}/Externals/log4cpp/include
  ${SCI_TEEM_INCLUDE}
  ${SCI_ZLIB_INCLUDE}
  ${SCI_TETGEN_INCLUDE}
)

INCLUDE_DIRECTORIES(SYSTEM ${SCI_BOOST_INCLUDE})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(QT_USE_QTOPENGL TRUE)

########################################################################
# Macros

MACRO(SCIRUN_ADD_UNIT_TEST test)
  ADD_EXECUTABLE(${test} ${ARGN})
  LIST(APPEND tmp ${test} ${UNIT_TEST_LIST})
  SET(UNIT_TEST_LIST ${tmp} CACHE INTERNAL "List of unit tests." FORCE)
ENDMACRO()

MACRO(SCIRUN_ADD_TEST_DIR directory)
  IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(${directory})
  ENDIF()
ENDMACRO()

MACRO(SCIRUN_ADD_LIBRARY library)
  ADD_LIBRARY(${library} ${ARGN})

  # Set up installer package (Release build only)
  IF(WIN32)
    INSTALL(TARGETS ${library} RUNTIME DESTINATION bin CONFIGURATIONS Release)
  ENDIF()
ENDMACRO()

########################################################################
# Output directories.

SET(SCIRun_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCIRun_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCIRun_BINARY_DIR} CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY)

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Externals)
ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Graphics)

ADD_SUBDIRECTORY(Main)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

########################################################################
# sci_defs variables
# TODO: temporarily included to support legacy code

# sci_defs/error_defs.h
SET(THROW_LEVEL 0 CACHE STRING "Set throw level N(0-3)")
SET(ASSERTION_LEVEL 1 CACHE STRING "Set assertion level N(0-3)")
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(ASSERTION_LEVEL 3)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

# sci_defs/environment_defs.h
SET(scirun_src_dir "${SCIRun_SOURCE_DIR}" CACHE INTERNAL "Path to SCIRun's src directory")
# DV Technically, I don't have access to this variable once we fix building
# these projets separately!!!
SET(scirun_obj_dir ${SCIRun_BINARY_DIR} CACHE INTERNAL "Path to SCIRun's binary directory")

MARK_AS_ADVANCED(THROW_LEVEL ASSERTION_LEVEL DEF_HASH)
# DV Add with_exc stuff from configure.ac
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS)
IF(HAVE_LIMITS)
  SET(HAVE_LIMITS_H 1)
ELSE(HAVE_LIMITS)
  SET(HAVE_LIMITS_H 0)
ENDIF(HAVE_LIMITS)

CHECK_INCLUDE_FILE(sys/select.h HAVE_SYS_SELECT)
IF(HAVE_SYS_SELECT)
  SET(HAVE_SYS_SELECT_H 1)
ELSE(HAVE_SYS_SELECT)
  SET(HAVE_SYS_SELECT_H 0)
ENDIF(HAVE_SYS_SELECT)

CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME)
IF(HAVE_SYS_TIME)
  SET(HAVE_SYS_TIME_H 1)
ELSE(HAVE_SYS_TIME)
  SET(HAVE_SYS_TIME_H 0)
ENDIF(HAVE_SYS_TIME)

# BW Fix to figure out if we have ext/algorithm
#TRY_COMPILE(HAVE_EXT_ALGORITHM
#  ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp
#  ${SCIRUN_CORE_SOURCE_DIR}/CMake/SCIRunTestExtAlgorithm.cc
#  OUTPUT_VARIABLE OUTPUT)
IF(HAVE_EXT_ALGORITHM)
  SET(HAVE_EXT_ALGORITHM_H 1)
ELSE(HAVE_EXT_ALGORITHM)
  SET(HAVE_EXT_ALGORITHM_H 0)
ENDIF(HAVE_EXT_ALGORITHM)

CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT)
IF(HAVE_STDINT)
  SET(HAVE_STDINT_H 1)
ELSE(HAVE_STDINT)
  SET(HAVE_STDINT_H 0)
ENDIF(HAVE_STDINT)

CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES)
IF(HAVE_INTTYPES)
  SET(HAVE_INTTYPES_H 1)
ELSE(HAVE_INTTYPES)
  SET(HAVE_INTTYPES_H 0)
ENDIF(HAVE_INTTYPES)

SET(HAVE_SYS_UUID_H 0)
SET(HAVE_UUID_UUID_H 0)

ADD_SUBDIRECTORY(include)

########################################################################
# Organize projects into folders for Visual Studio projects

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET Algorithms_Base         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Field         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Describe         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_BrainStimulator         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Factory   PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Fields         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Forward         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Visualization         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_FiniteElements         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Geometry         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_DataIO         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Converter         PROPERTY FOLDER "Core/Algorithms")

SET_PROPERTY(TARGET Dataflow_Network         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Mesh         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Nrrd         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Bundle         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Basis         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Persistent         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_DatabaseManager         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ConsoleApplication         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Command         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_SystemCall         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Thread         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ImportExport         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_IEPlugin         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Geometry_Primitives         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Preferences         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Session         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Utils         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ICom         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Services         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Math         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Matlab         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Parser         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_XMLUtil         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Exceptions_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Util_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Containers         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Thread_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Serialization_Network         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Core_Serialization_Network_Importer         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Dataflow_State         PROPERTY FOLDER "Dataflow")

SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_FiniteElements   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_BrainStimulator   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Testing   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Fields   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Bundle   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Matlab   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Forward   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Inverse   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Teem   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Legacy_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Fields   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_FiniteElements   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_Legacy_Bundle   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_BrainStimulator   PROPERTY FOLDER "Modules/Brain")
SET_PROPERTY(TARGET Modules_Legacy_FiniteElements   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_Legacy_Fields PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Legacy_Converters PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Legacy_Forward PROPERTY FOLDER "Modules/Forward")
SET_PROPERTY(TARGET Modules_Legacy_Inverse PROPERTY FOLDER "Modules/Inverse")
SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Legacy_MatlabInterface PROPERTY FOLDER "Modules/Matlab")
SET_PROPERTY(TARGET Modules_Legacy_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Legacy_Teem_DataIO   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Misc   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Converters   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

SET_PROPERTY(TARGET Graphics_Glyphs   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Widgets   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Datatypes   PROPERTY FOLDER "Graphics")

IF(BUILD_TESTING)
  SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gmock_main   PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest_main   PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_Utils PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_ModuleTestBase PROPERTY FOLDER "Testing Support")
  #SET_PROPERTY(TARGET SCIRunTestData_external   PROPERTY FOLDER "Testing Support")

  SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_BrainStimulator_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Field_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Describe_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_FiniteElements_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Core_Application_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Application_Session_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Basis_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Command_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_CommandLine_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Containers_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Mesh_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Geometry_Primitives_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Math_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Serialization_Network_Tests         PROPERTY FOLDER "Dataflow/Serialization/Tests")
  SET_PROPERTY(TARGET Core_Thread_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Utils_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_XMLUtil_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Parser_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_DatabaseManager_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_ImportExport_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_IEPlugin_Tests         PROPERTY FOLDER "Core/Tests")

  SET_PROPERTY(TARGET Engine_Network_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
  SET_PROPERTY(TARGET Engine_Scheduler_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")

  SET_PROPERTY(TARGET Dataflow_Network_Tests   PROPERTY FOLDER "Dataflow/Tests")
  SET_PROPERTY(TARGET Dataflow_State_Tests   PROPERTY FOLDER "Dataflow/Tests")

  SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
  SET_PROPERTY(TARGET Modules_Bundle_Tests   PROPERTY FOLDER "Modules/Basic")
  SET_PROPERTY(TARGET Modules_BrainStimulator_Tests   PROPERTY FOLDER "Modules/Brain")
  SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
  SET_PROPERTY(TARGET Modules_Teem_DataIO_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Teem_Misc_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Teem_Converters_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Fields_Tests   PROPERTY FOLDER "Modules/Fields")
  SET_PROPERTY(TARGET Modules_Legacy_Inverse_Tests   PROPERTY FOLDER "Modules/Inverse")
  SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
  SET_PROPERTY(TARGET Modules_FiniteElements_Tests   PROPERTY FOLDER "Modules/FiniteElements")
  SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
  SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
  SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
  SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO_Tests PROPERTY FOLDER "Modules/DataIO")
  SET_PROPERTY(TARGET Modules_Factory_Tests   PROPERTY FOLDER "Modules/Basic")

  SET_PROPERTY(TARGET Interface_Modules_Render_Tests PROPERTY FOLDER "Interface/Tests")
  IF(BUILD_WITH_PYTHON)
    SET_PROPERTY(TARGET Core_Python_Tests     PROPERTY FOLDER "Core/Tests")
    SET_PROPERTY(TARGET Engine_Python_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
  ENDIF()
ENDIF()

#SET_PROPERTY(TARGET Boost_external   PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET ${CPM_LIBRARIES} PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET ${SCI_LOG4CPP_LIBRARY} PROPERTY FOLDER "Externals")
#SET_PROPERTY(TARGET sqlite PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET cleaver PROPERTY FOLDER "Externals")
#SET_PROPERTY(TARGET Zlib_external   PROPERTY FOLDER "Externals")
#SET_PROPERTY(TARGET Teem_external   PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET xml2 PROPERTY FOLDER "Externals")

IF(BUILD_WITH_PYTHON)
  SET_PROPERTY(TARGET Core_Python         PROPERTY FOLDER "Core")
  SET_PROPERTY(TARGET SCIRunPythonAPI PROPERTY FOLDER "Dataflow/Engine")
ENDIF()

IF(WITH_TETGEN)
  SET_PROPERTY(TARGET tetgen         PROPERTY FOLDER "Externals")
  SET_PROPERTY(TARGET tetlib         PROPERTY FOLDER "Externals")
ENDIF()

########################################################################
# Custom CMake settings for CPack

########################################################################
# CPack

########################################################################
# PACKAGING Information

SET(PACKAGING_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" CACHE INTERNAL "Where to find the license file." FORCE )

SET(PACKAGING_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Templates/CPack.GenericDescription.txt" CACHE INTERNAL "Where to find the package description file." FORCE )
SET(PACKAGING_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt" CACHE INTERNAL "Where to find the readme file." FORCE )
SET(PACKAGING_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt" CACHE INTERNAL "Where to find the welcome file." FORCE )

# TODO: need release notes
#SET(PACKAGING_RELEASENOTES "${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes.txt" CACHE INTERNAL "Where to find the releasenotes file." FORCE )

SET(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\scirun5_256.png" CACHE INTERNAL "Packing icon." FORCE )

MESSAGE(STATUS "Configuring SCIRun packaging")

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME}")
SET(CPACK_PACKAGE_VENDOR "${APPLICATION_VENDOR}")

# TODO: actually used? looks like CPACK_RESOURCE_FILE_README gets used in OS X installer
# check NSIS installer once it exists...
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PACKAGING_DESCRIPTION})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}.${SCIRUN_VERSION_PATCH} is a problem solving environment.")

SET(CPACK_RESOURCE_FILE_LICENSE ${PACKAGING_LICENSE})
SET(CPACK_RESOURCE_FILE_README ${PACKAGING_README})
SET(CPACK_RESOURCE_FILE_WELCOME ${PACKAGING_WELCOME})

SET(CPACK_PACKAGE_VERSION_MAJOR "${SCIRUN_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SCIRUN_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SCIRUN_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


# other platform settings will be built up from here
IF(APPLE)

 SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications/")
 SET(CPACK_TOPLEVEL_TAG "")

 IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
   SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
 ENDIF()

 SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
 SET(CPACK_STRIP_FILES TRUE)
 SET(CPACK_SOURCE_STRIP_FILES TRUE)
 SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}.app" "${APPLICATION_NAME}")

ELSEIF(WIN32 AND NOT UNIX)

  # There is a bug in NSIS that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  SET(CPACK_PACKAGE_ICON "${PACKAGING_ICON}")

  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

  IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  ELSE(CMAKE_SIZEOF_VOID_P MATCHES 8)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES 8)

  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${APPLICATION_NAME}")
  SET(CPACK_NSIS_MUI_ICON "${APPLICATION_WINDOWS_ICON}")
  SET(CPACK_NSIS_DISPLAY_NAME "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_CONTACT ${APPLICATION_CONTACT} )
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        WriteRegStr HKCR '${APPLICATION_EXTENSION}' '' '${APPLICATION_NAME}'
        WriteRegStr HKCR '${APPLICATION_NAME}' '' '${APPLICATION_NAME} Network File'
        WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell' '' 'open'
        WriteRegStr HKCR '${APPLICATION_NAME}\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe,0'
        WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell\\\\open\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
        WriteRegStr HKCR '${APPLICATION_NAME}\\\\shell\\\\edit\\\\command' '' '$INSTDIR\\\\bin\\\\${APPLICATION_NAME}.exe \\\"%1\\\"'
        System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
  ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        DeleteRegKey HKCR '${APPLICATION_EXTENSION}'
        DeleteRegKey HKCR '${APPLICATION_NAME}'
  ")

ENDIF()

INCLUDE(CPack)

########################################################################
#GCov

OPTION(ENABLE_GCOV_DATA_FILES "Creates .gcda and .gcno files for coverage" OFF)
MARK_AS_ADVANCED(ENABLE_GCOV_DATAFILES)

IF(ENABLE_GCOV_DATA_FILES)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()
