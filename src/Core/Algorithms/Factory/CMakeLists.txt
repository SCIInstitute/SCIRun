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


SET(Algorithms_Factory_SRCS
  HardCodedAlgorithmFactory.cc
)

SET(Algorithms_Factory_HEADERS
  HardCodedAlgorithmFactory.h
  share.h
)

SET(AlgorithmFactoryImpl_Generated ${CMAKE_CURRENT_BINARY_DIR}/AlgorithmFactoryImpl_Generated.cc)

IF(GENERATE_MODULE_FACTORY_CODE)
  MESSAGE(STATUS "Setting up algorithm factory generator")
  ADD_EXECUTABLE(MakeAlgorithmFactory Generator/MakeAlgorithmFactory.h Generator/MakeAlgorithmFactory.cc)
  TARGET_LINK_LIBRARIES(MakeAlgorithmFactory
    Modules_Factory_Generator
    ${SCI_BOOST_LIBRARY})
  SET_PROPERTY(TARGET MakeAlgorithmFactory   PROPERTY FOLDER "Modules")

  # add the command to generate the source code
  ADD_CUSTOM_COMMAND(
    OUTPUT ${AlgorithmFactoryImpl_Generated}
    COMMAND MakeAlgorithmFactory ${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules/Factory/ ${AlgorithmFactoryImpl_Generated}
    DEPENDS MakeAlgorithmFactory
  )
ENDIF()

SCIRUN_ADD_LIBRARY(Algorithms_Factory
  ${Algorithms_Factory_HEADERS}
  ${Algorithms_Factory_SRCS}
  ${AlgorithmFactoryImpl_Generated}
)

IF(GENERATE_MODULE_FACTORY_CODE AND REGENERATE_MODULE_FACTORY_CODE)
  ADD_CUSTOM_COMMAND(
    TARGET Algorithms_Factory
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove -f ${AlgorithmFactoryImpl_Generated}
    COMMENT "Removed generated Algorithm Factory code--it will be regenerated next build. Turn off CMake flag REGENERATE_MODULE_FACTORY_CODE if new modules are not being added."
  )
ENDIF()

TARGET_LINK_LIBRARIES(Algorithms_Factory
  Algorithms_Base
  Algorithms_BrainStimulator
  Algorithms_Legacy_Inverse
  Core_Algorithms_Legacy_Fields
  Algorithms_Math
  Algorithms_DataIO
  Algorithms_Field
  Core_Algorithms_Visualization
  Core_Algorithms_Legacy_FiniteElements
  Core_Algorithms_Legacy_Converter
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Algorithms_Factory)
ENDIF(BUILD_SHARED_LIBS)

IF(BUILD_TESTING OR TRAVIS_BUILD)
  ADD_SUBDIRECTORY(Tests)
ENDIF()
