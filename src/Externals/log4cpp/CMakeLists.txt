#
# Nicholas Yue nicholas_yue@users.sourceforge.net
#
# Note:
# (1) A dummy file include/config.h is required (remance from configure)
# (2) Default installation directory is /usr/local, override with -DCMAKE_INSTALL_PREFIX="" during cmake
#     invocation
# (3) Do the usual "make clean all" to build the library
# (4) To install either "make install" or "make install DESTDIR=<your directory>"
# (5) Need to include changes in include/log4cpp/Portability.hh for OSX to build

PROJECT ( LOG4CPP )

INCLUDE_DIRECTORIES ( include )
INCLUDE_DIRECTORIES ( . )

IF (WIN32)
  ADD_DEFINITIONS ( -D_CRT_SECURE_NO_WARNINGS )
ELSE (WIN32)
  IF (APPLE)
    ADD_DEFINITIONS ( -DNDEBUG -DLOG4CPP_HAVE_SSTREAM )
  ELSE (APPLE)
    ADD_DEFINITIONS ( -pthread -DNDEBUG -DLOG4CPP_HAVE_SSTREAM )
  ENDIF (APPLE)
ENDIF (WIN32)

IF ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  SET ( LOG4CPP_LIBRARY_NAME "log4cppD" )
ELSE ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  SET ( LOG4CPP_LIBRARY_NAME "log4cpp" )
ENDIF ( CMAKE_BUILD_TYPE MATCHES "Debug" )

SET(SCI_LOG4CPP_LIBRARY ${LOG4CPP_LIBRARY_NAME} CACHE INTERNAL "log4cpp library name." FORCE)

ADD_LIBRARY ( ${SCI_LOG4CPP_LIBRARY}
  src/Appender.cpp
  src/AppenderSkeleton.cpp
  src/AppendersFactory.cpp
  src/BufferingAppender.cpp
  src/FactoryParams.cpp
  src/LayoutsFactory.cpp
  src/LevelEvaluator.cpp
  src/Localtime.cpp
  src/PassThroughLayout.cpp
  src/TriggeringEventEvaluatorFactory.cpp
  src/LayoutAppender.cpp
  src/FileAppender.cpp
  src/RollingFileAppender.cpp
  src/FixedContextCategory.cpp
  src/IdsaAppender.cpp
  src/OstreamAppender.cpp
  src/StringQueueAppender.cpp
  src/SyslogAppender.cpp
  src/RemoteSyslogAppender.cpp
  src/SimpleLayout.cpp
  src/BasicLayout.cpp
  src/PatternLayout.cpp
  src/Category.cpp
  src/CategoryStream.cpp
  src/HierarchyMaintainer.cpp
  src/Configurator.cpp
  src/BasicConfigurator.cpp
  src/SimpleConfigurator.cpp
  src/PropertyConfigurator.cpp
  src/PropertyConfiguratorImpl.cpp
  src/LoggingEvent.cpp
  src/Priority.cpp
  src/NDC.cpp
  src/Filter.cpp
  src/TimeStamp.cpp
  src/StringUtil.cpp
  src/Properties.cpp
  src/Win32DebugAppender.cpp
  src/NTEventLogAppender.cpp
  src/DllMain.cpp
  src/DummyThreads.cpp
  src/MSThreads.cpp
  src/OmniThreads.cpp
  src/PThreads.cpp
  src/PortabilityImpl.cpp
  src/AbortAppender.cpp
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DLOG4CPP_HAS_DLL)
  ADD_DEFINITIONS(-DLOG4CPP_BUILD_DLL)
ENDIF(BUILD_SHARED_LIBS)

IF (WIN32)
  TARGET_LINK_LIBRARIES (${SCI_LOG4CPP_LIBRARY} kernel32 user32 ws2_32 advapi32 )
  #SET_TARGET_PROPERTIES(${SCI_LOG4CPP_LIBRARY} PROPERTIES LINK_FLAGS /NODEFAULTLIB:msvcrt )

  # Add to instaler package (Release build only)
  INSTALL(TARGETS ${SCI_LOG4CPP_LIBRARY} RUNTIME DESTINATION bin CONFIGURATIONS Release)

ENDIF (WIN32)

# This works because log4cpp is part of the SCIRun git repository.
# If this project gets moved to a CMake external project, this directive will have to be revised.
#ADD_DEPENDENCIES(${SCI_LOG4CPP_LIBRARY} Boost_external)


#INSTALL (
#  DIRECTORY include/log4cpp
#  DESTINATION include
#  PATTERN "config.h" EXCLUDE
#  PATTERN ".svn" EXCLUDE
#  PATTERN "*.am" EXCLUDE
#  PATTERN "*.in" EXCLUDE
#  )

#INSTALL (
#  TARGETS ${SCI_LOG4CPP_LIBRARY}
#  ARCHIVE DESTINATION lib
#  )
