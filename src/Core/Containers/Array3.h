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

#include <sci_defs/bits_defs.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <Core/Util/Assert.h>

#include <Core/Persistent/Persistent.h>


namespace SCIRun {

  template<class T> class Array3;

  template<class T> void Pio(Piostream& stream, Array3<T>& array);
  template<class T> void Pio(Piostream& stream, Array3<T>& array, const std::string&);
  template<class T> void Pio(Piostream& stream, Array3<T>*& array);

/**************************************

 CLASS
 Array3

 KEYWORDS
 Array3

 DESCRIPTION
 Array3.h: Interface to dynamic 3D array class

 Written by:
 Steven G. Parker
 Department of Computer Science
 University of Utah
 March 1994

 PATTERNS

 WARNING
  
****************************************/

template<class T> class Array3 {

public:
  // import scirun size and index types
  typedef SCIRun::index_type index_type;
  typedef SCIRun::size_type  size_type;
  typedef T value_type;

  //Default Constructor
  Array3();
  
  //Constructor
  Array3(size_type, size_type, size_type);

  // Copy Constructor
  Array3(const Array3& array);
    
  //////////
  //Class Destructor
  virtual ~Array3();
    
  //////////
  //Access the nXnXn element of the array
  inline T& operator()(index_type d1, index_type d2, index_type d3) const
  {
    ASSERTL3(d1>=0 && d1<dm1);
    ASSERTL3(d2>=0 && d2<dm2);
    ASSERTL3(d3>=0 && d3<dm3);
    return objs[d1][d2][d3];
  }

  //////////
  // Access the underlying data array as a 1D memory block 
  // Note: for performance reasons we do not do an assert here  
  inline const T& operator()(index_type d) const
  {
    return obj[d];
  }

  inline T& operator()(index_type d)
  {
    return obj[d];
  }

  inline const T& operator[](index_type d) const
  {
    return obj[d];
  }

  inline T& operator[](index_type d)
  {
    return obj[d];
  }

    
  //////////
  //Array3 Copy Method
  void copy(const Array3&);

  //////////
  //Returns the number of spaces in dim1	    
  inline size_type dim1() const {return dm1;}
  //////////
  //Returns the number of spaces in dim2
  inline size_type dim2() const {return dm2;}
  //////////
  //Returns the number of spaces in dim3
  inline size_type dim3() const {return dm3;}
  
  //Datasize larger than 2Gb only make sense when using 64bits only
  //In that case dm1, dm2 and dm3 are already 64bits 
  inline size_type get_datasize() const { return (dm1*dm2*dm3*sizeof(T)); }

  inline size_type size() const { return (dm1*dm2*dm3); }
    
  //////////
  //Re-size the Array
  void resize(size_type, size_type, size_type);

  void zero()
    { size_type sz = size(); for(size_type j=0; j<sz; j++) obj[j] = T(0); } 

  //////////
  //Initialize all elements to T
  void initialize(const T&);

  inline T*** get_dataptr() {return objs;}
  inline T*   get_datablock_ptr() {return obj;}

  //////////
  //read/write from a separate raw file
  int input( const std::string& );
  int output( const std::string&);

  friend void Pio <> (Piostream&, Array3<T>&);
  friend void Pio <> (Piostream&, Array3<T>&, const std::string &);
  friend void Pio <> (Piostream&, Array3<T>*&);

private:
  T*** objs;
  T* obj;
  size_type dm1;
  size_type dm2;
  size_type dm3;
  void allocate();

  //////////
  //Assignment Operator
  Array3<T>& operator=(const Array3&);
}; // end class Array3

template<class T>
Array3<T>::Array3() :
  objs(0), obj(0), dm1(0), dm2(0), dm3(0)
{
}

template<class T>
Array3<T>::Array3(const Array3<T>& array) :
  objs(0), obj(0), dm1(0), dm2(0), dm3(0)
{
  copy(array);
}


template<class T>
void
Array3<T>::allocate()
{
  if( (dm1>0) && (dm2>0) && (dm3>0) )
  {
    T** p;
    T* pp;  
    
    try
    {
      objs  = new T**[dm1];
      p     = new T*[dm1*dm2];
      pp    = new T[dm1*dm2*dm3];
    } 
    catch( std::bad_alloc ba ) 
    {
      size_t memsize = static_cast<size_t>(dm1*dm2*dm3);
      std::cerr << "Array3::allocate(): Could not allocate enough memory\n";
      std::cerr << "Array3 was trying to allocate ("<<dm1<<" x "<<dm2<<" x ";
      std::cerr << dm3 <<") "<< memsize << " values of size "<< sizeof(T) << "\n";
      
      objs = 0; // Keeps destructor from croaking allowing exception to propagate.
      throw ba;
    }
    
    obj = pp; // quick pointer into datablock
    for(index_type i=0;i<dm1;i++)
    {
      objs[i]=p;
      p+=dm2;
      for(index_type j=0;j<dm2;j++)
      {
        objs[i][j]=pp;
        pp+=dm3;
      }
    }
  } 
  else 
  {
    objs = 0;
    obj = 0;
  }
}

template<class T>
void
Array3<T>::resize(size_type d1, size_type d2, size_type d3)
{
  if(objs && dm1==d1 && dm2==d2 && dm3==d3) return;
  dm1=d1;
  dm2=d2;
  dm3=d3;
  if(objs)
  {
    delete[] objs[0][0];
    delete[] objs[0];
    delete[] objs;
  }
  allocate();
}

template<class T>
Array3<T>::Array3(size_type dm1, size_type dm2, size_type dm3) :
  objs(0), obj(0), dm1(dm1), dm2(dm2),dm3(dm3)
{
  allocate();
}

template<class T>
Array3<T>::~Array3()
{
  if(objs)
  {
    delete[] objs[0][0];
    delete[] objs[0];
    delete[] objs;
  }
}

template<class T>
void
Array3<T>::initialize(const T& t)
{
  ASSERT(objs != 0);
  for(index_type i=0;i<dm1;i++)
  {
    for(index_type j=0;j<dm2;j++)
    {
      for(index_type k=0;k<dm3;k++)
      {
        objs[i][j][k]=t;
      }
    }
  }
}

template<class T>
void
Array3<T>::copy(const Array3<T> &copy)
{
  resize( copy.dim1(), copy.dim2(), copy.dim3() );
  for(index_type i=0;i<dm1;i++)
    for(index_type j=0;j<dm2;j++)
      for(index_type k=0;k<dm3;k++)
        objs[i][j][k] = copy.objs[i][j][k];
}

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

} // End namespace SCIRun

#endif // SCI_Containers_Array3_h

