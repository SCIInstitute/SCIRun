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


SET(Interface_Modules_Factory_HEADERS
  ModuleDialogFactory.h
  share.h
)

SET(Interface_Modules_Factory_SOURCES
  ModuleDialogFactory.cc
  ModuleDialogFactoryAdditional.cc
)

SET(ModuleDialogFactory_Generated ${CMAKE_CURRENT_BINARY_DIR}/ModuleDialogFactory_Generated.cc)

IF(GENERATE_MODULE_FACTORY_CODE)
  MESSAGE(STATUS "Setting up dialog factory generator")
  ADD_EXECUTABLE(MakeDialogFactory Generator/MakeDialogFactory.h Generator/MakeDialogFactory.cc)
  TARGET_LINK_LIBRARIES(MakeDialogFactory
    Modules_Factory_Generator
    ${SCI_BOOST_LIBRARY}
  )
  SET_PROPERTY(TARGET MakeDialogFactory PROPERTY FOLDER "Interface")

  # add the command to generate the source code
  ADD_CUSTOM_COMMAND(
    OUTPUT ${ModuleDialogFactory_Generated}
    COMMAND echo "MakeDialogFactory ${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules/Factory/ ${ModuleDialogFactory_Generated}"
    COMMAND MakeDialogFactory ${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules/Factory/ ${ModuleDialogFactory_Generated}
    DEPENDS MakeDialogFactory
    VERBATIM
  )
ENDIF()

SCIRUN_ADD_LIBRARY(Interface_Modules_Factory
  ${Interface_Modules_Factory_HEADERS}
  ${Interface_Modules_Factory_SOURCES}
  ${ModuleDialogFactory_Generated}
)

IF(GENERATE_MODULE_FACTORY_CODE AND REGENERATE_MODULE_FACTORY_CODE)
  ADD_CUSTOM_COMMAND(
    TARGET Interface_Modules_Factory
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E remove -f ${ModuleDialogFactory_Generated}
       COMMENT "Removed generated Module Dialog Factory code--it will be regenerated next build. Turn off CMake flag REGENERATE_MODULE_FACTORY_CODE if new modules are not being added."
  )
ENDIF()

TARGET_LINK_LIBRARIES(Interface_Modules_Factory
  Interface_Modules_Base
  Interface_Modules_DataIO
  Interface_Modules_Math
  Interface_Modules_Fields
  Interface_Modules_Bundle
  Interface_Modules_FiniteElements
  Interface_Modules_BrainStimulator
  Interface_Modules_Forward
  Interface_Modules_Inverse
  Interface_Modules_Render
  Interface_Modules_String
  Interface_Modules_Matlab
  Interface_Modules_Teem
  Interface_Modules_Visualization
  Interface_Modules_Python
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(WITH_OSPRAY)
  TARGET_LINK_LIBRARIES(Interface_Modules_Factory
    Interface_Modules_Render_Ospray
  )
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Factory)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SCIRUN_ADD_TEST_DIR(Tests)
