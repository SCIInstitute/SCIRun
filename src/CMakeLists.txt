#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9 FATAL_ERROR)

PROJECT(SCIRun5Prototype)

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
# SCIRUN version number.

SET(SCIRUN_VERSION_MAJOR "5")
SET(SCIRUN_VERSION_MINOR "0")
SET(SCIRUN_VERSION_PATCH "0")

SET(SCIRUN_VERSION_STRING "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


########################################################################
# APPLICATION Information (used by CPack)
#
# Based on Seg3D settings...

SET(APPLICATION_NAME "SCIRunGUIPrototype" CACHE INTERNAL "Application name" FORCE)
# TODO: temporary hack to allow testing
SET(TEST_APPLICATION_NAME "SCIRunGUIPrototype_test" CACHE INTERNAL "Application name" FORCE)
# add others?
SET(APPLICATION_EXTENSION ".net" CACHE INTERNAL "Network file extension" FORCE)
SET(APPLICATION_URL "www.scirun.org" CACHE INTERNAL "Project URL" FORCE)
SET(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Vendor information" FORCE)
SET(APPLICATION_CONTACT "scirun-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
SET(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Interface/Application\\\\scirun-icon-large.ico" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Interface/Application/scirun-icon-large.icns" CACHE INTERNAL "OSX icon" FORCE)
SET(APPLICATION_OSX_ICON "scirun-icon-large.icns" CACHE INTERNAL "OSX icon" FORCE)

########################################################################
# Standard tests

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(ExternalProject)


########################################################################
# Configure test support

INCLUDE( CTest )

# ON by default
IF(BUILD_TESTING)
  ENABLE_TESTING()

  # Hack BUILDNAME to differentiate from regular SCIRun builds
  # TODO: remove when transitioning to production code
  SET(BUILDNAME "${BUILDNAME_SCIRun5Prototype}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)

  OPTION(RUN_UNIT_TESTS "Run gtest unit tests" ON)
  OPTION(RUN_BASIC_REGRESSION_TESTS "Run basic regression tests" ON)
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

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LANGUAGE_STANDARD_FLAG}")
ENDIF()

IF(APPLE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
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
# Find and configure Awk & Sed  {(OS X only), needed to build app bundle

# should be installed by default, searching system paths only
IF(APPLE)
  FIND_PROGRAM(AWK
    NAMES awk
  )
  MARK_AS_ADVANCED(AWK)

  FIND_PROGRAM(SED
    NAMES sed
  )
  MARK_AS_ADVANCED(SED)
ENDIF()


########################################################################
# Configure Boost external project

# TODO: make macro

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB=1)

SET(boost_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/boost")
SET(boost_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/boost")

SET(SCI_BOOST_INCLUDE ${boost_SOURCE_DIR} CACHE INTERNAL "Boost include directories." FORCE)
SET(SCI_BOOST_LIBRARY boost CACHE INTERNAL "Boost library name." FORCE)

IF(WIN32)
#  # See cmake-2.8/Modules/FindBoost.cmake
#  #SET(boost_LIB_PREFIX "lib")
  SET(boost_LIB_PREFIX "")
ELSE()
  SET(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
ENDIF()


SET(boost_LIBRARY_PATH "${boost_BINARY_DIR}/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
# other configuration options are RELWITHDEBINFO and MINSIZEREL
SET(boost_LIBRARY_PATH_RELEASE "${boost_BINARY_DIR}/Release/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
SET(boost_LIBRARY_PATH_DEBUG "${boost_BINARY_DIR}/Debug/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")


# TODO: fix install step
#
# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Boost_external
  PREFIX ${boost_BINARY_DIR}
  GIT_REPOSITORY "https://github.com/SCIInstitute/boost.git"
  GIT_TAG "origin/master"
  SOURCE_DIR ${boost_SOURCE_DIR}
  BINARY_DIR ${boost_BINARY_DIR}
  DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/boost"
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${boost_BINARY_DIR}"
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
)

ADD_LIBRARY(${SCI_BOOST_LIBRARY} STATIC IMPORTED GLOBAL)

# adding Boost as a build target and dependency
#
# TODO: how to make boost include dependent on Boost_external?
ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} Boost_external)
ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} ${SCI_BOOST_INCLUDE})

IF (CMAKE_GENERATOR MATCHES "Makefiles")
  SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
    PROPERTIES
      IMPORTED_LOCATION ${boost_LIBRARY_PATH}
  )
ELSE() # IDEs: Xcode, VS, others...
  SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
    PROPERTIES
      IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
      IMPORTED_LOCATION_RELEASE ${boost_LIBRARY_PATH_RELEASE}
      IMPORTED_LOCATION_DEBUG ${boost_LIBRARY_PATH_DEBUG}
  )
ENDIF()

INCLUDE_DIRECTORIES(${SCI_BOOST_INCLUDE})

########################################################################
# Find and configure Qt

# TODO: exclude for headless
SET(QT_MIN_VERSION "4.8.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(QT_FOUND)
  # TODO: configure Qt OpenGL?
  # TODO: may want to grab Seg3D's custom FindQt4 module and UseQt4 file if
  # build issues haven't been corrected in CMake 2.8.7 and newer

  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
  INCLUDE(${QT_USE_FILE})
  ADD_DEFINITIONS(${QT_DEFINITIONS})
ELSE()
  MESSAGE(FATAL_ERROR "QT 4.8 or greater is required to build SCIRun5Prototype")
ENDIF()

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)


########################################################################
# Includes

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
  ${SCIRun5Prototype_SOURCE_DIR}/
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/gtest/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/include
  ${SCI_BOOST_INCLUDE}
  ${SCIRun5Prototype_BINARY_DIR}/
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/eigen_3_1_1
)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

