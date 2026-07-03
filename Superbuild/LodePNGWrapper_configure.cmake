# LodePNGWrapper_configure.cmake
# Executed with `cmake -P` from ExternalProject_Add.
# Creates a minimal wrapper source dir and configures it with the chosen generator.

# Required inputs
foreach(req IN ITEMS WRAPPER_SOURCE_DIR WRAPPER_BUILD_DIR WRAPPER_LIST_FILE LODEPNG_SRC CMAKE_INSTALL_PREFIX CMAKE_GENERATOR)
  if(NOT DEFINED ${req})
    message(FATAL_ERROR "Missing required variable: ${req}")
  endif()
endforeach()

# Sanitize: strip accidental surrounding quotes; normalize to CMake-style paths.
foreach(var IN ITEMS WRAPPER_SOURCE_DIR WRAPPER_BUILD_DIR WRAPPER_LIST_FILE LODEPNG_SRC CMAKE_INSTALL_PREFIX)
  if(DEFINED ${var})
    string(REGEX REPLACE "^\"|\"$" "" ${var} "${${var}}")
    file(TO_CMAKE_PATH "${${var}}" ${var})
  endif()
endforeach()

file(MAKE_DIRECTORY "${WRAPPER_SOURCE_DIR}")
file(MAKE_DIRECTORY "${WRAPPER_BUILD_DIR}")

# Copy the wrapper template to a proper CMakeLists.txt
if(NOT EXISTS "${WRAPPER_LIST_FILE}")
  message(FATAL_ERROR "WRAPPER_LIST_FILE does not exist: '${WRAPPER_LIST_FILE}'")
endif()
file(COPY   "${WRAPPER_LIST_FILE}" DESTINATION "${WRAPPER_SOURCE_DIR}")
file(RENAME "${WRAPPER_SOURCE_DIR}/LodePNGWrapperProject.cmake.in"
            "${WRAPPER_SOURCE_DIR}/CMakeLists.txt")

# Prepare generator args for Visual Studio
set(_GEN_ARGS -G "${CMAKE_GENERATOR}")
if(DEFINED CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
  list(APPEND _GEN_ARGS -A "${CMAKE_GENERATOR_PLATFORM}")
endif()
if(DEFINED CMAKE_GENERATOR_TOOLSET AND NOT CMAKE_GENERATOR_TOOLSET STREQUAL "")
  list(APPEND _GEN_ARGS -T "${CMAKE_GENERATOR_TOOLSET}")
endif()

message(STATUS "[LodePNGWrapper_configure] LODEPNG_SRC='${LODEPNG_SRC}'")
message(STATUS "[LodePNGWrapper_configure] WRAPPER_SOURCE_DIR='${WRAPPER_SOURCE_DIR}'")
message(STATUS "[LodePNGWrapper_configure] WRAPPER_BUILD_DIR='${WRAPPER_BUILD_DIR}'")
message(STATUS "[LodePNGWrapper_configure] CMAKE_INSTALL_PREFIX='${CMAKE_INSTALL_PREFIX}'")
message(STATUS "[LodePNGWrapper_configure] CMAKE_GENERATOR='${CMAKE_GENERATOR}'")
if(DEFINED CMAKE_GENERATOR_PLATFORM)
  message(STATUS "[LodePNGWrapper_configure] CMAKE_GENERATOR_PLATFORM='${CMAKE_GENERATOR_PLATFORM}'")
endif()
if(DEFINED CMAKE_GENERATOR_TOOLSET)
  message(STATUS "[LodePNGWrapper_configure] CMAKE_GENERATOR_TOOLSET='${CMAKE_GENERATOR_TOOLSET}'")
endif()

# Configure the wrapper project
execute_process(
  COMMAND "${CMAKE_COMMAND}" ${_GEN_ARGS}
          -S "${WRAPPER_SOURCE_DIR}"
          -B "${WRAPPER_BUILD_DIR}"
          -D LODEPNG_SRC:PATH="${LODEPNG_SRC}"
          -D CMAKE_INSTALL_PREFIX:PATH="${CMAKE_INSTALL_PREFIX}"
          -D BUILD_SHARED_LIBS:BOOL=OFF
  RESULT_VARIABLE _res
)
if(NOT _res EQUAL 0)
  message(FATAL_ERROR "Failed to configure LodePNG wrapper project (code=${_res}).")
endif()