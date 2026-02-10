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


# =========================
# Modules/Python CMake file
# =========================

# --- Sources & headers ---
set(Modules_Python_SRCS
  PythonObjectForwarder.cc
  InterfaceWithPython.cc
  PythonInterfaceParser.cc
  ModuleStateModifierTester.cc
  LoopEnd.cc
  LoopStart.cc
  #LoopIncrement.cc
)

set(Modules_Python_HEADERS
  PythonObjectForwarder.h
  InterfaceWithPython.h
  PythonInterfaceParser.h
  ModuleStateModifierTester.h
  LoopEnd.h
  LoopStart.h
  #LoopIncrement.h
  share.h
)

# --- Library target ---
SCIRUN_ADD_LIBRARY(Modules_Python
  ${Modules_Python_HEADERS}
  ${Modules_Python_SRCS}
)

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Modules_Python)
endif()

# ====================================================================================
# Optional Python detection (only if the build is configured with Python support)
# ====================================================================================

set(_SCIRUN_MODULES_PY_OK FALSE)
if(BUILD_WITH_PYTHON)
  # 1) Prefer modern FindPython (generic)
  find_package(Python QUIET COMPONENTS Interpreter Development)
  if(Python_Interpreter_FOUND AND Python_Development_FOUND)
    set(_SCIRUN_MODULES_PY_OK TRUE)
  elseif(NOT _SCIRUN_MODULES_PY_OK)
    # 2) Try Python3 module
    find_package(Python3 QUIET COMPONENTS Interpreter Development)
    if(Python3_Interpreter_FOUND AND Python3_Development_FOUND)
      # Normalize to Python_* names so logic below is uniform
      set(Python_EXECUTABLE   "${Python3_EXECUTABLE}")
      set(Python_INCLUDE_DIRS "${Python3_INCLUDE_DIRS}")
      set(Python_LIBRARIES    "${Python3_LIBRARIES}")
      # Alias Python3::Python → Python::Python for consistent linking
      if(TARGET Python3::Python AND NOT TARGET Python::Python)
        add_library(Python::Python ALIAS Python3::Python)
      endif()
      set(_SCIRUN_MODULES_PY_OK TRUE)
    endif()
  endif()

  # 3) Legacy fallback (older CMake or missing modules)
  if(NOT _SCIRUN_MODULES_PY_OK)
    find_package(PythonInterp REQUIRED)
    find_package(PythonLibs REQUIRED)
    # Normalize legacy vars
    set(Python_EXECUTABLE   "${PYTHON_EXECUTABLE}")
    set(Python_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
    set(Python_LIBRARIES    "${PYTHON_LIBRARIES}")
    # Provide an imported target for consistency
    if(NOT TARGET Python::Python)
      add_library(Python::Python UNKNOWN IMPORTED)
      set_target_properties(Python::Python PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Python_INCLUDE_DIRS}"
      )
      if(WIN32)
        set_target_properties(Python::Python PROPERTIES
          IMPORTED_IMPLIB "${Python_LIBRARIES}"
        )
      else()
        set_target_properties(Python::Python PROPERTIES
          IMPORTED_LOCATION "${Python_LIBRARIES}"
        )
      endif()
    endif()
    set(_SCIRUN_MODULES_PY_OK TRUE)
  endif()

  if(NOT _SCIRUN_MODULES_PY_OK)
    message(FATAL_ERROR "BUILD_WITH_PYTHON is ON, but Python (Interpreter + Development) could not be found.")
  endif()
endif() # BUILD_WITH_PYTHON

# ====================================================================================
# Link dependencies
# ====================================================================================

# Always link these:
target_link_libraries(Modules_Python
  PRIVATE
    Dataflow_Network
    Core_Datatypes
    Algorithms_Base
    Core_Thread
)

# Only link Python-related libs when enabled:
if(BUILD_WITH_PYTHON)
  # Link the Python runtime and your Core_Python layer
  target_link_libraries(Modules_Python
    PRIVATE
      Python::Python
      Core_Python
  )
endif()

# ====================================================================================
# Tests (kept as in your original)
# ====================================================================================

if(BUILD_WITH_PYTHON)
  SCIRUN_ADD_TEST_DIR(Tests)
endif()

# ====================================================================================
# Diagnostics (helpful while bringing up the superbuild)
# ====================================================================================
if(BUILD_WITH_PYTHON)
  message(STATUS "[Modules/Python] Python executable : ${Python_EXECUTABLE}")
  message(STATUS "[Modules/Python] Python include(s) : ${Python_INCLUDE_DIRS}")
  message(STATUS "[Modules/Python] Python libraries  : ${Python_LIBRARIES}")
endif()