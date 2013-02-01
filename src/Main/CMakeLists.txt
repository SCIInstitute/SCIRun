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

ENDIF()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  Interface_Application
)

SET(BUILD_BUNDLE TRUE)

IF(${AWK} MATCHES "NOTFOUND" OR ${SED} MATCHES "NOTFOUND")
  SET(BUILD_BUNDLE FALSE)
  MESSAGE(WARNING "Awk and sed are required to build the SCIRun app bundle. Please ensure that both are installed and in your path.")
ENDIF()

IF(APPLE AND BUILD_BUNDLE)
# TODO: Xcode settings go here, once Xcode build is working
# IF(${CMAKE_GENERATOR} MATCHES "Xcode")

  # TODO: not sure if this is the best place both for the script and this command
  CONFIGURE_FILE(${SCIRun5Prototype_SOURCE_DIR}/CMake/Utils/processSCIRunLibs.sh.in ${SCIRUN_CORE_BINARY_DIR}/processSCIRunLibs.sh @ONLY)

  IF(${CMAKE_GENERATOR} MATCHES "Unix Makefiles")
    # otool can be used to check list of dependent dylibs
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME} 
      POST_BUILD
        # configure Qt libs
        COMMAND cp -rf "${QT_QTCORE_LIBRARY_RELEASE}/Versions/4/QtCore" "${APPLICATION_NAME}.app/Contents/MacOS/"
        COMMAND cp -rf "${QT_QTGUI_LIBRARY_RELEASE}/Versions/4/QtGui" "${APPLICATION_NAME}.app/Contents/MacOS/"
        COMMAND cp -rf "${QT_QTOPENGL_LIBRARY_RELEASE}/Versions/4/QtOpenGL" "${APPLICATION_NAME}.app/Contents/MacOS/"

        COMMAND install_name_tool -id QtCore ${APPLICATION_NAME}.app/Contents/MacOS/QtCore
        COMMAND install_name_tool -id QtGui ${APPLICATION_NAME}.app/Contents/MacOS/QtGui
        COMMAND install_name_tool -id QtOpenGL ${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL

        COMMAND install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${APPLICATION_NAME}.app/Contents/MacOS/QtGui
        COMMAND install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL
        COMMAND install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui ${APPLICATION_NAME}.app/Contents/MacOS/QtOpenGL

        COMMAND install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
        COMMAND install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}
        COMMAND install_name_tool -change QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/QtOpenGL ${APPLICATION_NAME}.app/Contents/MacOS/${APPLICATION_NAME}

        COMMAND cp "-rf" "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" "${APPLICATION_NAME}.app/Contents"
        COMMAND [ -d ${APPLICATION_NAME}.app/Contents/MacOS/plugins ] || mkdir ${APPLICATION_NAME}.app/Contents/MacOS/plugins
        COMMAND cp -rf "${CMAKE_SOURCE_DIR}/CMake/qt.conf" ${APPLICATION_NAME}.app/Contents/Resources

        COMMAND cp -rf ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${APPLICATION_NAME}.app/Contents/Resources
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/processSCIRunLibs.sh

        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    ENDIF()

    INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION . BUNDLE DESTINATION . )

  ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
