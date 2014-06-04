#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

# Sources of Core/Thread classes

SET(Core_Thread_Legacy_SRCS
  AtomicCounter_default.cc
  Barrier_default.cc
  CleanupManager.cc 
  CrowdMonitor_default.cc
  Guard.cc 
  MutexPool.cc 
  ParallelBase.cc 
  RecursiveMutex_default.cc
  Runnable.cc 
  Thread.cc 
  ThreadError.cc 
  SimpleReducer.cc 
  ThreadLock.cc 
  ThreadGroup.cc 
  Thread_unix.cc 
  ThreadPool.cc 
  WorkQueue.cc 
)

SET(Core_Thread_Legacy_HEADERS
  AtomicCounter.h
  Barrier.h
  CleanupManager.h
  ConditionVariable.h
  CrowdMonitor.h
  FutureValue.h
  Guard.h
  Mailbox.h
  Mutex.h
  MutexPool.h
  Parallel.h
  Parallel1.h
  Parallel2.h
  Parallel3.h
  Parallel4.h
  ParallelBase.h
  RecursiveMutex.h
  Reducer.h
  Runnable.h
  Semaphore.h
  Thread.h
  ThreadError.h
  share.h
  SimpleReducer.h
  ThreadLock.h
  ThreadGroup.h
  Thread_unix.h
  ThreadPool.h
  Time.h  
  UsedWithLockingHandle.h
  WorkQueue.h
)

IF(WIN32)
  SET(Core_Thread_Legacy_SRCS ${Core_Thread_Legacy_SRCS} Thread_win32.cc Time_win32.cc)
ENDIF(WIN32)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Thread_Legacy)
ENDIF(BUILD_SHARED_LIBS)

ADD_LIBRARY(Core_Thread_Legacy 
  ${Core_Thread_Legacy_SRCS}
  ${Core_Thread_Legacy_HEADERS}
)

SET(Core_Thread_Legacy_Link
  Core_Exceptions_Legacy 
  #${THREAD_LIBRARY}  
  #${TRACEBACK_LIB} 
)

IF(WIN32)
  SET(Core_Thread_Legacy_Link ${Core_Thread_Legacy_Link} winmm.lib)
ENDIF(WIN32)

IF(UNIX AND NOT APPLE)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrt")
ENDIF()

TARGET_LINK_LIBRARIES(Core_Thread_Legacy 
  ${Core_Thread_Legacy_Link}
)

ADD_DEPENDENCIES(Core_Thread_Legacy Boost_external)
