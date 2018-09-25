cmake_minimum_required(VERSION 2.8.8)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project(googletest-distribution)
set(GOOGLETEST_VERSION 1.9.0)

enable_testing()

include(CMakeDependentOption)
include(GNUInstallDirs)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
else()
  add_subdirectory( googletest )
endif()

#############
# CIBC part #
#############

SET(SCI_GTEST_INCLUDE
  ${CMAKE_SOURCE_DIR}/googletest/include
  ${CMAKE_SOURCE_DIR}/googlemock/include
  )
#SET(SCI_GTEST_LIBRARY qwt CACHE INTERNAL "Qwt library" FORCE)
#SET(SCI_GMOCK_LIBRARY qwt CACHE INTERNAL "Qwt library" FORCE)

#set(SCI_QWT_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
#set(GTEST_USE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/UseGoogleTest.cmake")

#CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/CMake/GoogleTestConfig.cmake.in"
#  "${CMAKE_BINARY_DIR}/GoogleTestConfig.cmake" @ONLY IMMEDIATE)
#EXPORT(TARGETS ${SCI_GTEST_LIBRARY} FILE QwtExports.cmake)
