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
 *  Array2.h: Interface to dynamic 2D array class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 */

#ifndef CORE_CONAINTERS_ARRAY2_H
#define CORE_CONAINTERS_ARRAY2_H 1

// import index_type and  size_type
#include <Core/Datatypes/Types.h>
#include <Core/Persistent/Persistent.h>

#include <Core/Util/Assert.h>

namespace SCIRun {

template<class T> class Array2;

template<class T> void Pio(Piostream& stream, Array2<T>& data);
template<class T> void Pio(Piostream& stream, Array2<T>*& data);

/**************************************

 CLASS
 Array2

 KEYWORDS
 Array2

 DESCRIPTION
 Array2.h: Interface to dynamic 2D array class

 Written by:
 Steven G. Parker
 Department of Computer Science
 University of Utah
 March 1994

 PATTERNS

 WARNING
  
****************************************/
template<class T>
class Array2 {

public:
  // import scirun size and index types
  typedef SCIRun::index_type index_type;
  typedef SCIRun::size_type  size_type;
  typedef T                  value_type;
  
  //////////
  //Create a 0X0 Array
  Array2();

  Array2(const Array2& array);
    
  //////////
  //Create an n by n array
  Array2(size_type, size_type);

  //////////
  //Class Destructor
  virtual ~Array2();

  //////////
  //Used for accessing elements in the Array
  inline T& operator()(index_type d1, index_type d2) const
  {
    ASSERTL3(d1>=0 && d1<dm1);
    ASSERTL3(d2>=0 && d2<dm2);
    return objs[d1][d2];
  }
  
  //////////
  // Access the underlying data array as a 1D memory block 
  // Note: for performance reasons we do not do an assert here  

  inline T& operator()(index_type d) const
  {
    return obj[d];
  }

  inline T& operator[](index_type d) const
  {
    return obj[d];
  }
    
  //////////
  //Array2 Copy Method
  void copy(const Array2&);
    
  //////////
  //Returns number of rows
  inline size_type dim1() const {return dm1;}
    
  //////////
  //Returns number of cols
  inline size_type dim2() const {return dm2;}
    
  //////////
  //Resize Array
  void resize(size_type, size_type);

  inline size_type size() const { return (static_cast<size_type>(dm1*dm2)); }

  void zero()
    { size_type sz = size(); for(size_type j=0; j<sz; j++) obj[j] = T(0); } 

  //////////
  //Initialize all values in an array
  void initialize(const T&);

  inline T** get_dataptr() {return objs;}
  inline T*  get_datablock_ptr() {return obj;}

  friend void Pio <> (Piostream&, Array2<T>&);
  friend void Pio <> (Piostream&, Array2<T>*&);

private:

  //////////
  //Assignment Operator
  Array2<T>& operator=(const Array2&);

  //////////
  // Pointers to the data
  T** objs;
  T* obj;
  
  //////////
  // Size of the array
  size_type dm1;
  size_type dm2;
  void allocate();


};

template<class T>
Array2<T>::Array2() :
  objs(0), obj(0), dm1(0), dm2(0)
{
}

template<class T>
Array2<T>::Array2(const Array2<T>& array) :
  objs(0), obj(0), dm1(0), dm2(0)
{
  copy(array);
}


template<class T>
void Array2<T>::allocate()
{
  if(dm1 == 0 || dm2 == 0)
  {
    objs=0;
    obj=0;
  } 
  else 
  {
    objs=new T*[dm1];
    T* p=new T[dm1*dm2];
    obj = p;
    for(index_type i=0;i<dm1;i++)
    {
      objs[i]=p;
      p+=dm2;
    }
  }
}

template<class T>
void Array2<T>::resize(size_type d1, size_type d2)
{
  if(objs && dm1==d1 && dm2==d2) return;
  dm1=d1;
  dm2=d2;
  if(objs)
  {
    delete[] objs[0];
    delete[] objs;
  }
  allocate();
}

template<class T>
Array2<T>::Array2(size_type dm1, size_type dm2)
  : dm1(dm1), dm2(dm2)
{
  allocate();
}

template<class T>
Array2<T>::~Array2()
{
  if(objs)
  {
    delete[] objs[0];
    delete[] objs;
  }
}

template<class T>
void Array2<T>::initialize(const T& t)
{
  ASSERT(dm1==0 || dm2==0 || objs != 0);
  for(index_type i=0;i<dm1;i++)
  {
    for(index_type j=0;j<dm2;j++)
    {
      objs[i][j]=t;
    }
  }
}

template<class T>
void Array2<T>::copy(const Array2<T> &copy)
{
  resize( copy.dim1(), copy.dim2() );
  for(index_type i=0;i<dm1;i++)
    for(index_type j=0;j<dm2;j++)
      objs[i][j] = copy.objs[i][j];
}

#define ARRAY2_VERSION 2

template<class T>
void Pio(Piostream& stream, Array2<T>& data)
{
  int version = stream.begin_class("Array2", ARRAY2_VERSION);
  if(stream.reading())
  {
    // Allocate the array...
    if (version < 2)
    {
      int d1, d2;
      Pio(stream, d1);
      Pio(stream, d2);
      data.resize(static_cast<size_type>(d1), static_cast<size_type>(d2));
    }
    else
    {
      long long d1, d2;
      Pio(stream, d1);
      Pio(stream, d2);
      data.resize(static_cast<size_type>(d1), static_cast<size_type>(d2));
    }
  } 
  else 
  {
    long long d1 = static_cast<long long>(data.dm1);
    long long d2 = static_cast<long long>(data.dm2);
    Pio(stream, d1);
    Pio(stream, d2);
  }
  if (stream.supports_block_io())
  {
    stream.block_io(data.obj,sizeof(T),data.dm1*data.dm2);
  }
  else
  {
    for(index_type i=0;i<data.dm1;i++)
    {
      for(index_type j=0;j<data.dm2;j++)
      {
        Pio(stream, data.objs[i][j]);
      }
    }
  }
  stream.end_class();
}

template<class T>
void Pio(Piostream& stream, Array2<T>*& data) 
{
  if (stream.reading()) 
  {
    data=new Array2<T>;
  }
  Pio(stream, *data);
}

} // End namespace SCIRun

#endif

