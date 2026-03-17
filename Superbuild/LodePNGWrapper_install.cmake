# Cross‑platform manual install script for LodePNG wrapper.

foreach(req IN ITEMS WRAPPER_BUILD_DIR WRAPPER_SOURCE_DIR LODEPNG_INSTALL_DIR CONFIGURATION)
  if(NOT DEFINED ${req})
    message(FATAL_ERROR "Missing required variable: ${req}")
  endif()
endforeach()

# Normalize paths
foreach(var IN ITEMS WRAPPER_BUILD_DIR WRAPPER_SOURCE_DIR LODEPNG_INSTALL_DIR CONFIGURATION)
  string(REGEX REPLACE "^\"|\"$" "" ${var} "${${var}}")
  file(TO_CMAKE_PATH "${${var}}" ${var})
endforeach()

# ---------------------------------------------------------------
# 1) HEADER
# ---------------------------------------------------------------
set(_hdr1 "${WRAPPER_SOURCE_DIR}/lodepng/lodepng.h")
set(_hdr2 "${WRAPPER_SOURCE_DIR}/lodepng.h")
set(_hdr "")

if(EXISTS "${_hdr1}")
  set(_hdr "${_hdr1}")
elseif(EXISTS "${_hdr2}")
  set(_hdr "${_hdr2}")
endif()

if(_hdr)
  file(MAKE_DIRECTORY "${LODEPNG_INSTALL_DIR}/include/lodepng")
  file(COPY "${_hdr}" DESTINATION "${LODEPNG_INSTALL_DIR}/include/lodepng")
  message(STATUS "[LodePNGManualInstall] Copied header: ${_hdr}")
else()
  message(WARNING "[LodePNGManualInstall] No lodepng.h found under: ${WRAPPER_SOURCE_DIR}")
endif()

# ---------------------------------------------------------------
# 2) LIBRARY  (Windows: .lib   macOS/Linux: .a)
# ---------------------------------------------------------------

if(WIN32)
  # Multi‑config: Debug gets lodepngd.lib
  if(CONFIGURATION STREQUAL "Debug")
    set(_libname "lodepngd.lib")
  else()
    set(_libname "lodepng.lib")
  endif()
else()
  # Unix always produces liblodepng.a for static library
  set(_libname "liblodepng.a")
endif()

file(MAKE_DIRECTORY "${LODEPNG_INSTALL_DIR}/lib")

# Try <build>/<config>/<lib>
set(_primary "${WRAPPER_BUILD_DIR}/${CONFIGURATION}/${_libname}")

# Try <build>/<lib> (single-config generators)
set(_alt "${WRAPPER_BUILD_DIR}/${_libname}")

if(EXISTS "${_primary}")
  file(COPY "${_primary}" DESTINATION "${LODEPNG_INSTALL_DIR}/lib")
  message(STATUS "[LodePNGManualInstall] Copied lib: ${_primary}")
elseif(EXISTS "${_alt}")
  file(COPY "${_alt}" DESTINATION "${LODEPNG_INSTALL_DIR}/lib")
  message(STATUS "[LodePNGManualInstall] Copied lib (alt): ${_alt}")
else()
  message(FATAL_ERROR
    "[LodePNGManualInstall] Built library not found.\n"
    "  Tried:\n"
    "    ${_primary}\n"
    "    ${_alt}"
  )
endif()