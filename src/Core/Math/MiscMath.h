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

#ifndef CORE_MATH_MISCMATH_H
#define CORE_MATH_MISCMATH_H 1

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
// ReSharper disable once CppUnusedIncludeDirective
#include <math.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <cmath>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <limits>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Math/share.h>

namespace SCIRun
{

template <class T, typename F>
T Interpolate(T d1, T d2, F weight)
{
  return T(d2 * weight + d1 * (1.0 - weight));
}

template <typename T>
bool IsNan(T val)
{
  return boost::math::isnan(val);
}

template <typename T>
bool IsFinite(T val)
{
  return boost::math::isfinite(val);
}

template <typename T>
bool IsInfinite(T val)
{
  return boost::math::isinf(val);
}

inline bool is_integral_value(double x)
{
  return (std::numeric_limits<size_type>::min)() <= x &&
         x <= (std::numeric_limits<size_type>::max)() && x == static_cast<size_type>(x);
}

}

#endif
