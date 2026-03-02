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
# Dataflow/Engine/Network
# =========================

set(Engine_Network_SRCS
  DynamicPortManager.cc
  NetworkEditorController.cc
  NetworkCommands.cc
  ProvenanceItem.cc
  ProvenanceItemFactory.cc
  ProvenanceItemImpl.cc
  ProvenanceManager.cc
  PythonImpl.cc
)

set(Engine_Network_HEADERS
  ControllerInterfaces.h
  DynamicPortManager.h
  NetworkEditorController.h
  NetworkCommands.h
  ProvenanceItem.h
  ProvenanceItemFactory.h
  ProvenanceItemImpl.h
  ProvenanceManager.h
  PythonImpl.h
  share.h
)

SCIRUN_ADD_LIBRARY(Engine_Network
  ${Engine_Network_HEADERS}
  ${Engine_Network_SRCS}
)

set(Engine_Network_NonPythonDependentLibs
  Dataflow_Network
  Core_Serialization_Network
  Core_Command
  Engine_Scheduler
  Core_Thread
  Core_Matlab
)

# --------------------------------------------------------------------
# Python includes (needed for Boost.Python headers -> pyconfig.h on Win)
# --------------------------------------------------------------------
if (BUILD_WITH_PYTHON)
  set(_PY_INCLUDES "")

  if (DEFINED PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_INCLUDE_DIR}")
  endif()
  if (DEFINED PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_PC_INCLUDE_DIR}")
  endif()

  # Fallback to PCbuild layout if cache vars aren’t present
  if (MSVC AND NOT _PY_INCLUDES)
    set(_py_src "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external")
    if (EXISTS "${_py_src}/Include")
      list(APPEND _PY_INCLUDES "${_py_src}/Include")
    endif()
    if (EXISTS "${_py_src}/PC")
      list(APPEND _PY_INCLUDES "${_py_src}/PC")
    endif()
  endif()

  if (_PY_INCLUDES)
    list(REMOVE_DUPLICATES _PY_INCLUDES)
    target_include_directories(Engine_Network PRIVATE ${_PY_INCLUDES})
    message(STATUS "[Engine/Network] Added Python includes: ${_PY_INCLUDES}")
  else()
    message(WARNING "[Engine/Network] Python includes not found at configure time; Boost.Python compilation may fail.")
  endif()
endif()

# ---------------------------------------------------------
# Link libraries (use targets, not file paths; keep PRIVATE)
# ---------------------------------------------------------
if (BUILD_WITH_PYTHON)
  target_link_libraries(Engine_Network
    PRIVATE
      ${Engine_Network_NonPythonDependentLibs}
      # Python bits
      Core_Python         # SCIRun's Python core (brings Python usage + policies)
      SCIRunPythonAPI     # link to the target; CMake will use the import .lib, not the .pyd
      SCIRunPython        # ensure the CPython import libs (python313[_d].lib) are available
  )
  target_compile_definitions(Engine_Network PRIVATE BOOST_PYTHON_NO_LIB BOOST_PYTHON_STATIC_LIB)
else()
  target_link_libraries(Engine_Network
    PRIVATE
      ${Engine_Network_NonPythonDependentLibs}
  )
endif()

# -------------------------------------------------------------------------
# Debug-only: block autolink of release python313.lib (e.g., Boost.Python)
# -------------------------------------------------------------------------
if (BUILD_WITH_PYTHON)
  # If you want to derive digits programmatically, you can, but 313 is fine here.
  target_link_options(Engine_Network PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
  )
endif()

# -----------------------------------
# DLL build define (preserve behavior)
# -----------------------------------
if (BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Engine_Network)
endif()

SCIRUN_ADD_TEST_DIR(Tests)