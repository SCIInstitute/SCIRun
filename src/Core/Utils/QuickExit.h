/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#ifndef CORE_UTILS_QUICKEXIT_H
#define CORE_UTILS_QUICKEXIT_H

#include <cstdlib>

#ifdef __APPLE__
  #include <Availability.h>
#endif

/// quick_exit only entered libSystem in macOS 15.0, and the SDK declares it with no
/// availability annotation ("void quick_exit(int) __dead2;" in _stdlib.h). Compiling
/// against the macOS 15 SDK with an older deployment target therefore emits a hard
/// reference and links cleanly with no warning, but the binary aborts on load on
/// macOS < 15 with "dyld: Symbol not found: _quick_exit". Fall back to _Exit there.
///
/// The literal 150000 is deliberate: __MAC_15_0 is undefined on SDKs older than 15
/// and would silently evaluate to 0, inverting this test.
#if defined(__APPLE__) && (__MAC_OS_X_VERSION_MIN_REQUIRED < 150000)
  #define SCIRUN_NO_QUICK_EXIT 1
#endif

namespace SCIRun
{
namespace Core
{

/// Terminates the process immediately, skipping static destructors and atexit
/// handlers, while still propagating @a code to the caller (e.g. CTest). Used in
/// regression mode to avoid hangs and crashes in async teardown paths where
/// streaming execution threads outlive the GUI objects.
///
/// Both branches below must stay behaviorally equivalent, which holds only so long
/// as nothing registers at_quick_exit handlers: _Exit does not run them.
[[noreturn]] inline void quickExit(int code)
{
#ifdef SCIRUN_NO_QUICK_EXIT
  std::_Exit(code);
#else
  std::quick_exit(code);
#endif
}

}}

#endif
