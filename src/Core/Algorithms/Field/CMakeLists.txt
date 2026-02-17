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

# ---- Algorithms_Field ----

set(Algorithms_Field_SRCS
  ReportFieldInfoAlgorithm.cc
  InterfaceWithCleaver2Algorithm.cc
  CalculateNodeLocationFrequencyAlgorithm.cc
  RefineTetMeshLocallyAlgorithm.cc
)

set(Algorithms_Field_HEADERS
  ReportFieldInfoAlgorithm.h
  InterfaceWithCleaver2Algorithm.h
  CalculateNodeLocationFrequencyAlgorithm.h
  RefineTetMeshLocallyAlgorithm.h
  share.h
)

SCIRUN_ADD_LIBRARY(Algorithms_Field
  ${Algorithms_Field_HEADERS}
  ${Algorithms_Field_SRCS}
)

# Keep original shared define behavior
target_compile_definitions(Algorithms_Field
  PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:BUILD_Algorithms_Field>
)

# -----------------------------------------------------------------------------------
# Cleaver2 consumption via superbuild-exported paths (no find_package needed)
# Expected variables (from superbuild -> top-level normalization):
#   CLEAVER2_INCLUDE       - parent that contains 'cleaver2/vec3.h' (e.g. .../Install/Cleaver2_external/include)
#   CLEAVER2_LIBRARY_DIR   - directory with the Cleaver2 library file
#   (Optional aliases: SCI_CLEAVER2_INCLUDE, SCI_CLEAVER2_LIBRARY_DIR)
# -----------------------------------------------------------------------------------

# Prefer normalized aliases if defined
if(DEFINED SCI_CLEAVER2_INCLUDE)
  set(_c2_inc "${SCI_CLEAVER2_INCLUDE}")
else()
  set(_c2_inc "${CLEAVER2_INCLUDE}")
endif()
if(DEFINED SCI_CLEAVER2_LIBRARY_DIR)
  set(_c2_libdir "${SCI_CLEAVER2_LIBRARY_DIR}")
else()
  set(_c2_libdir "${CLEAVER2_LIBRARY_DIR}")
endif()

# Validate presence
if(NOT _c2_inc OR NOT EXISTS "${_c2_inc}/cleaver2/vec3.h")
  message(FATAL_ERROR
    "Cleaver2 include directory is missing or invalid.\n"
    "  CLEAVER2_INCLUDE='${CLEAVER2_INCLUDE}' ; SCI_CLEAVER2_INCLUDE='${SCI_CLEAVER2_INCLUDE}'\n"
    "  Expected: '<include>/cleaver2/vec3.h'.")
endif()

if(NOT _c2_libdir OR NOT EXISTS "${_c2_libdir}")
  message(FATAL_ERROR
    "Cleaver2 library directory missing.\n"
    "  CLEAVER2_LIBRARY_DIR='${CLEAVER2_LIBRARY_DIR}' ; SCI_CLEAVER2_LIBRARY_DIR='${SCI_CLEAVER2_LIBRARY_DIR}'")
endif()

# Locate the library file inside the provided lib dir
set(_c2_libpath "")
if(WIN32)
  # Windows import/static lib
  foreach(_cand IN ITEMS "cleaver2.lib" "libcleaver2.lib")
    if(EXISTS "${_c2_libdir}/${_cand}")
      set(_c2_libpath "${_c2_libdir}/${_cand}")
      break()
    endif()
  endforeach()
elseif(APPLE)
  foreach(_cand IN ITEMS "libcleaver2.dylib" "libcleaver2.a")
    if(EXISTS "${_c2_libdir}/${_cand}")
      set(_c2_libpath "${_c2_libdir}/${_cand}")
      break()
    endif()
  endforeach()
else()
  foreach(_cand IN ITEMS "libcleaver2.so" "libcleaver2.a")
    if(EXISTS "${_c2_libdir}/${_cand}")
      set(_c2_libpath "${_c2_libdir}/${_cand}")
      break()
    endif()
  endforeach()
endif()

if(NOT _c2_libpath)
  message(FATAL_ERROR
    "Could not find Cleaver2 library under '${_c2_libdir}'.\n"
    "Expected one of: cleaver2(.lib|.so|.dylib|.a)")
endif()

# Define a private imported target so include usage is target-scoped and modern
add_library(Cleaver2::cleaver2 UNKNOWN IMPORTED)
set_target_properties(Cleaver2::cleaver2 PROPERTIES
  IMPORTED_LOCATION "${_c2_libpath}"
  INTERFACE_INCLUDE_DIRECTORIES "${_c2_inc}"
)

# Link everything
target_link_libraries(Algorithms_Field
  PRIVATE
    Core_Datatypes
    Core_Datatypes_Legacy_Field
    Algorithms_Base
    Core_Algorithms_Legacy_Fields
    ${SCI_BOOST_LIBRARY}
    Cleaver2::cleaver2
)

# Tests
SCIRUN_ADD_TEST_DIR(Tests)