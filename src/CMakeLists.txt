#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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
#


IF(UNIX)
  # temporary - needed to make Travis CI build work
  # TODO: revisit and delete when CMake 2.8.11 or greater is available
  # or switch to another CI tool
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
ELSE()
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)
ENDIF()

IF(POLICY CMP0042)
  # TODO: when support for versions older than 2.8.12 is dropped,
  # enable new policy and get rid of version check
  #
  # see https://cmake.org/cmake/help/v3.0/policy/CMP0042.html
  CMAKE_POLICY(SET CMP0042 OLD)
ENDIF()
IF(POLICY CMP0043)
  CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()
IF(POLICY CMP0049)
  CMAKE_POLICY(SET CMP0049 NEW)
ENDIF()

PROJECT(SCIRun)

IF(GENERATE_COMPILATION_DATABASE)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ENDIF()

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

########################################################################
# Build shared libraries by default

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)

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

IF(VERSION_TAG MATCHES "HEAD-HASH-NOTFOUND")
  FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/LATEST_TAG.txt VERSION_TAG)
  STRING(STRIP ${VERSION_TAG} VERSION_TAG)
  MESSAGE(STATUS "Found tag file for version info: " "${VERSION_TAG}")
ENDIF()

MESSAGE(STATUS "Git version tag: " "${VERSION_TAG}")
MESSAGE(STATUS "Git version branch: " "${VERSION_BRANCH}")
MESSAGE(STATUS "Git version commit hash: " "${VERSION_HASHVAR}")

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION_TAG}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION_TAG}")
#TODO: make generic
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\-(beta.[A-Za-z0-9_]+)\\-?.*" "\\1" VERSION_PATCH "${VERSION_TAG}")
#TODO: parse SHA
#string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
#set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

#MESSAGE(STATUS "Git version major: " "${VERSION_MAJOR}")
#MESSAGE(STATUS "Git version minor: " "${VERSION_MINOR}")
#MESSAGE(STATUS "Git version patch name: " "${VERSION_PATCH}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/Version.cc.in
 ${CMAKE_CURRENT_BINARY_DIR}/Core/Application/Version.cc)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/Core/Application/Version.cc")


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

SET(APPLICATION_WINDOWS_RC "${CMAKE_SOURCE_DIR}/Resources/Icons/SCIRun.rc" CACHE INTERNAL "Windows icon" FORCE)
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
# Configure Spdlog
set (cxxfeaturelist ${CMAKE_CXX_COMPILE_FEATURES})

list (FIND cxxfeaturelist "cxx_thread_local" _index)
if (${_index} GREATER -1)
  #message("Found cxx_thread_local")
else()
  message(WARNING "Did NOT find cxx_thread_local--turning off feature of spdlog")
  add_definitions(-DDISABLE_SPDLOG_TLS=1)
endif()

#########################################################################
# External projects

MACRO(CONFIG_STANDARD_EXTERNAL name config_file directory)
  FIND_PACKAGE(${name} CONFIGS ${config_file} HINTS ${directory} NO_SYSTEM_ENVIRONMENT_PATH)
  IF(NOT ${name}_FOUND)
    MESSAGE(FATAL_ERROR "${name} library not found in ${directory}")
  ENDIF()
ENDMACRO()

CONFIG_STANDARD_EXTERNAL( Zlib ZlibConfig.cmake ${Zlib_DIR} )
INCLUDE(${ZLIB_USE_FILE})

CONFIG_STANDARD_EXTERNAL( LibPNG LibPNGConfig.cmake ${LibPNG_DIR} )
INCLUDE(${PNG_USE_FILE})

