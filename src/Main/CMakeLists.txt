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

SET(SCIRunMain_SOURCES
  scirunMain.cc
)

IF(BUILD_HEADLESS)
  ADD_DEFINITIONS(-DBUILD_HEADLESS)
ENDIF(BUILD_HEADLESS)

ADD_DEFINITIONS(-DAPPLICATION_NAME=${APPLICATION_NAME})

IF(APPLE)
  # TODO: temporary - until testing can be configured with app bundle
  ADD_EXECUTABLE(${TEST_APPLICATION_NAME}
    ${SCIRunMain_SOURCES}
  )
  TARGET_LINK_LIBRARIES(${TEST_APPLICATION_NAME}
    Interface_Application
  )

  LIST(APPEND SCIRunMain_SOURCES "${APPLICATION_OSX_ICON_FULL_PATH}")
  SET_SOURCE_FILES_PROPERTIES(${APPLICATION_OSX_ICON_FULL_PATH}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION Resources)
  SET(MACOSX_BUNDLE_ICON_FILE "${APPLICATION_OSX_ICON}" )

  ADD_EXECUTABLE(${APPLICATION_NAME} MACOSX_BUNDLE ${SCIRunMain_SOURCES})

ELSEIF(WIN32)
  SET(SCIRunMain_SOURCES ${SCIRunMain_SOURCES} "${APPLICATION_WINDOWS_RC}")
  IF(SCIRUN_SHOW_CONSOLE)
    ADD_DEFINITIONS(-DSCIRUN_SHOW_CONSOLE=1)
  ENDIF()
  ADD_EXECUTABLE(${APPLICATION_NAME} WIN32 ${SCIRunMain_SOURCES})

  # TODO: probably need this for Xcode
  IF(WIN32 AND MSVC)
    # TODO: more build types to be supported?
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy_directory
      ${SCIRun_BINARY_DIR}/Shaders
      ${SCIRun_BINARY_DIR}/$<CONFIGURATION>/Shaders
   )
   ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
     POST_BUILD
     COMMAND ${CMAKE_COMMAND}
     ARGS -E copy_directory
     ${SCIRun_BINARY_DIR}/Assets
     ${SCIRun_BINARY_DIR}/$<CONFIGURATION>/Assets
    )
  ENDIF()

ELSE()
  ADD_EXECUTABLE(${APPLICATION_NAME} ${SCIRunMain_SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
  Interface_Application
  Core_Util_Legacy
)

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME}
    Core_Python
  )
ENDIF()

OPTION(DISABLE_MAC_BUNDLE "Disable Mac bundle" OFF)
MARK_AS_ADVANCED(DISABLE_MAC_BUNDLE)

IF(APPLE)
  SET(BUILD_BUNDLE NOT ${DISABLE_MAC_BUNDLE})

  IF(${MACDEPLOYQT} MATCHES "NOTFOUND" OR ${SED} MATCHES "NOTFOUND" OR ${AWK} MATCHES "NOTFOUND")
    SET(BUILD_BUNDLE FALSE)
    MESSAGE(WARNING "macdeployqt, sed and awk are required to build the SCIRun app bundle. Please ensure that all are installed and in your path.")
  ENDIF()
ENDIF()

IF(APPLE AND BUILD_BUNDLE)
  MESSAGE(STATUS "Building Mac OS X bundle")
  SET(APP_PATH "${CMAKE_CFG_INTDIR}/${APPLICATION_NAME}.app/Contents")

  IF(${CMAKE_GENERATOR} MATCHES "Xcode")
    SET(SCIRUN_LIB_DIR ${SCIRun_BINARY_DIR}/lib/$<CONFIGURATION>)
  ELSE()
    SET(SCIRUN_LIB_DIR ${SCIRun_BINARY_DIR}/lib)
  ENDIF()
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
      COMMAND install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python
        @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python
        ${APP_PATH}/MacOS/${APPLICATION_NAME}
        WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
    SET(SCIRUN_LIBS "Core_Application"
                    "Core_Python"
                    "SCIRunPythonAPI"
                    "Interface_Application"
                    "Engine_Network"
                    "Interface_Modules_Factory"
                    "Interface_Modules_Render"
                    "Core_ConsoleApplication"
    )
    FOREACH(lib ${SCIRUN_LIBS})
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
        POST_BUILD
        COMMAND install_name_tool -change ${SCI_PYTHON_FRAMEWORK}/${VERSION_PATH}/Python
          @executable_path/../${FRAMEWORK_PATH}/${VERSION_PATH}/Python
          ${APP_PATH}/Frameworks/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}
        WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
    ENDFOREACH()

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
       WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
  ENDIF()

  INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION . BUNDLE DESTINATION . )

ELSE()

  IF(WIN32)
    IF(BUILD_WITH_PYTHON)
      # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
      # TODO: copy python dll
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} $<CONFIGURATION>/${PYTHON_MODULE_SEARCH_PATH}
        WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
    ENDIF()

    # Note: installer is created from Release build only!
    GET_FILENAME_COMPONENT(QTCORENAME ${QT_QTCORE_LIBRARY_RELEASE} NAME_WE)
    GET_FILENAME_COMPONENT(QTGUINAME ${QT_QTGUI_LIBRARY_RELEASE} NAME_WE)
    GET_FILENAME_COMPONENT(QTOPENGLNAME ${QT_QTOPENGL_LIBRARY_RELEASE} NAME_WE)
    GET_FILENAME_COMPONENT(QTNETWORKNAME ${QT_QTNETWORK_LIBRARY_RELEASE} NAME_WE)

    INSTALL(TARGETS ${APPLICATION_NAME} RUNTIME DESTINATION bin)
    INSTALL(PROGRAMS
      ${QT_BINARY_DIR}/${QTCORENAME}.dll
      ${QT_BINARY_DIR}/${QTGUINAME}.dll
      ${QT_BINARY_DIR}/${QTOPENGLNAME}.dll
      ${QT_BINARY_DIR}/${QTNETWORKNAME}.dll
      DESTINATION bin)

    INSTALL(DIRECTORY "${SCIRun_BINARY_DIR}/Shaders" DESTINATION bin USE_SOURCE_PERMISSIONS)
    INSTALL(DIRECTORY "${SCIRun_BINARY_DIR}/Assets" DESTINATION bin USE_SOURCE_PERMISSIONS)
  ELSE()
    IF(BUILD_WITH_PYTHON)
      ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PYTHON_MODULE_SEARCH_PATH}
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
        WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
      IF(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH AND EXISTS ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH})
        ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
          POST_BUILD
          COMMAND echo "${CMAKE_COMMAND} ARGS -E copy_directory ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}"
          COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory  ${SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH} ${PYTHON_MODULE_SEARCH_PATH}
          WORKING_DIRECTORY ${SCIRun_BINARY_DIR})
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

SET(NETWORKS_DESTINATION_DIR  "./SCIRunNetworks")

# TODO: be careful of SCIRun network file extension
# TODO: refactor when networks are reorganized
INSTALL(DIRECTORY "${SCIRun_SOURCE_DIR}/ExampleNets/regression" DESTINATION ${NETWORKS_DESTINATION_DIR}
  USE_SOURCE_PERMISSIONS FILES_MATCHING PATTERN "*.srn5")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
