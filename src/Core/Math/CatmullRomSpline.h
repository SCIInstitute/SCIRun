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
///@class CatmullRomSpline
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///
///@date  March 1994
///

#ifndef SCI_Math_CatmullRomSpline_h
#define SCI_Math_CatmullRomSpline_h

#include <Core/Containers/Array1.h>

namespace SCIRun {


template<class T>
class CatmullRomSpline
{
public:
  CatmullRomSpline();
  CatmullRomSpline( const Array1<T>& );
  CatmullRomSpline( const int );
  CatmullRomSpline( const CatmullRomSpline<T>& );

  void setData( const Array1<T>& );
  void add( const T& );
  void insertData( const int, const T& );
  void removeData( const int );

  void clear();

  T operator()( double ) const; // 0-1
  T& operator[]( const int );

private:

  Array1<T> d;

};

} // End namespace SCIRun

////////////////////////////////////////////////////////////
// Start of included CatmullRomSpline.cc

#include <Core/Util/Assert.h>

namespace SCIRun {

template<class T>
CatmullRomSpline<T>::CatmullRomSpline() :
  d(0)
{
}

template<class T>
CatmullRomSpline<T>::CatmullRomSpline( const Array1<T>& data ) :
  d(data)
{
}

template<class T>
CatmullRomSpline<T>::CatmullRomSpline( const int n ) :
  d(n)
{
}

template<class T>
CatmullRomSpline<T>::CatmullRomSpline( const CatmullRomSpline& s ) :
  d(s.d)
{
}

template<class T>
void
CatmullRomSpline<T>::setData( const Array1<T>& data )
{
   d = data;
}

template<class T>
void
CatmullRomSpline<T>::clear()
{
  d.remove_all();
}

template<class T>
void
CatmullRomSpline<T>::add( const T& obj )
{
  d.add(obj);
}

template<class T>
void
CatmullRomSpline<T>::insertData( const int idx, const T& obj )
{
  d.insert(idx, obj);
}

template<class T>
void
CatmullRomSpline<T>::removeData( const int idx )
{
  d.remove(idx);
}

template<class T>
T
CatmullRomSpline<T>::operator()( double x ) const
{
   int    idx = (int)x;
   double t   = x - idx;

   double t2  = t * t;
   double t3  = t2 * t;

   int size = d.size();

   int idx1 = (idx-1+size) % size;
   int idx2 = (idx  +size) % size;
   int idx3 = (idx+1+size) % size;
   int idx4 = (idx+2+size) % size;

   T p0 = d[ idx1 ];
   T p1 = d[ idx2 ];
   T p2 = d[ idx3 ];
   T p3 = d[ idx4 ];

   T result = ( (p0*-1 + p1*3  + p2*-3 + p3   ) * (t3 * 0.5)+
                (p0*2  + p1*-5 + p2*4  + p3*-1) * (t2 * 0.5)+
                (p0*-1         + p2           ) * (t  * 0.5)+
                (        p1                   ) );

   return result;
}

template<class T>
T&
CatmullRomSpline<T>::operator[]( const int idx )
{
   return d[idx];
}

} // End namespace SCIRun


#endif /* SCI_Math_CatmullRomSpline_h */
