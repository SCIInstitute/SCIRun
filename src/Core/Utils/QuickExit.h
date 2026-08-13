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

namespace SCIRun
{
namespace Core
{

/// Terminates the process immediately, skipping static destructors and atexit
/// handlers, while still propagating @a code to the caller (e.g. CTest). Used in
/// regression mode to avoid hangs and crashes in async teardown paths where
/// streaming execution threads outlive the GUI objects.
///
/// Apple always takes _Exit so that one build runs on every supported macOS.
/// quick_exit entered libSystem in macOS 15.0, which breaks both directions:
///
///   - Against SDKs older than 15, it is not declared at all. Both spellings fail to
///     compile -- "quick_exit" as an undeclared identifier, "std::quick_exit" as a
///     reference to an unresolved using declaration, since libc++'s <cstdlib> has no
///     ::quick_exit to pull in.
///   - Against the 15 SDK it is declared ("void quick_exit(int) __dead2;" in
///     _stdlib.h) gated only on __DARWIN_C_LEVEL/C11 -- a compile-time feature gate
///     carrying no availability annotation -- so the compiler can neither warn nor
///     weak-link. The object gets a hard _quick_exit reference and aborts on load on
///     macOS < 15 with "dyld: Symbol not found: _quick_exit".
///
/// The trigger is the SDK, not the host: a macOS 14 machine with Xcode 16 installed
/// builds the broken binary. Gating on __MAC_OS_X_VERSION_MIN_REQUIRED does not help
/// either, because the deployment target is not pinned -- it follows the build host,
/// and so reads 150000 on exactly the builders that emit the hard reference.
///
/// The two branches are equivalent only so long as nothing registers at_quick_exit
/// handlers: _Exit does not run them.
[[noreturn]] inline void quickExit(int code)
{
#ifdef __APPLE__
  std::_Exit(code);
#else
  std::quick_exit(code);
#endif
}

}}

#endif
