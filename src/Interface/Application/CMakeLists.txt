
#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

##################################################
# Set sources
##################################################


SET(Interface_Application_SOURCES
  ClosestPortFinder.cc
  DeveloperConsole.cc
  DialogErrorControl.cc
  GuiApplication.cc
  GuiCommandFactory.cc
  GuiCommands.cc
  GuiLogger.cc
  MainWindowCollaborators.cc
  ModuleLogWindow.cc
  ModuleWidget.cc
  ModuleProxyWidget.cc
  NetworkExecutionProgressBar.cc
  ProvenanceWindow.cc
  SCIRunMainWindow.cc
  Node.cc
  Note.cc
  NoteEditor.cc
  Port.cc
  PreferencesWindow.cc
  PythonConsoleWidget.cc
  Settings.cc
  ShortcutsInterface.cc
  TagManagerWindow.cc
  Connection.cc
  NetworkEditor.cc
  NetworkEditorControllerGuiProxy.cc
  TreeViewCollaborators.cc
  ModuleWizard/ModuleWizard.cc
  TriggeredEventsWindow.cc
)

SET(Interface_Application_HEADERS
  ClosestPortFinder.h
  DeveloperConsole.h
  DialogErrorControl.h
  GuiApplication.h
  GuiCommandFactory.h
  GuiCommands.h
  GuiLogger.h
  HasNotes.h
  MainWindowCollaborators.h
  ModuleLogWindow.h
  ModuleWidget.h
  ModuleProxyWidget.h
  NetworkExecutionProgressBar.h
  PortWidgetManager.h
  ProvenanceWindow.h
  SCIRunMainWindow.h
  ShortcutsInterface.h
  Node.h
  Note.h
  NoteEditor.h
  Connection.h
  Port.h
  PositionProvider.h
  PreferencesWindow.h
  PythonConsoleWidget.h
  TagManagerWindow.h
  NetworkEditor.h
  NetworkEditorControllerGuiProxy.h
  TreeViewCollaborators.h
  Utility.h
  ModuleWizard/ModuleWizard.h
  TriggeredEventsWindow.h
)

SET(Interface_Application_FORMS
  DeveloperConsole.ui
  Module.ui
  ModuleMini.ui
  ModuleLogWindow.ui
  NoteEditor.ui
  Preferences.ui
  ProvenanceWindow.ui
  SCIRunMainWindow.ui
  ShortcutsInterface.ui
  RenderWindow.ui
  TagManager.ui
  TriggeredEventsWindow.ui
)

SET(Interface_Application_RESOURCES
scirun5.qrc
)

QT4_WRAP_UI(Interface_Application_FORMS_HEADERS ${Interface_Application_FORMS})
QT4_WRAP_CPP(Interface_Application_HEADERS_MOC ${Interface_Application_HEADERS})
QT4_ADD_RESOURCES(Interface_Application_RESOURCES_RCC ${Interface_Application_RESOURCES})

SCIRUN_ADD_LIBRARY(Interface_Application
  ${Interface_Application_SOURCES}
  ${Interface_Application_HEADERS}
  ${Interface_Application_HEADERS_MOC}
  ${Interface_Application_FORMS_HEADERS}
  ${Interface_Application_RESOURCES_RCC}
)

SET(NonPythonInterface_ApplicationLinkLibraries
  Dataflow_Network
  Core_Serialization_Network_Importer
  Core_Application
  Core_Logging
  Core_Command
  Core_ConsoleApplication
  Interface_Modules_Base
  Interface_Modules_Factory
  Core_Application_Preferences
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
  ${CMAKE_THREAD_LIBS}
)

IF(BUILD_WITH_PYTHON)

TARGET_LINK_LIBRARIES(Interface_Application
  ${NonPythonInterface_ApplicationLinkLibraries}
  Core_Python
  SCIRunPythonAPI
)

ELSE()

TARGET_LINK_LIBRARIES(Interface_Application
  ${NonPythonInterface_ApplicationLinkLibraries}
)

ENDIF()


IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Application)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