CONFIG_STANDARD_EXTERNAL( SQLite SQLiteConfig.cmake ${SQLite_DIR} )
INCLUDE(${SQLITE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Teem TeemConfig.cmake ${Teem_DIR} )
INCLUDE(${Teem_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Freetype FreetypeConfig.cmake ${Freetype_DIR} )
INCLUDE(${FREETYPE_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Cleaver2 Cleaver2Config.cmake ${CLEAVER2_DIR} )
INCLUDE(${CLEAVER2_USE_FILE})

IF(BUILD_WITH_PYTHON)
  CONFIG_STANDARD_EXTERNAL( Python PythonConfig.cmake ${Python_DIR} )
  INCLUDE(${SCI_PYTHON_USE_FILE})
ENDIF()

CONFIG_STANDARD_EXTERNAL( Boost BoostConfig.cmake ${Boost_DIR} )
INCLUDE(${SCI_BOOST_USE_FILE})

CONFIG_STANDARD_EXTERNAL( Tny TnyConfig.cmake ${TNY_DIR} )
INCLUDE(${TNY_USE_FILE})

CONFIG_STANDARD_EXTERNAL( LodePng LodePngConfig.cmake ${LODEPNG_DIR} )
INCLUDE(${LODEPNG_USE_FILE})

IF(WITH_TETGEN)
  CONFIG_STANDARD_EXTERNAL( Tetgen TetgenConfig.cmake ${Tetgen_DIR} )
  INCLUDE(${TETGEN_USE_FILE})
ENDIF()

IF(WITH_OSPRAY)
  set(ospray_DIR ${Ospray_External_Dir}/install/ospray/lib/cmake/ospray-2.0.1)
  find_package(ospray 2.0.0 REQUIRED HINTS ${Ospray_External_Dir}/install/ospray)
ENDIF()

IF(WIN32)
  CONFIG_STANDARD_EXTERNAL( Glew GlewConfig.cmake ${GLEW_DIR} )
  INCLUDE(${GLEW_USE_FILE})
ENDIF()

IF(NOT BUILD_HEADLESS)
  CONFIG_STANDARD_EXTERNAL( Qwt QwtConfig.cmake ${QWT_DIR} )
  INCLUDE(${QWT_USE_FILE})
ENDIF()

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

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1024")
  endif()

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

OPTION(BUILD_BUNDLE "Build Mac bundle" FALSE)
MARK_AS_ADVANCED(BUILD_BUNDLE)

########################################################################
# Find and configure Awk & Sed  {(OS X only), needed to build App bundle

# should be installed by default, searching system paths only
IF(APPLE)
  FIND_PROGRAM(MACDEPLOYQT
    HINTS ${Qt5_PATH} ${Qt5_PATH}/bin
    NAMES macdeployqt
    NO_DEFAULT_PATH
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
# Configure ospray module build

IF(WITH_OSPRAY)
  ADD_DEFINITIONS(-DWITH_OSPRAY)
  SET(OSPRAY_LIBRARIES ospray)
ENDIF()

########################################################################
# Detailed renderer logging

OPTION(RENDERER_TRACE_ON "Enable detailed renderer logging." OFF)
MARK_AS_ADVANCED(RENDERER_TRACE_ON)
IF(RENDERER_TRACE_ON)
  ADD_DEFINITIONS(-DRENDERER_TRACE_ON)
ENDIF()

########################################################################
# Copy Spire-SCIRun specific assets and shaders

set(OUTPUT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
set(OUTPUT_FONT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Fonts)
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

function (CopyFonts src_font_dir dest_font_dir)
  if (NOT dest_font_dir STREQUAL "")
    file(COPY ${src_font_dir}/ DESTINATION ${dest_font_dir}
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

if (DEFINED OUTPUT_FONT_DIR)
  CopyFonts(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/fonts ${OUTPUT_FONT_DIR})
else()
  message(WARNING "Not copying font files. OUTPUT_FONT_DIR variable not found.")
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

IF(NOT BUILD_HEADLESS)
  FIND_PACKAGE(Qt5 COMPONENTS Core Gui Widgets Network OpenGL Concurrent PrintSupport Svg REQUIRED HINTS ${Qt5_PATH})

  IF(Qt5Core_FOUND)
    MESSAGE(STATUS "Found Qt version: ${Qt5Core_VERSION_STRING}")
    ADD_DEFINITIONS(-DQT5_VERSION=${Qt5Core_VERSION_STRING})
    INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
	  INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5OpenGL_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5Concurrent_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5PrintSupport_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${Qt5Svg_INCLUDE_DIRS})
    ADD_DEFINITIONS(${Qt5Core_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5Gui_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5Network_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5OpenGL_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5Concurrent_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5PrintSupport_DEFINITIONS})
    ADD_DEFINITIONS(${Qt5Svg_DEFINITIONS})
    SET(QT_LIBRARIES Qt5::Widgets)
  ELSE()
    MESSAGE(FATAL_ERROR "Qt5 is required for building the SCIRun GUI")
  ENDIF()

  IF(Qt5Core_FOUND)
    INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
    ADD_DEFINITIONS(${QT_DEFINITIONS})
  ELSE()
    MESSAGE(FATAL_ERROR "QT 5.13 or greater is required to build SCIRun")
  ENDIF()

  ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

  MACRO(QT_WRAP_UI arg1 arg2)
    QT5_WRAP_UI(${arg1} ${arg2})
  ENDMACRO(QT_WRAP_UI)

  MACRO(QT_WRAP_CPP arg1 arg2)
    QT5_WRAP_CPP(${arg1} ${arg2})
  ENDMACRO(QT_WRAP_CPP)

  MACRO(QT_ADD_RESOURCES arg1 arg2)
    QT5_ADD_RESOURCES(${arg1} ${arg2})
  ENDMACRO(QT_ADD_RESOURCES)

ENDIF()

IF(TRAVIS_BUILD OR Qt5Widgets_VERSION VERSION_LESS 5.13.0)
  ADD_DEFINITIONS(-DOLDER_QT_SUPPORT_NEEDED)
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
  ${SCIRun_SOURCE_DIR}/Externals/libxml2/include
  ${SCIRun_SOURCE_DIR}/Externals/spire/
  ${SCIRun_SOURCE_DIR}/Externals/submodules/googletest/googletest/include
  ${SCIRun_SOURCE_DIR}/Externals/submodules/googletest/googlemock/include
  ${CMAKE_CURRENT_BINARY_DIR}/Externals/libxml2/include
  ${SCIRun_SOURCE_DIR}/Externals/
  ${SCIRun_BINARY_DIR}/
  ${SCIRun_BINARY_DIR}/include
  ${SCI_TEEM_INCLUDE}
  ${SCI_ZLIB_INCLUDE}
  ${GLM_DIR}
  ${SPDLOG_DIR}/include
  ${Eigen_DIR}
  ${SCI_TNY_INCLUDE}
  ${SCI_LODEPNG_INCLUDE}
  ${SCI_QWT_INCLUDE}
  ${SCI_CLEAVER2_INCLUDE}
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
  TARGET_COMPILE_DEFINITIONS(${test} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
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
IF(NOT BUILD_HEADLESS)
  ADD_SUBDIRECTORY(Interface)
ELSE()
  ADD_DEFINITIONS(-DBUILD_HEADLESS=1)
ENDIF()
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Graphics)

ADD_SUBDIRECTORY(Main)


ADD_SUBDIRECTORY(Testing)

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
SET_PROPERTY(TARGET Algorithms_Legacy_Inverse         PROPERTY FOLDER "Core/Algorithms")
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
SET_PROPERTY(TARGET Core_Thread         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ImportExport         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_IEPlugin         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Geometry_Primitives         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Preferences         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Session         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Utils         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Math         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Matlab         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Parser         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_XMLUtil         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Exceptions_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Util_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Containers         PROPERTY FOLDER "Core")
#SET_PROPERTY(TARGET Core_Thread_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Serialization_Network         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Core_Serialization_Network_Importer         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET bundle_toolkit         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Dataflow_State         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET ${SPIRE_LIBRARY_NAME} PROPERTY FOLDER "Graphics")

IF(NOT BUILD_HEADLESS)
  SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_FiniteElements   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_BrainStimulator   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Fields   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Bundle   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Matlab   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Forward   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Inverse   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Teem   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")
ENDIF()

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Legacy_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Fields   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Legacy_Bundle   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_BrainStimulator   PROPERTY FOLDER "Modules/Brain")
SET_PROPERTY(TARGET Modules_Legacy_FiniteElements   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_Legacy_Fields PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Legacy_Converters PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Legacy_Forward PROPERTY FOLDER "Modules/Forward")
SET_PROPERTY(TARGET Modules_Legacy_String PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Legacy_Inverse PROPERTY FOLDER "Modules/Inverse")
SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Legacy_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Legacy_Teem_DataIO   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Misc   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Converters   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

SET_PROPERTY(TARGET Graphics_Glyphs   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Widgets   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Datatypes   PROPERTY FOLDER "Graphics")

IF(BUILD_TESTING OR TRAVIS_BUILD)
  SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gmock_main   PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest_main   PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_Utils PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_ModuleTestBase PROPERTY FOLDER "Testing Support")
ENDIF()

IF(BUILD_TESTING)

  #SET_PROPERTY(TARGET SCIRunTestData_external   PROPERTY FOLDER "Testing Support")

  SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_BrainStimulator_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Field_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Describe_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_FiniteElements_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithm_Layer_Test   PROPERTY FOLDER "Core/Algorithms/Tests")
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
  SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
  SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
  SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
  SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO_Tests PROPERTY FOLDER "Modules/DataIO")
  SET_PROPERTY(TARGET Modules_Factory_Tests   PROPERTY FOLDER "Modules/Basic")

  IF(NOT BUILD_HEADLESS)
    SET_PROPERTY(TARGET Interface_Modules_Render_Tests PROPERTY FOLDER "Interface/Tests")
	SET_PROPERTY(TARGET Modules_Dialog_Factory_Tests PROPERTY FOLDER "Interface/Tests")
  ENDIF()
  IF(BUILD_WITH_PYTHON)
    SET_PROPERTY(TARGET Core_Python_Tests     PROPERTY FOLDER "Core/Tests")
    SET_PROPERTY(TARGET Engine_Python_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
	SET_PROPERTY(TARGET Modules_Python_Tests   PROPERTY FOLDER "Modules/Python")
  ENDIF()
ENDIF()

SET_PROPERTY(TARGET cleaver PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET xml2 PROPERTY FOLDER "Externals")

IF(BUILD_WITH_PYTHON)
  SET_PROPERTY(TARGET Core_Python         PROPERTY FOLDER "Core")
  SET_PROPERTY(TARGET SCIRunPythonAPI PROPERTY FOLDER "Dataflow/Engine")
  SET_PROPERTY(TARGET Modules_Python   PROPERTY FOLDER "Modules/Python")
  IF(NOT BUILD_HEADLESS)
    SET_PROPERTY(TARGET Interface_Modules_Python   PROPERTY FOLDER "Interface/Modules")
  ENDIF()
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
SET(CPACK_PACKAGE_VERSION "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}.${SCIRUN_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${APPLICATION_NAME}_${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


# other platform settings will be built up from here
IF(APPLE)
  SET(CPACK_GENERATOR "productbuild;TGZ")
  SET(CPACK_TOPLEVEL_TAG "")
  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
   SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  ENDIF()
  # productbuild packaging repair hack
  # call repair_package.sh after running package target in build
  # Tip: can inspect packages using the pkgutil --expand command
  IF(CPACK_GENERATOR MATCHES "productbuild")
    # make sure there are paths to packaging tools
    FIND_PROGRAM(PKGBUILD_EXE pkgbuild)
    FIND_PROGRAM(PRODUCTBUILD_EXE productbuild)
    FIND_PROGRAM(PKGUTIL_EXE pkgutil)
    SET(PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Darwin")
    SET(PKG_BUILD_DIR "${CMAKE_BINARY_DIR}/_CPack_Packages/productbuild/${PKG_FILE_NAME}")
    SET(PKG_ROOT_DIR "${PKG_BUILD_DIR}/Applications")

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Modules/CPack.Distribution.dist.in
                   ${CMAKE_BINARY_DIR}/CPack.Distribution.dist)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/scripts/repair_package.sh.in
                   ${CMAKE_BINARY_DIR}/repair_package.sh)
  ENDIF()
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