########################################################################
# Macros

MACRO(SCIRUN_ADD_UNIT_TEST test)
  ADD_EXECUTABLE(${test} ${ARGN})
  LIST(APPEND tmp ${test} ${UNIT_TEST_LIST})
  SET(UNIT_TEST_LIST ${tmp} CACHE INTERNAL "List of unit tests." FORCE)
ENDMACRO(SCIRUN_ADD_UNIT_TEST)


########################################################################
# Output directories.

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR} CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY)

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Externals)

ADD_SUBDIRECTORY(Main)

ADD_SUBDIRECTORY(Testing)


########################################################################
# Organize projects into folders for Visual Studio projects

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET Algorithms_Base         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")

SET_PROPERTY(TARGET Dataflow_Network         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Dataflow_Network_Tests   PROPERTY FOLDER "Dataflow/Tests")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Datatypes_Mesh         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Mesh_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Basis         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Basis_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_CommandLine         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Command         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Command_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Geometry_Primitives         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Geometry_Primitives_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Application         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Utils         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Utils_Tests         PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Math         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Math_Tests         PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Serialization_Network         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Core_Serialization_Network_Tests         PROPERTY FOLDER "Dataflow/Serialization/Tests")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Network_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Scheduler_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
SET_PROPERTY(TARGET Dataflow_State         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Dataflow_State_Tests   PROPERTY FOLDER "Dataflow/Tests")

SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gmock_main   PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest_main   PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET Testing_Utils PROPERTY FOLDER "Testing Support")

SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Testing   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Fields   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Fields   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Fields_Tests   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

########################################################################
# Custom CMake settings for CPack

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

########################################################################
# CPack


########################################################################
# PACKAGING Information

SET(PACKAGING_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" CACHE INTERNAL "Where to find the license file." FORCE )

SET(PACKAGING_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt" CACHE INTERNAL "Where to find the readme file." FORCE )
SET(PACKAGING_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt" CACHE INTERNAL "Where to find the welcome file." FORCE )

# TODO: need release notes
#SET(PACKAGING_RELEASENOTES "${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes.txt" CACHE INTERNAL "Where to find the releasenotes file." FORCE )

SET(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\package_icon.bmp" CACHE INTERNAL "Packing icon." FORCE )

MESSAGE(STATUS "Configuring SCIRun packaging")

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME}")
SET(CPACK_PACKAGE_VENDOR "${APPLICATION_VENDOR}")
      
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PACKAGING_README} )
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}.${SCIRUN_VERSION_PATCH} is a problem solving environment.")

SET(CPACK_RESOURCE_FILE_LICENSE ${PACKAGING_LICENSE})
SET(CPACK_RESOURCE_FILE_README ${PACKAGING_RELEASENOTES})
SET(CPACK_RESOURCE_FILE_WELCOME ${PACKAGING_WELCOME})

SET(CPACK_PACKAGE_VERSION_MAJOR "${SCIRUN_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SCIRUN_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SCIRUN_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


# other platform settings will be built up from here
IF(APPLE)
 SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications/")
 SET(CPACK_TOPLEVEL_TAG "")
 IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
   SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
 ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
 SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
 SET(CPACK_STRIP_FILES TRUE)
 SET(CPACK_SOURCE_STRIP_FILES TRUE)
 SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}.app" "${APPLICATION_NAME}")
ENDIF()

INCLUDE(CPack)
