# LodePNGWrapper_install.cmake
# Manual install for the wrapper (avoids cmake_install.cmake path issues).

foreach(req IN ITEMS WRAPPER_BUILD_DIR WRAPPER_SOURCE_DIR LODEPNG_INSTALL_DIR CONFIGURATION)
  if(NOT DEFINED ${req})
    message(FATAL_ERROR "Missing required variable: ${req}")
  endif()
endforeach()

# Normalize inputs
foreach(var IN ITEMS WRAPPER_BUILD_DIR WRAPPER_SOURCE_DIR LODEPNG_INSTALL_DIR CONFIGURATION)
  string(REGEX REPLACE "^\"|\"$" "" ${var} "${${var}}")
  file(TO_CMAKE_PATH "${${var}}" ${var})
endforeach()

# 1) Headers: copy whichever layout exists into <install>/include/lodepng/lodepng.h
set(_hdr1 "${WRAPPER_SOURCE_DIR}/lodepng/lodepng.h")
set(_hdr2 "${WRAPPER_SOURCE_DIR}/lodepng.h")
set(_hdr  "")
if(EXISTS "${_hdr1}")
  set(_hdr "${_hdr1}")
elseif(EXISTS "${_hdr2}")
  set(_hdr "${_hdr2}")
endif()

if(_hdr)
  file(MAKE_DIRECTORY "${LODEPNG_INSTALL_DIR}/include/lodepng")
  file(COPY "${_hdr}" DESTINATION "${LODEPNG_INSTALL_DIR}/include/lodepng")
  message(STATUS "[LodePNGManualInstall] Copied header: ${_hdr} -> ${LODEPNG_INSTALL_DIR}/include/lodepng/")
else()
  message(WARNING "[LodePNGManualInstall] No lodepng.h found under: ${WRAPPER_SOURCE_DIR}")
endif()

# 2) Library: choose name by configuration and copy to <install>/lib
if(CONFIGURATION STREQUAL "Debug")
  set(_libname "lodepngd.lib")
else()
  set(_libname "lodepng.lib")
endif()

# VS output typically: <build>/<Config>/<name>.lib
set(_libsrc "${WRAPPER_BUILD_DIR}/${CONFIGURATION}/${_libname}")
file(MAKE_DIRECTORY "${LODEPNG_INSTALL_DIR}/lib")

if(EXISTS "${_libsrc}")
  file(COPY "${_libsrc}" DESTINATION "${LODEPNG_INSTALL_DIR}/lib")
  message(STATUS "[LodePNGManualInstall] Copied lib: ${_libsrc} -> ${LODEPNG_INSTALL_DIR}/lib/")
else()
  # Fallback: try without config subdir (just in case)
  set(_alt "${WRAPPER_BUILD_DIR}/${_libname}")
  if(EXISTS "${_alt}")
    file(COPY "${_alt}" DESTINATION "${LODEPNG_INSTALL_DIR}/lib")
    message(STATUS "[LodePNGManualInstall] Copied lib (alt): ${_alt} -> ${LODEPNG_INSTALL_DIR}/lib/")
  else()
    message(FATAL_ERROR "[LodePNGManualInstall] Built library not found.\n  Tried:\n    ${_libsrc}\n    ${_alt}")
  endif()
endif()