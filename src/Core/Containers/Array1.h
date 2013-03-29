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



/*
 *  Array1.h: Interface to dynamic 1D array class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 */

#ifndef CORE_CONAINTERS_ARRAY1_H
#define CORE_CONAINTERS_ARRAY1_H 1

#include <vector>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Persistent/Persistent.h>
#endif

namespace SCIRun {

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template<class T> class Array1;
template<class T> void Pio(Piostream& stream, Array1<T>& array);
#endif

#define Array1 std::vector
//template<class T> 
//class Array1 : public std::vector<T>
//{
//};


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#define ARRAY1_VERSION 3

template<class T>
void Pio(Piostream& stream, Array1<T>& array)
{
  int version= stream.begin_class("Array1", ARRAY1_VERSION);
  size_type size;
  if (version < 3)
  {
    int sz;
    Pio(stream,sz);
    size = static_cast<size_type>(sz);
  }
  else
  {
    long long sz = static_cast<long long>(array.dm1);
    Pio(stream,sz);
    size = static_cast<size_type>(sz);
  }
  
  if(stream.reading())
  {
    array.remove_all();
    array.grow(size);
  }

  if (stream.supports_block_io())
  {
    stream.block_io(array.objs,sizeof(T),size);
  }
  else
  {
    for(index_type i=0;i<size;i++)
      Pio(stream, array.objs[i]);
  }
  stream.end_class();
}

template<class T>
void Pio(Piostream& stream, Array1<T>*& array) 
{
  if (stream.reading())
    array=new Array1<T>;
  Pio(stream, *array);
}
#endif

} // End namespace SCIRun


#endif

