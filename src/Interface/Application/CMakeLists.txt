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
# Interface_Application
# =========================

# Allow building the GUI app without Python even if global BUILD_WITH_PYTHON=ON
# Set to OFF in the superbuild cache to remove all Python linkage from this app.
option(BUILD_INTERFACE_APPLICATION_WITH_PYTHON "Enable Python console in Interface_Application" ON)

set(Interface_Application_SOURCES
  ClosestPortFinder.cc
  DeveloperConsole.cc
  GuiApplication.cc
  GuiCommandFactory.cc
  GuiCommands.cc
  GuiLogger.cc
  MainWindowCollaborators.cc
  ModuleWidget.cc
  ModuleProxyWidget.cc
  NetworkExecutionProgressBar.cc
  ProvenanceWindow.cc
  SCIRunMainWindow.cc
  SCIRunMainWindowSlotsPublic.cc
  SCIRunMainWindowSlotsPrivate.cc
  SCIRunMainWindowQtOverrides.cc
  SCIRunMainWindowCommandInterface.cc
  SCIRunMainWindowTreeFunctions.cc
  SCIRunMainWindowSetup.cc
  Note.cc
  NoteEditor.cc
  Port.cc
  PreferencesWindow.cc
  Settings.cc
  ShortcutsInterface.cc
  TagManagerWindow.cc
  Connection.cc
  NetworkEditor.cc
  NetworkEditorControllerGuiProxy.cc
  TreeViewCollaborators.cc
  ModuleWizard/ModuleWizard.cc
  TriggeredEventsWindow.cc
  Subnetworks.cc
  MacroEditor.cc
  StateViewer.cc
  ModuleOptionsDialogConfiguration.cc
)

set(Interface_Application_HEADERS
  ClosestPortFinder.h
  DeveloperConsole.h
  GuiApplication.h
  GuiCommandFactory.h
  GuiCommands.h
  GuiLogger.h
  HasNotes.h
  MainWindowCollaborators.h
  ModuleWidget.h
  ModuleProxyWidget.h
  NetworkExecutionProgressBar.h
  PortWidgetManager.h
  ProvenanceWindow.h
  SCIRunMainWindow.h
  ShortcutsInterface.h
  Note.h
  NoteEditor.h
  Connection.h
  Port.h
  PositionProvider.h
  PreferencesWindow.h
  TagManagerWindow.h
  NetworkEditor.h
  NetworkEditorControllerGuiProxy.h
  TreeViewCollaborators.h
  Utility.h
  Subnetworks.h
  ModuleWizard/ModuleWizard.h
  TriggeredEventsWindow.h
  MacroEditor.h
  StateViewer.h
  ModuleOptionsDialogConfiguration.h
)

set(Interface_Application_FORMS
  DeveloperConsole.ui
  Module.ui
  ConnectionStyleWizardPage.ui
  OtherSettingsWizardPage.ui
  NetworkSearch.ui
  NoteEditor.ui
  Preferences.ui
  ProvenanceWindow.ui
  SCIRunMainWindow.ui
  ShortcutsInterface.ui
  SubnetEditor.ui
  SubnetWindow.ui
  RenderWindow.ui
  TagManager.ui
  TriggeredEventsWindow.ui
  MacroEditor.ui
  StateViewer.ui
)

# ---- Python-specific sources/UI (conditionally included) ----
set(Interface_Application_PY_SOURCES
  PythonConsoleWidget.cc
)
set(Interface_Application_PY_HEADERS
  PythonConsoleWidget.h
)
set(Interface_Application_PY_FORMS
  PythonWizardPage.ui
  PythonWizardCodePage.ui
  IntWithPythonPage.ui
)

if (BUILD_WITH_PYTHON AND BUILD_INTERFACE_APPLICATION_WITH_PYTHON)
  list(APPEND Interface_Application_SOURCES ${Interface_Application_PY_SOURCES})
  list(APPEND Interface_Application_HEADERS ${Interface_Application_PY_HEADERS})
  list(APPEND Interface_Application_FORMS   ${Interface_Application_PY_FORMS})
endif()

set(Interface_Application_RESOURCES scirun5.qrc)

qt_wrap_ui(Interface_Application_FORMS_HEADERS "${Interface_Application_FORMS}")
qt_wrap_cpp(Interface_Application_HEADERS_MOC "${Interface_Application_HEADERS}")
qt_add_resources(Interface_Application_RESOURCES_RCC "${Interface_Application_RESOURCES}")

if (BUILD_BUNDLE)
  add_definitions(-DBUILD_BUNDLE)
endif()

SCIRUN_ADD_LIBRARY(Interface_Application
  ${Interface_Application_SOURCES}
  ${Interface_Application_HEADERS}
  ${Interface_Application_HEADERS_MOC}
  ${Interface_Application_FORMS_HEADERS}
  ${Interface_Application_RESOURCES_RCC}
)

