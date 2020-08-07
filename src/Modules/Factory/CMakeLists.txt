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


SET(Modules_Factory_SRCS
  HardCodedModuleFactory.cc
  ModuleFactoryImpl1.cc
  ModuleFactoryImpl2.cc
)

SET(Modules_Factory_HEADERS
  HardCodedModuleFactory.h
  ModuleDescriptionLookup.h
  share.h
)

ADD_SUBDIRECTORY(Generator)

SET(ModuleFactoryImpl_Generated ${CMAKE_CURRENT_BINARY_DIR}/ModuleFactoryImpl_Generated.cc)

IF(GENERATE_MODULE_FACTORY_CODE)
  MESSAGE(STATUS "Setting up module factory generator")

  # add the command to generate the source code
  ADD_CUSTOM_COMMAND(
   OUTPUT ${ModuleFactoryImpl_Generated}
   COMMAND echo "MakeModuleFactory ${CMAKE_CURRENT_SOURCE_DIR} ${ModuleFactoryImpl_Generated}"
   COMMAND MakeModuleFactory ${CMAKE_CURRENT_SOURCE_DIR} ${ModuleFactoryImpl_Generated}
   DEPENDS MakeModuleFactory
 )
ENDIF()

SCIRUN_ADD_LIBRARY(Modules_Factory
  ${Modules_Factory_HEADERS}
  ${Modules_Factory_SRCS}
  ${ModuleFactoryImpl_Generated}
)

IF(GENERATE_MODULE_FACTORY_CODE AND REGENERATE_MODULE_FACTORY_CODE)
  ADD_CUSTOM_COMMAND(
    TARGET Modules_Factory
    POST_BUILD
    COMMAND echo "${CMAKE_COMMAND} -E remove -f ${ModuleFactoryImpl_Generated}"
    COMMAND ${CMAKE_COMMAND} -E remove -f ${ModuleFactoryImpl_Generated}
    COMMENT "Removed generated Module Factory code--it will be regenerated next build. Turn off CMake flag REGENERATE_MODULE_FACTORY_CODE if new modules are not being added."
  )
ENDIF()

TARGET_LINK_LIBRARIES(Modules_Factory
  Dataflow_Network
  Modules_Math
  Modules_Fields
  Modules_BrainStimulator
  Modules_Legacy_FiniteElements
  Modules_Legacy_Forward
  Modules_Legacy_Inverse
  Modules_Legacy_Visualization
  Modules_Basic
  Modules_DataIO
  Modules_String
  Modules_Render
  Modules_Python
  Modules_Visualization
  Modules_Legacy_Math
  Modules_Legacy_Bundle
  Modules_Legacy_Fields
  Modules_Legacy_Matlab_DataIO
  Modules_Legacy_Converters
  Modules_Legacy_Teem_DataIO
  Modules_Legacy_Teem_Misc
  Modules_Legacy_Teem_Converters
  Modules_Legacy_String
)

IF(WITH_TETGEN)
  ADD_DEFINITIONS(-DWITH_TETGEN)
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Factory)
ENDIF(BUILD_SHARED_LIBS)

IF(BUILD_TESTING OR TRAVIS_BUILD)
  ADD_SUBDIRECTORY(Tests)
ENDIF()
