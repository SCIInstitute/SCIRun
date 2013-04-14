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
 *  Array3.h: Interface to dynamic 3D array class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 */

#ifndef CORE_CONAINTERS_ARRAY3_H
#define CORE_CONAINTERS_ARRAY3_H 1

#include <boost/multi_array.hpp>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <sci_defs/bits_defs.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Core/Persistent/Persistent.h>
#endif

#include <Core/Utils/Legacy/Assert.h>

namespace SCIRun {

template<class T> 
class Array3 
{
public:
  typedef boost::multi_array<T, 3> impl_type;
  typedef T value_type;

  void resize(size_t size1, size_t size2, size_t size3)
  {
    typename impl_type::extent_gen extents;
    impl_.resize(extents[size1][size2][size3]);
  }

  size_t size() const
  {
    return dim1() * dim2() * dim3();
  }

  T& operator[](size_t idx)
  {
    return impl_.origin()[idx];
  }

  const T& operator[](size_t idx) const
  {
    return impl_.origin()[idx];
  }

  inline size_t dim1() const {return impl_.shape()[0];}
  inline size_t dim2() const {return impl_.shape()[1];}
  inline size_t dim3() const {return impl_.shape()[2];}

private:
  impl_type impl_;
};

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

template<class T> void Pio(Piostream& stream, Array3<T>& array);
template<class T> void Pio(Piostream& stream, Array3<T>& array, const std::string&);
template<class T> void Pio(Piostream& stream, Array3<T>*& array);

#define ARRAY3_VERSION 2

template<class T>
void
Pio(Piostream& stream, Array3<T>& data)
{
  int version=stream.begin_class("Array3", ARRAY3_VERSION);

  if(stream.reading())
  {
    if (version < 2)
    {
      // Allocate the array
      int d1, d2, d3;
      Pio(stream, d1);
      Pio(stream, d2);
      Pio(stream, d3);
      data.resize(d1, d2, d3);      
    }
    else
    {
      // Allocate the array
      long long d1, d2, d3;
      Pio(stream, d1);
      Pio(stream, d2);
      Pio(stream, d3);
      data.resize(static_cast<size_type>(d1), 
                  static_cast<size_type>(d2),
                  static_cast<size_type>(d3));            
    }
  }
  else 
  {
    long long d1, d2, d3;
    d1 = static_cast<long long>(data.dm1);
    d2 = static_cast<long long>(data.dm2);
    d3 = static_cast<long long>(data.dm3);
    Pio(stream, d1);
    Pio(stream, d2);
    Pio(stream, d3);
  }
  
  if (stream.supports_block_io())
  {
    stream.block_io(reinterpret_cast<void*>(data.obj), sizeof(T), 
                    static_cast<size_t>(data.dm1*data.dm2*data.dm3));
  }
  else
  {
    for(index_type i=0;i<data.dm1;i++)
    {
      for(index_type j=0;j<data.dm2;j++)
      {
        for(index_type k=0;k<data.dm3;k++)
        {
          Pio(stream, data.objs[i][j][k]);
        }
      }
    }
  }
  stream.end_class();
}


template<class T>
void
Pio(Piostream& stream, Array3<T>*& data) 
{
  if (stream.reading()) 
  {
    data= new Array3<T>;
  }
  Pio(stream, *data);
}


template<class T>
void
Pio( Piostream& stream, Array3<T>& data, 
     const std::string& filename )
{
  int version=stream.begin_class("Array3", ARRAY3_VERSION);

  if(stream.reading())
  {
    if (version < 2)
    {
      // Allocate the array
      int d1, d2, d3;
      Pio(stream, d1);
      Pio(stream, d2);
      Pio(stream, d3);
      data.resize(d1, d2, d3);
      data.input( filename );
    }
    else
    {
      // Allocate the array
      long long d1, d2, d3;
      Pio(stream, d1);
      Pio(stream, d2);
      Pio(stream, d3);
      data.resize(static_cast<size_type>(d1), 
                  static_cast<size_type>(d2),
                  static_cast<size_type>(d3));
      data.input( filename );
    }
  }
  else 
  {
    long long d1, d2, d3;
    d1 = static_cast<long long>(data.dm1);
    d2 = static_cast<long long>(data.dm2);
    d3 = static_cast<long long>(data.dm3);
    Pio(stream, d1);
    Pio(stream, d2);
    Pio(stream, d3);
    data.output( filename );
  }
    
  stream.end_class();
}

template<class T>
int
Array3<T>::input( const std::string &filename ) 
{
  std::cerr << "Array3: Split input\n";

  // get raw data
  int file=open( filename.c_str(), O_RDONLY, 0666);
  if ( file == -1 ) {
    printf("can not open file %s\n", filename.c_str());
    return 0;
  }
  
  int maxiosz=1024*1024;
  size_type size = dm1*dm2*dm3*sizeof(T);
  int n = int(size / maxiosz);
  char *p = reinterpret_cast<char *>(objs[0][0]);

  for ( ; n> 0 ; n--, p+= maxiosz) 
  {
    int i = read( file, p, maxiosz);
    if ( i != maxiosz ) 
      perror( "io read ");
  }
  int i =  read( file, p, size % maxiosz);
  if ( i != (size % maxiosz) ) 
    perror("on last io");
        
  fsync(file);
  close(file);

  return 1;
}

template<class T>
int
Array3<T>::output( const std::string &filename ) 
{
  // get raw data
  //  printf("output [%s] [%s]\n", filename.c_str(), rawfile() );
  int file=open( filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if ( file == -1 ) {
    perror("open file");
    return 0;
  }
  
  int maxiosz=1024*1024;

  size_type size = dm1*dm2*dm3*sizeof(T);
  int n = size / maxiosz;
  char *p = reinterpret_cast<char *>(objs[0][0]);

  printf("Start writing...%d %d %d\n", size, maxiosz, n);

  for ( ; n> 0 ; n--, p+= maxiosz) {
    int l = write( file, p, maxiosz);
    if ( l != maxiosz ) 
      perror("write ");
  }
  int sz = (size % maxiosz );
  int l = write( file, p, sz); 
  if ( l != (size % maxiosz ) ) {
    printf("Error: wrote %d / %d\n", l,(size % maxiosz )); 
    perror("write ");
  }
        
  fsync(file);
  close(file);

  return 1;
} 
#endif

} // End namespace SCIRun

#endif // SCI_Containers_Array3_h