# ---- Add Python include dirs *only if this app builds with Python* ----
if (BUILD_WITH_PYTHON AND BUILD_INTERFACE_APPLICATION_WITH_PYTHON)
  set(_PY_INCLUDES "")
  if (DEFINED PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_INCLUDE_DIR}")
  endif()
  if (DEFINED PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_PC_INCLUDE_DIR}")
  endif()
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
    target_include_directories(Interface_Application PRIVATE ${_PY_INCLUDES})
    message(STATUS "[Interface/Application] Added Python includes: ${_PY_INCLUDES}")
  else()
    message(WARNING "[Interface/Application] Python includes not found at configure time; Boost.Python compilation may fail.")
  endif()
endif()

# ---- Link libraries ----
set(NonPythonInterface_ApplicationLinkLibraries
  Dataflow_Network
  Core_Serialization_Network_Importer
  Core_Application
  Core_Logging
  Core_Command
  Core_ConsoleApplication
  Interface_Modules_Base
  Interface_Modules_Factory
  Core_Application_Preferences
  ${SCI_BOOST_LIBRARY}    # Ensure this doesn't include Boost.Python for this app
  ${QT_LIBRARIES}
  ${CMAKE_THREAD_LIBS}
)

if (BUILD_WITH_PYTHON AND BUILD_INTERFACE_APPLICATION_WITH_PYTHON)
  target_link_libraries(Interface_Application
    PUBLIC
      ${NonPythonInterface_ApplicationLinkLibraries}
      Core_Python
      SCIRunPythonAPI
  )
else()
  target_link_libraries(Interface_Application
    PUBLIC
      ${NonPythonInterface_ApplicationLinkLibraries}
  )
endif()

target_link_libraries(Interface_Application PUBLIC ${QT_NETWORK_LIBRARIES})

if (BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Interface_Application)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# -------------------------------------------------------------
# ---- Force the correct Python import lib per configuration
#      & ignore wrong defaultlibs (MSVC only), like in Core_ConsoleApplication
# -------------------------------------------------------------
if (MSVC AND BUILD_WITH_PYTHON AND BUILD_INTERFACE_APPLICATION_WITH_PYTHON)
  # Neutralize wrong defaultlibs embedded by dependent .libs (e.g., Boost.Python)
  target_link_options(Interface_Application PRIVATE
    $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
    $<$<CONFIG:Release>:/NODEFAULTLIB:python313_d.lib>
    $<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:python313_d.lib>
    $<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:python313_d.lib>
  )

  # Prefer the imported Python target from superbuild helpers
  if (TARGET SCIRunPython::Python)
    target_link_libraries(Interface_Application PRIVATE
      $<$<CONFIG:Debug>:SCIRunPython::Python>
      $<$<NOT:$<CONFIG:Debug>>:SCIRunPython::Python>
    )
  elseif (TARGET Python::Python)
    target_link_libraries(Interface_Application PRIVATE
      $<$<CONFIG:Debug>:Python::Python>
      $<$<NOT:$<CONFIG:Debug>>:Python::Python>
    )
  else()
    # Fallback to explicit path probing for python313[_d].lib within superbuild
    set(_PY_DBG "")
    set(_PY_REL "")
    foreach(dir
      "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/libs"
      "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/lib"
      "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/PCbuild/amd64"
    )
      if (EXISTS "${dir}/python313_d.lib" AND NOT _PY_DBG)
        set(_PY_DBG "${dir}/python313_d.lib")
      endif()
      if (EXISTS "${dir}/python313.lib" AND NOT _PY_REL)
        set(_PY_REL "${dir}/python313.lib")
      endif()
    endforeach()

    if (_PY_DBG)
      target_link_libraries(Interface_Application PRIVATE
        $<$<CONFIG:Debug>:${_PY_DBG}>
      )
      message(STATUS "[Interface/Application] Using ${_PY_DBG} for Debug")
    else()
      message(WARNING "[Interface/Application] Could not locate python313_d.lib; Debug link may still fail.")
    endif()

    if (_PY_REL)
      target_link_libraries(Interface_Application PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:${_PY_REL}>
      )
      message(STATUS "[Interface/Application] Using ${_PY_REL} for non-Debug")
    else()
      message(WARNING "[Interface/Application] Could not locate python313.lib; non-Debug link may fail.")
    endif()
  endif()
endif()

# -------------------------------------------------------------
# Hard-kill any stray literal python313 libs if they leaked in
# (e.g., via LINK_LIBRARIES property). This is a safety net.
# -------------------------------------------------------------
if (MSVC)
  get_target_property(_ia_link_libs Interface_Application LINK_LIBRARIES)
  if (_ia_link_libs)
    list(FILTER _ia_link_libs EXCLUDE REGEX ".*python313(_d)?\\.lib$")
    set_property(TARGET Interface_Application PROPERTY LINK_LIBRARIES "${_ia_link_libs}")
  endif()
endif()