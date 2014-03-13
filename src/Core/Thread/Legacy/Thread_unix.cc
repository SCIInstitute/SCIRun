#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/



/**
 *@file  Thread_unix.cc
 *@brief Utilities for all unix implementations of the
 * 		    the thread library
 *
 *@author
 *   Steve Parker
 *   Department of Computer Science
 *   University of Utah
 *@date: June 1997
 *
 */

#include <Core/Thread/Legacy/Thread_unix.h>
#include <sstream>

#ifdef _WIN32
#include <signal.h>
#include <errno.h>
#else
#include <sys/errno.h>
#include <sys/signal.h>
#endif

std::string
Core_Thread_signal_name(int sig, void* addr)
{
  std::stringstream str;
  switch(sig){
  case SIGINT:
    str << "SIGINT (interrupt) ";
    break;
  case SIGILL:
    str << "SIGILL at address " << addr << " (illegal instruction)";
    break;
  case SIGABRT:
    str << "SIGABRT (Abort) ";
    break;
  case SIGSEGV:
    str << "SIGSEGV at address " << addr << " (segmentation violation)";
    break;
  case SIGTERM:
    str << "SIGTERM (killed) ";
    break;
  case SIGFPE:
    str << "SIGFPE (floating point exception) ";
    break;
#ifdef SIGBREAK
  case SIGBREAK:
    str << "SIGBREAK (CTRL-Break sequence) ";
    break;
#endif

    // these signals don't exist in win32
#ifndef _WIN32 
  case SIGHUP:
    str << "SIGHUP (hangup) ";
    break;
  case SIGQUIT:
    str << "SIGQUIT (quit) ";
    break;
  case SIGTRAP:
    str << "SIGTRAP (trace trap) ";
    break;
#ifdef SIGEMT
  case SIGEMT:
    str << "SIGEMT (Emulation Trap) ";
    break;
#endif
#ifdef SIGIOT
#if SIGEMT != SIGIOT && SIGIOT != SIGABRT
  case SIGIOT:
    str << "SIGIOT (IOT Trap) ";
    break;
#endif
#endif
  case SIGBUS:
    str << "SIGBUS at address " << addr << " (bus error)";
    break;
  case SIGKILL:
    str << "SIGKILL (kill) ";
    break;
#ifdef SIGSYS
  case SIGSYS:
    str << "SIGSYS (bad argument to system call) ";
    break;
#endif
  case SIGPIPE:
    str << "SIGPIPE (broken pipe) ";
    break;
  case SIGALRM:
    str << "SIGALRM (alarm clock) ";
    break;
  case SIGUSR1:
    str << "SIGUSR1 (user defined signal 1) ";
    break;
  case SIGUSR2:
    str << "SIGUSR2 (user defined signal 2) ";
    break;
  case SIGCHLD:
    str << "SIGCLD (death of a child) ";
    break;
#ifdef SIGPWR
  case SIGPWR:
    str << "SIGPWR (power fail restart) ";
    break;
#endif
  case SIGWINCH:
    str << "SIGWINCH (window size changes) ";
    break;
  case SIGURG:
    str << "SIGURG (urgent condition on IO channel) ";
    break;
  case SIGIO:  // Also SIGPOLL
    str << "SIGIO/SIGPOLL (i/o possible) ";
    break;
  case SIGSTOP:
    str << "SIGSTOP (sendable stop signal) ";
    break;
  case SIGTSTP:
    str << "SIGTSTP (TTY stop) ";
    break;
  case SIGCONT:
    str << "SIGCONT (continue) ";
    break;
  case SIGTTIN:
    str << "SIGTTIN ";
    break;
  case SIGTTOU:
    str << "SIGTTOU ";
    break;
  case SIGVTALRM:
    str << "SIGVTALRM (virtual time alarm) ";
    break;
  case SIGPROF:
    str << "SIGPROF (profiling alarm) ";
    break;
  case SIGXCPU:
    str << "SIGXCPU (CPU time limit exceeded) ";
    break;
  case SIGXFSZ:
    str << "SIGXFSZ (Filesize limit exceeded) ";
    break;
#endif
  default:
    str << "unknown signal(" << sig << ")";
    break;
  }
  return str.str();
}
#endif
