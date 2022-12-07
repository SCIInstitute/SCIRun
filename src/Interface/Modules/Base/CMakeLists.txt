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


SET(Interface_Modules_Base_FORMS
  ModuleDialogBasic.ui
  ParserHelp.ui
  ModuleButtonBar.ui
  SubnetDialog.ui
  ModuleLogWindow.ui
  CustomWidgets/QtHistogramWidget.ui
)

SET(Interface_Modules_Base_HEADERS
  share.h
  ModuleDialogManager.h
  DialogErrorControl.h
  CustomWidgets/CTK/Core/ctkPimpl.h
  CustomWidgets/CTK/Core/ctkUtils.h
  CustomWidgets/CTK/ctkWidgetsUtils.h
)

SET(Interface_Modules_Base_HEADERS_M
  ModuleDialogBasic.h
  ModuleDialogGeneric.h
  RemembersFileDialogDirectory.h
  WidgetSlotManagers.h
  HasParserHelpDialog.h
  ModuleButtonBar.h
  ModuleLogWindow.h
  CustomWidgets/QtHistogramWidget.h
  CustomWidgets/QtHistogramGraph.h
  CustomWidgets/CodeEditorWidgets.h
  CustomWidgets/CTK/ctkColorPickerButton.h
  CustomWidgets/CTK/ctkDoubleSpinBox.h
  CustomWidgets/CTK/ctkDoubleSpinBox_p.h
  CustomWidgets/CTK/ctkBasePopupWidget.h
  CustomWidgets/CTK/ctkBasePopupWidget_p.h
  CustomWidgets/CTK/ctkPopupWidget.h
  CustomWidgets/CTK/ctkPopupWidget_p.h
  CustomWidgets/CTK/Core/ctkValueProxy.h
)

SET(Interface_Modules_Base_SOURCES
  ModuleDialogBasic.cc
  ModuleDialogGeneric.cc
  RemembersFileDialogDirectory.cc
  WidgetSlotManagers.cc
  HasParserHelpDialog.cc
  ModuleButtonBar.cc
  ModuleDialogManager.cc
  ModuleLogWindow.cc
  DialogErrorControl.cc
  CustomWidgets/QtHistogramWidget.cc
  CustomWidgets/QtHistogramGraph.cc
  CustomWidgets/CodeEditorWidgets.cc
  CustomWidgets/CTK/ctkColorPickerButton.cpp
  CustomWidgets/CTK/ctkDoubleSpinBox.cpp
  CustomWidgets/CTK/ctkPopupWidget.cpp
  CustomWidgets/CTK/ctkBasePopupWidget.cpp
  CustomWidgets/CTK/Core/ctkUtils.cpp
  CustomWidgets/CTK/Core/ctkValueProxy.cpp
  CustomWidgets/CTK/ctkWidgetsUtils.cpp
)

QT_WRAP_UI(Interface_Modules_Base_FORMS_HEADERS "${Interface_Modules_Base_FORMS}")
QT_WRAP_CPP(Interface_Modules_Base_HEADERS_MOC "${Interface_Modules_Base_HEADERS_M}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Base
  ${Interface_Modules_Base_HEADERS}
  ${Interface_Modules_Base_HEADERS_M}
  ${Interface_Modules_Base_FORMS_HEADERS}
  ${Interface_Modules_Base_HEADERS_MOC}
  ${Interface_Modules_Base_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Base
  Algorithms_Base
  Core_Logging
  Core_Math
  Core_Datatypes
  Dataflow_State
  Core_Application_Preferences
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Base)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
