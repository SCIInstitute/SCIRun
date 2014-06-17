#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

SET(SCIRunGUIPrototype_SOURCES 
  scirunMain.cc 
)

IF(BUILD_HEADLESS)
  ADD_DEFINITIONS(-DBUILD_HEADLESS)
ENDIF(BUILD_HEADLESS)

IF(APPLE)
  # TODO: temporary - until testing can be configured with app bundle
  ADD_EXECUTABLE(${TEST_APPLICATION_NAME}
    ${SCIRunGUIPrototype_SOURCES}
  )
  TARGET_LINK_LIBRARIES(${TEST_APPLICATION_NAME}
    Interface_Application
  )

  LIST(APPEND SCIRunGUIPrototype_SOURCES "${APPLICATION_OSX_ICON_FULL_PATH}")
  SET_SOURCE_FILES_PROPERTIES(${APPLICATION_OSX_ICON_FULL_PATH}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION Resources)
  SET(MACOSX_BUNDLE_ICON_FILE "${APPLICATION_OSX_ICON}" )

  ADD_EXECUTABLE(${APPLICATION_NAME} MACOSX_BUNDLE ${SCIRunGUIPrototype_SOURCES})

ELSE()
# TODO: ELSE IF WIN32 with NSIS settings, generic Unix build

  ADD_EXECUTABLE(${APPLICATION_NAME}
    ${SCIRunGUIPrototype_SOURCES}
  )

  # TODO: probably need this for Xcode
  IF(WIN32 AND MSVC)
    # TODO: more build types to be supported?
    FOREACH(BUILD_TYPE Debug Release)
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
                         POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                           ARGS -E copy_directory
                           ${SCIRUN_CORE_BINARY_DIR}/Shaders
                           ${SCIRUN_CORE_BINARY_DIR}/${BUILD_TYPE}/Shaders
      )
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
                         POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                           ARGS -E copy_directory
                           ${SCIRUN_CORE_BINARY_DIR}/Assets
                           ${SCIRUN_CORE_BINARY_DIR}/${BUILD_TYPE}/Assets
      )
    ENDFOREACH()
  ENDIF()

ENDIF()

# TODO: Spire library linked against executable (shouldn't be necessary)?
TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  Interface_Application
)

IF(APPLE)
  SET(BUILD_BUNDLE TRUE)

  IF(${AWK} MATCHES "NOTFOUND" OR ${SED} MATCHES "NOTFOUND" OR ${MACDEPLOYQT} MATCHES "NOTFOUND")
    SET(BUILD_BUNDLE FALSE)
    MESSAGE(WARNING "Awk, sed and macdeployqt are required to build the SCIRun app bundle. Please ensure that both are installed and in your path.")
  ENDIF()
ENDIF()

IF(APPLE AND BUILD_BUNDLE)
# TODO: Xcode settings go here, once Xcode build is working
# IF(${CMAKE_GENERATOR} MATCHES "Xcode")

  IF(${CMAKE_GENERATOR} MATCHES "Unix Makefiles")
    # otool can be used to check list of dependent dylibs
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME} 
      POST_BUILD
        # configure Qt libs
        # TODO: starting with debug level output, reduce later...
        COMMAND ${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=${MACDEPLOYQT_OUTPUT_LEVEL}

        COMMAND cp -Rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" ${APPLICATION_NAME}.app/Contents/Resources

        # configure shaders (copied from build directory)
        COMMAND cp -Rf "${SCIRUN_CORE_BINARY_DIR}/Shaders" "${APPLICATION_NAME}.app/Contents/MacOS"
        COMMAND chmod +x "${APPLICATION_NAME}.app/Contents/MacOS/Shaders"
        COMMAND cp -Rf "${SCIRUN_CORE_BINARY_DIR}/Assets" "${APPLICATION_NAME}.app/Contents/MacOS"
        COMMAND chmod +x "${APPLICATION_NAME}.app/Contents/MacOS/Assets"

        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
  ENDIF()

  INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION . BUNDLE DESTINATION . )

ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
