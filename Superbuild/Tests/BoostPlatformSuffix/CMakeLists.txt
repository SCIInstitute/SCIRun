# Regression test for the Boost PLATFORM_SUFFIX architecture detection in
# Superbuild/UseBoost.cmake.
#
# UseBoost.cmake historically probed the CPU architecture with the Apple-only
# "-arch arm64" compiler flag. GCC/Clang on Linux reject that flag, so on
# Linux/aarch64 the probe failed and the build fell back to the "x64" suffix --
# baking the wrong architecture into the expected Boost library filenames.
# See the aarch64/GB10 bug report.
#
# This test includes the REAL UseBoost.cmake (SCI_BOOST_LIBRARY is left unset,
# so its per-library foreach is a no-op) and asserts that PLATFORM_SUFFIX
# matches the host architecture. It goes red on aarch64 today and green once
# the detection is fixed; it stays green on x86_64.

cmake_minimum_required(VERSION 3.10)
project(BoostPlatformSuffixTest C CXX)

include(CheckCCompilerFlag)

# Path to the file under test: <repo>/Superbuild/UseBoost.cmake
include(${CMAKE_CURRENT_SOURCE_DIR}/../../UseBoost.cmake)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(EXPECTED_SUFFIX "a64")
else()
  set(EXPECTED_SUFFIX "x64")
endif()

message(STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "PLATFORM_SUFFIX        = ${PLATFORM_SUFFIX}")
message(STATUS "EXPECTED_SUFFIX        = ${EXPECTED_SUFFIX}")

if(NOT PLATFORM_SUFFIX STREQUAL EXPECTED_SUFFIX)
  message(FATAL_ERROR
    "Boost PLATFORM_SUFFIX mismatch: got '${PLATFORM_SUFFIX}', "
    "expected '${EXPECTED_SUFFIX}' for CMAKE_SYSTEM_PROCESSOR="
    "'${CMAKE_SYSTEM_PROCESSOR}'. UseBoost.cmake misdetected the CPU "
    "architecture.")
endif()

message(STATUS "PASS: Boost PLATFORM_SUFFIX matches host architecture.")
