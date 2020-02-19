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
#include <math.h>
#include <cmath>
#include <limits>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <Core/Math/share.h>

// Define missing Windows function
#ifdef _WIN32
inline double acosh(double x)
{
	return (x<1) ? log(-1.0) : log(x+sqrt(x*x-1));
}
#endif

namespace SCIRun {

	#if BOOST_VERSION >= 106700
	namespace btt = boost::math::fpc;
	#else
	namespace btt = boost::test_tools;
	#endif

template<typename T>
inline bool nonzero(T d)
{
  btt::close_at_tolerance<T> comp(btt::percent_tolerance(std::numeric_limits<T>::epsilon()));
  return(! comp(d, 0));
}

// 3 unsigned long long values
inline unsigned long long Min(unsigned long long d1, unsigned long long d2, unsigned long long d3)
{
    unsigned long long m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline unsigned long long Mid(unsigned long long a, unsigned long long b, unsigned long long c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) : \
	            ((b < c) ? b : ((a > c) ? a : c)));
}

inline unsigned long long Max(unsigned long long d1, unsigned long long d2, unsigned long long d3)
{
    unsigned long long m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

// 3 doubles
inline double Min(double d1, double d2, double d3)
{
    double m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline double Mid(double a, double b, double c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) :
	            ((b < c) ? b : ((a > c) ? a : c)));
}

inline double Max(double d1, double d2, double d3)
{
    double m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

// 3 integers
inline int Min(int d1, int d2, int d3)
{
    int m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline int Mid(int a, int b, int c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) :
	            ((b < c) ? b : ((a > c) ? a : c)));
}

inline int Max(int d1, int d2, int d3)
{
    int m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

// 3 unsigned integers
inline unsigned int Min(unsigned int d1,
			unsigned int d2,
			unsigned int d3)
{
    unsigned int m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline unsigned int Mid(unsigned int a,
			unsigned int b,
			unsigned int c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) :
	            ((b < c) ? b : ((a > c) ? a : c)));
}

inline unsigned int Max(unsigned int d1,
			unsigned int d2,
			unsigned int d3)
{
    unsigned int m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

// 3 Long Integers
inline long  Min(long  d1, long  d2, long  d3)
{
    long  m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline long Mid(long a, long b, long c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) :
	            ((b < c) ? b : ((a > c) ? a : c)));
}


inline long  Max(long  d1, long  d2, long  d3)
{
    long  m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

// 3 Long Integers
inline long long  Min(long long  d1, long long  d2, long long  d3)
{
    long long m=d1<d2?d1:d2;
    m=m<d3?m:d3;
    return m;
}

inline long long Mid(long long a, long long b, long long c)
{
  return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) :
	            ((b < c) ? b : ((a > c) ? a : c)));
}


inline long long Max(long long  d1, long long  d2, long long d3)
{
    long long m=d1>d2?d1:d2;
    m=m>d3?m:d3;
    return m;
}

//-----------------------------------------
// Sign function
inline double Abs(double d)
{
  return d < 0 ? -d : d;
}

inline float Abs(float f)
{
  return f < 0 ? -f : f;
}

inline int Abs(int i)
{
  return i < 0 ? -i : i;
}

//-----------------------------------------
// Sign function
inline int Sign(double d)
{
  return d<0.0?-1:1;
}

inline int Sign(float f)
{
  return f<0.0?-1:1;
}

inline int Sign(int i)
{
  return i<0?-1:1;
}

//-----------------------------------------
// Interpolation function
template <class T>
inline T Interpolate(T d1, T d2, double weight)
{
  return T(d2*weight+d1*(1.0-weight));
}

template <class T>
inline T Interpolate(T d1, T d2, float weight)
{
  return T(d2*weight+d1*(1-weight));
}

inline int Round(double d)
{
  return static_cast<int>(d+0.5);
}

inline int Round(float f)
{
  return static_cast<int>(f+0.5);
}

inline int Floor(double d)
{
  if(d<0)
  {
    int i=-static_cast<int>(-d);
    if(static_cast<double>(i) == d)
      return i;
    else
      return i-1;
  }
  else
  {
    return (static_cast<int>(d));
  }
}

inline int Floor(float f)
{
  if(f<0)
  {
    int i=-static_cast<int>(-f);
    if(static_cast<float>(i) == f)
      return (i);
    else
      return (i-1);
  }
  else
  {
    return (static_cast<int>(f));
  }
}

inline bool IsNan(double val)
{
  return boost::math::isnan(val);
}

inline bool IsFinite(double val)
{
  return boost::math::isfinite(val);
}

inline bool IsInfinite(double val)
{
  return boost::math::isinf(val);
}

inline bool is_integral_value(double x)
{
  return std::numeric_limits<SCIRun::size_type>::min() <= x &&
    x <= std::numeric_limits<SCIRun::size_type>::max() &&
    x == static_cast<SCIRun::size_type>(x);
}

} // End namespace SCIRun


#endif
