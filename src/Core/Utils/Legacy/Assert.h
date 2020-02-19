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


///
///@file  Assert.h
///@brief Utility for specifying data invariants (Assertions)
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  Feb. 1994
///

#ifndef SCI_Containers_Assert_h
#define SCI_Containers_Assert_h 1

#include <sci_defs/error_defs.h>

#include <Core/Exceptions/AssertionFailed.h>
#include <Core/Exceptions/ArrayIndexOutOfBounds.h>

/// @todo: make sure default SCI_ASSERTION_LEVEL is consistent across platforms

#define ASSERTFAIL(string) \
   SCI_THROW(SCIRun::AssertionFailed(string, __FILE__, __LINE__));

#define ASSERTMSG_LEGACY(condition,message) \
   if(!(condition)){ \
      SCI_THROW(SCIRun::AssertionFailed(message, __FILE__, __LINE__)); \
   }

#if SCI_ASSERTION_LEVEL >= 1
#  define IFASSERT(x) x
#  define ASSERTL1(condition) \
     if(!(condition)){ \
        SCI_THROW(SCIRun::AssertionFailed(#condition, __FILE__, __LINE__)); \
     }
#else
#  define ASSERTL1(condition)
#endif

#if SCI_ASSERTION_LEVEL >= 2
#  define ASSERTL2(condition) \
     if(!(condition)){ \
        SCI_THROW(SCIRun::AssertionFailed(#condition, __FILE__, __LINE__)); \
     }
#else
#  define ASSERTL2(condition)
#endif

#if SCI_ASSERTION_LEVEL >= 3
#  define ASSERTL3(condition) \
     if(!(condition)){ \
        SCI_THROW(SCIRun::AssertionFailed(#condition, __FILE__, __LINE__)); \
     }
#  define CHECKARRAYBOUNDS(value, lower, upper) \
     if(value < lower || value >= upper){ \
        SCI_THROW(SCIRun::ArrayIndexOutOfBounds(value, lower, upper, __FILE__, __LINE__)); \
     }
#else
#  define ASSERTL3(condition)
#  define CHECKARRAYBOUNDS(value, lower, upper)
#endif

#if SCI_ASSERTION_LEVEL == 0
#  define USE_IF_ASSERTS_ON(line)
#  define ASSERTL1(condition)
#  define ASSERTL2(condition)
#  define ASSERTL3(condition)
#  define ASSERTEQ(c1, c2)
#  define ASSERTRANGE(c, l, h)
#  define IFASSERT(x)
#else
#  define USE_IF_ASSERTS_ON(line) line
#endif

/* USE_IF_ASSERTS_ON allows us to remove lines that are necessary for
   code that is needed if asserts are on but causes warnings if
   asserts are off (ie: in optimized builds).  All it does it remove
   the line or put the line in. */

#define ASSERT(condition) ASSERTL2(condition)

#endif
