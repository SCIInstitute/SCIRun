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



#ifndef CORE_MATH_MISCMATH_H
#define CORE_MATH_MISCMATH_H 1

#include <cmath>
#include <limits>
#include <Core/Datatypes/Types.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <Core/Math/share.h>

#include <boost/test/floating_point_comparison.hpp>
namespace btt=boost::test_tools;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define missing Windows function
#ifdef _WIN32
inline double acosh(double x)
{
	return (x<1) ? log(-1.0) : log(x+sqrt(x*x-1));
}
#endif

namespace SCIRun {

template<typename T>
inline bool nonzero(T d)
{
  btt::close_at_tolerance<T> comp(btt::percent_tolerance(std::numeric_limits<T>::epsilon()));
  return(! comp(d, 0));
}
  
// Faster versions of several math functions

//------------------------------------------
// Min/Max functions
//
// 2 unsigned long long values
inline unsigned long long Min(unsigned long long d1, unsigned long long d2)
{
    return d1<d2?d1:d2;
}

inline unsigned long long Max(unsigned long long d1, unsigned long long d2)
{
    return d1>d2?d1:d2;
}

// 2 Integers
inline int Min(int d1, int d2)
{
    return d1<d2?d1:d2;
}

inline int Max(int d1, int d2)
{
    return d1>d2?d1:d2;
}

// 2 Unsigned Integers
inline unsigned int Min(unsigned int d1, unsigned int d2)
{
    return d1<d2?d1:d2;
}

inline unsigned int Max(unsigned int d1, unsigned int d2)
{
    return d1>d2?d1:d2;
}

// 2 Long Integers
inline long Min(long d1, long d2)
{
    return d1<d2?d1:d2;
}

inline long Max(long d1, long d2)
{
    return d1>d2?d1:d2;
}


// 2 Long Long Integers
inline long long Min(long long d1, long long d2)
{
    return d1<d2?d1:d2;
}

inline long long Max(long long d1, long long d2)
{
    return d1>d2?d1:d2;
}

// 2 floats
inline float Max(float d1, float d2)
{
  return d1>d2?d1:d2;
}

inline float Min(float d1, float d2)
{
  return d1<d2?d1:d2;
}

// 2 doubles
inline double Max(double d1, double d2)
{
  return d1>d2?d1:d2;
}

inline double Min(double d1, double d2)
{
  return d1<d2?d1:d2;
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

//------------------------------------------
// Power function

inline double Pow(double d, double p)
{
  return pow(d,p);
}

inline double Pow(double x, unsigned int p)
{
  double result=1;
  while(p)
  {
    if(p&1) result*=x;
    x*=x;
    p>>=1;
  }
  return result;
}

inline double Pow(double x, int p)
{
  if(p < 0)
  {
    p=-p;
    double result=1;
    while(p)
    {
      if(p&1)
        result*=x;
      x*=x;
      p>>=1;
    }
    return 1./result;
  } 
  else
  { 
    double result=1;
    while(p){
      if(p&1)
        result*=x;
      x*=x;
      p>>=1;
    }
    return result;
  }
}

//------------------------------------------
// Sqrt and Cbrt function

inline int Sqrt(int i)
{
  return static_cast<int>(sqrt(static_cast<double>(i)));
}

inline double Sqrt(double d)
{
  return sqrt(d);
}

inline float Sqrt(float d)
{
  return sqrtf(d);
}

inline double Cbrt(double d)
{
  return pow(d, 1.0/3.0);
}

inline float Cbrt(float d)
{
  return static_cast<float>(pow(static_cast<double>(d), 1.0/3.0));
}

inline double Sqr(double x)
{
  return x*x;
}

inline float Sqr(float x)
{
  return x*x;
}
  
//------------------------------------------
// Absolute function
inline double Abs(double d)
{
  return d<0?-d:d;
}

inline float Abs(float f)
{
  return f<0?-f:f;
}

inline int Abs(int i)
{
    return i<0?-i:i;
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
// Clamp function
inline double Clamp(double d, double min, double max)
{
  return d<=min?min:d>=max?max:d;
}

inline float Clamp(float f, float min, float max)
{
  return f<=min?min:f>=max?max:f;
}

inline int Clamp(int i, int min, int max)
{
  return i<min?min:i>max?max:i;
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

// this version is twice as fast for negative numbers
// (on an available Pentium 4 machine, only about 25%
// faster on Powerbook G4)
// than the more robust version above, but will only
//work for fabs(d) <= offset, use with caution
inline int Floor(double d, int offset)
{
  return (static_cast<int>((d + offset) - offset));
}

inline int Floor(float f, int offset)
{
   return (static_cast<int>((f + offset) - offset));
}

inline int Ceil(double d)
{
  if(d<0)
  {
    int i=-static_cast<int>(-d);
    return (i);
  } 
  else 
  {
    int i=static_cast<int>(d);
    if(static_cast<double>(i) == d)
      return (i);
    else
      return (i+1);
  }
}

inline int Ceil(float f)
{
  if(f<0)
  {
    int i=-static_cast<int>(-f);
    return (i);
  } 
  else 
  {
    int i=static_cast<int>(f);
    if(static_cast<float>(i) == f)
      return (i);
    else
      return (i+1);
  }
}

// using the same trick as above, this
// version of Ceil is a bit faster for
// the architectures it has been tested
// on (Pentium4, Powerbook G4)
inline int Ceil(double d, int offset)
{
  return (static_cast<int>((d - offset) + offset));
}

inline int Ceil(float f, int offset)
{
   return (static_cast<int>((f - offset) + offset));
}

inline bool IsNan(double val)
{
  return boost::math::isnan(val);
}

inline bool IsFinite(double val)
{
  return boost::math::isfinite(val);
}

// TODO: replace with boost
inline bool IsInfinite(double val)
{
  return boost::math::isinf(val);
}

// Fast way to check for power of two
inline bool IsPowerOf2(unsigned int n)
{
  return ((n & (n-1)) == 0);
}

inline bool is_integral_value(double x)
{
  return std::numeric_limits<SCIRun::size_type>::min() <= x && 
    x <= std::numeric_limits<SCIRun::size_type>::max() && 
    x == static_cast<SCIRun::size_type>(x);
}

// Returns a number Greater Than or Equal to dim
// that is an exact power of 2
// Used for determining what size of texture to
// allocate to store an image
inline unsigned int
Pow2(const unsigned int dim) 
{
  if (IsPowerOf2(dim)) return dim;
  unsigned int val = 4;
  while (val < dim) val = val << 1;;
  return val;
}

// Returns a number Less Than or Equal to dim
// that is an exact power of 2
inline unsigned int
LargestPowerOf2(const unsigned int dim) 
{
  if (IsPowerOf2(dim)) return dim;
  return Pow2(dim) >> 1;
}

// Returns the power of 2 of the next higher number that is a power of 2
// Log2 of Pow2 function above
inline unsigned int
Log2(const unsigned int dim) 
{
  unsigned int log = 0;
  unsigned int val = 1;
  while (val < dim) { val = val << 1; log++; };
  return log;
}

// Takes the square root of "value" and tries to find two factors that
// are closest to that square root.
void findFactorsNearRoot(const int value, int& factor1, int& factor2);

inline double Cot(double d)
{
  return 1./tan(d);
}

inline double DtoR(double d)
{
  return d*(M_PI/180.);
}

inline double RtoD(double r)
{
  return r*(180./M_PI);
}

inline float DtoR(float d)
{
  return d*static_cast<float>(M_PI/180.0);
}

inline float RtoD(float r)
{
  return r*static_cast<float>(180.0/M_PI);
}

} // End namespace SCIRun


#endif
