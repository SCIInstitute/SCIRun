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



///
///@class MiscMath
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///
///@date  June 2004
///

#include <Core/Math/MiscMath.h>

#ifdef _WIN32
#include <float.h>
#define finite _finite
#endif

namespace SCIRun {

void findFactorsNearRoot(const int value, int &factor1, int &factor2) 
{
  int f1,f2;
  f1 = f2 = (int) Sqrt((double)value);
  // now we are basically looking for a pair of multiples that are closest to
  // the square root.
  while ((f1 * f2) != value) {
    // look for another multiple
    for(int i = f1+1; i <= value; i++) {
      if (value%i == 0) {
        // we've found a root
        f1 = i;
        f2 = value/f1;
        break;
      }
    }
  }
  factor1 = f1;
  factor2 = f2;
}

} // namespace SCIRun
