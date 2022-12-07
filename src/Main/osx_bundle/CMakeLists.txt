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


SET(SCIRunMainOSXBundle_SOURCES
  ../scirunMain.cc
)

LIST(APPEND SCIRunMainOSXBundle_SOURCES "${APPLICATION_OSX_ICON_FULL_PATH}")
SET_SOURCE_FILES_PROPERTIES(${APPLICATION_OSX_ICON_FULL_PATH}
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION Resources)
SET(MACOSX_BUNDLE_ICON_FILE "${APPLICATION_OSX_ICON}" )

ADD_EXECUTABLE(${APPLICATION_NAME} MACOSX_BUNDLE
  ${SCIRunMainOSXBundle_SOURCES}
)

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  Core_Util_Legacy
)

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  Interface_Application
)

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
    Core_Python
  )
ENDIF()

IF(${CMAKE_GENERATOR} MATCHES "Xcode")
  SET(SCIRUN_LIB_DIR ${SCIRun_BINARY_DIR}/lib/$<CONFIGURATION>)
ELSE()
  SET(SCIRUN_LIB_DIR ${SCIRun_BINARY_DIR}/lib)
ENDIF()




SET(APP_PATH "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents")

# otool can be used to check list of dependent dylibs
ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
  POST_BUILD
  # clean out libraries
  COMMAND ${CMAKE_COMMAND} ARGS -E remove -f ${APP_PATH}/Frameworks/*${CMAKE_SHARED_LIBRARY_SUFFIX}
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCIRUN_LIB_DIR} ${APP_PATH}/Frameworks
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

IF(BUILD_WITH_PYTHON)
  SET(VERSION_PATH Versions/${SCI_PYTHON_VERSION_SHORT})
  SET(FRAMEWORK_PATH Frameworks/Python.framework)
  SET(PYTHON_PATH ${APP_PATH}/${FRAMEWORK_PATH}/${VERSION_PATH})
  ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
    POST_BUILD
    # macdeployqt puts frameworks in app/Content/Frameworks
    # TODO: when CMake 2.8 support is dropped, and CMP0042 new is enabled, change to @rpath
    COMMAND ${CMAKE_COMMAND} ARGS -E chdir ${APP_PATH}/Frameworks tar xf ${SCI_PYTHON_FRAMEWORK_ARCHIVE}
    COMMAND chmod u+w  ${PYTHON_PATH}/Python
    COMMAND install_name_tool -id Python ${PYTHON_PATH}/Python
    COMMAND install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python
      @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python
      ${APP_PATH}/MacOS/${APPLICATION_NAME}
      WORKING_DIRECTORY ${SCIRun_BINARY_DIR})

  SET(SCIRUN_LIBS "Core_Application"
                  "Core_Application_Preferences"
                  "Core_Python"
                  "SCIRunPythonAPI"
                  "Interface_Application"
                  "Engine_Network"
                  "Interface_Modules_Factory"
                  "Interface_Modules_Render"
                  "Interface_Modules_Python"
                  "Interface_Modules_Visualization"
                  "Interface_Modules_Math"
                  "Core_ConsoleApplication"
                  "Modules_Factory"
                  "Modules_Python"
                  "Modules_Math"
  )

  FOREACH(lib ${SCIRUN_LIBS})
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python
        @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python
        ${APP_PATH}/Frameworks/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}
      WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
  ENDFOREACH()
ENDIF()



# macdeployqt puts frameworks in app/Content/Frameworks

MESSAGE(STATUS "Building Mac OS X bundle")

ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
  POST_BUILD
  # configure Qt libs
  # Note: macdeployqt also processes SCIRun dynamically linked libraries under the lib directory
  COMMAND ${MACDEPLOYQT} ${APPLICATION_NAME}.app -verbose=${MACDEPLOYQT_OUTPUT_LEVEL}
  COMMAND ${CMAKE_COMMAND} ARGS -E copy ${SCIRun_SOURCE_DIR}/CMake/qt.conf ${APPLICATION_NAME}.app/Contents/Resources
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
    ${SCIRun_BINARY_DIR}/Shaders ${APP_PATH}/MacOS/Shaders
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
    ${SCIRun_BINARY_DIR}/Assets ${APP_PATH}/MacOS/Assets
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
    ${SCIRun_BINARY_DIR}/Fonts ${APP_PATH}/MacOS/Fonts
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION . BUNDLE DESTINATION . )
