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

// import index_type and  size_type
#include <Core/Datatypes/Types.h>
#include <Core/Persistent/Persistent.h>

#include <Core/Util/Assert.h>

namespace SCIRun {

template<class T> class Array1;
template<class T> void Pio(Piostream& stream, Array1<T>& array);

/**************************************

  CLASS
  Array1

  KEYWORDS
  Array1

  DESCRIPTION
  Array1.h: Interface to dynamic 1D array class

  Written by:
  Steven G. Parker
  Department of Computer Science
  University of Utah
  March 1994

  PATTERNS

  WARNING
  
****************************************/
template<class T> class Array1 {

public:
  // import scirun size and index types
  typedef SCIRun::index_type index_type;
  typedef SCIRun::size_type  size_type;
  typedef T                  value_type;

  //////////
  //Copy the array - this can be costly, so try to avoid it.
  Array1(const Array1&);

  //////////
  //Deletes the array and frees the associated memory
  virtual ~Array1();

  //////////
  //Make a new array 1. <i>size</i> gives the initial size of the array,
  //<i>default_grow_size</i> indicates the minimum number of objects that
  //should be added to the array at a time.  <i>asize</i> tells how many
  //objects should be allocated initially
  Array1(size_type size=0, size_type default_grow_size=10, size_type asize=0);

  //////////
  //Copy over the array - this can be costly, so try to avoid it.
  Array1<T>& operator=(const Array1&);

  //////////
  //Compare over the array - this can be costly, so try to avoid it.
  bool operator==(const Array1<T>&) const;
  bool operator!=(const Array1<T>&) const;

    
  //////////
  // Accesses the nth element of the array
  inline const T& operator[](index_type n) const 
  {
    CHECKARRAYBOUNDS(n, 0, dm1);
    return objs[n];
  }

  //////////
  // Accesses the nth element of the array
  inline T& operator[](index_type n) 
  {
    CHECKARRAYBOUNDS(n, 0, dm1);
    return objs[n];
  }
    
  //////////
  // Returns the size of the array
  inline size_type size() const{ return dm1;}

  //////////
  //Returns the number of spaces in dm1	    
  inline size_type dim1() const {return dm1;}

  //////////
  // Make the array larger by count elements
  void grow(size_type count, size_type grow_size=10);

  //////////
  // set allocated size 
  void reserve(size_type n);

  //////////
  // Add one element to the array.  equivalent to:
  //  grow(1)
  //  array[array.size()-1]=data
  void add(const T&);

  //////////
  // Insert one element into the array.  This is very inefficient
  // if you insert anything besides the (new) last element.
  void insert(index_type, const T&);


  //////////
  // Remove one element from the array.  This is very inefficient
  // if you remove anything besides the last element.
  void remove(index_type);


  //////////
  // Remove all elements in the array.  The array is not freed,
  // and the number of allocated elements remains the same.
  void remove_all();


  //////////
  // Change the size of the array.
  void resize(size_type newsize);

  //////////
  // Changes size, makes exact if currently smaller...
  void setsize(size_type newsize);

  //////////
  // This will allocate an array which is equal to the number of
  // elements in the array and copy the values over.
  //
  // dm1 is not changed.
  //
  // newsize is an optional parameter which indicates the desired
  // size.  If newsize is greater than dm1 the new array will have
  // newsize elements in it (nalloc = newsize).  If newsize is less
  // than dm1 then dm1 elemets will be allocated (nalloc = dm1).
  void trim(size_type newsize=0);

  //////////
  // Initialize all elements of the array
  void initialize(const T& val);


  //////////
  // Get the array information
  T* get_objs();

  friend void Pio <> (Piostream&, Array1<T>&);


private:
  T* objs;
  size_type dm1;
  size_type nalloc;
  size_type default_grow_size;

};

template<class T>
Array1<T>::Array1(const Array1<T>& a)
{
  dm1=a.dm1;
  nalloc=dm1;
  objs=new T[dm1];
  for(index_type i=0;i<dm1;i++)objs[i]=a.objs[i];
  nalloc=dm1;
  default_grow_size=a.default_grow_size;
}

template<class T>
Array1<T>& Array1<T>::operator=(const Array1<T>& copy)
{
  if (&copy == this)
  {
    // Doing A=A, so don't do anything
    return (*this);
  }
  setsize(copy.dm1);
  for(index_type i=0;i<dm1;i++)objs[i]=copy.objs[i];
  default_grow_size=copy.default_grow_size;
  return(*this);
}

template<class T>
bool Array1<T>::operator==(const Array1<T>& a) const
{
  for(index_type i=0;i<dm1;i++)
    if( objs[i]!=a.objs[i] )
      return (false);
  return (true);
}

template<class T>
bool Array1<T>::operator!=(const Array1<T>& a) const
{
  for(index_type i=0;i<dm1;i++)
    if( objs[i]!=a.objs[i] )
      return (true);
  return (false);
}

template<class T>
Array1<T>::Array1(size_type size, size_type gs, size_type asize)
{
  ASSERT(size >= 0);
  default_grow_size=gs;
  dm1=size;
  if(size)
  {
    if(asize <= size)
    {
      objs=new T[size];
      nalloc=dm1;
    } 
    else 
    {
      objs=new T[asize];
      nalloc=asize;
    }
  } 
  else 
  {
    if(asize > 0)
    {
      objs=new T[asize];
      nalloc=asize;
    } 
    else 
    {
      objs=0;
      nalloc=0;
    }
  }
}	

template<class T>
Array1<T>::~Array1()
{
  if(objs) delete [] objs;
}

template<class T>
void Array1<T>::grow(size_type count, size_type grow_size)
{
  size_type newsize=dm1+count;
  if(newsize>nalloc)
  {
    // Reallocate...
    size_type gs1=newsize>>2;
    size_type gs=gs1>grow_size?gs1:grow_size;
    size_type newalloc=newsize+gs;
    T* newobjs=new T[newalloc];
    if(objs)
    {
      for(index_type i=0;i<dm1;i++)
      {
        newobjs[i]=objs[i];
      }
      delete[] objs;
    }
    objs=newobjs;
    nalloc=newalloc;
  }
  dm1=newsize;
}

template<class T>
void Array1<T>::reserve(size_type n)
{
  if(n>nalloc)
  {
    // Reallocate...
    T* newobjs=new T[n];
    if(objs)
    {
      for(index_type i=0;i<dm1;i++)
      {
        newobjs[i]=objs[i];
      }
      delete[] objs;
    }
    objs=newobjs;
    nalloc=n;
  }
}

template<class T>
void Array1<T>::add(const T& obj)
{
  grow(1, default_grow_size);
  objs[dm1-1]=obj;
}

template<class T>
void Array1<T>::insert(index_type idx, const T& obj)
{
  grow(1, default_grow_size);
  for(index_type i=dm1-1;i>idx;i--)objs[i]=objs[i-1];
  objs[idx]=obj;
}

template<class T>
void Array1<T>::remove(index_type idx)
{
  dm1--;
  for(index_type i=idx;i<dm1;i++)objs[i]=objs[i+1];
}

template<class T>
void Array1<T>::remove_all()
{
  dm1=0;
}

template<class T>
void Array1<T>::resize(size_type newsize)
{
  if(newsize > dm1)
    grow(newsize-dm1);
  else
    dm1=newsize;
}

template<class T>
void Array1<T>::setsize(size_type newsize)
{ 
  if(newsize > nalloc) 
  { // have to reallocate...
    T* newobjs=new T[newsize];     // make it exact!
    if (objs) 
    {
      for(index_type i=0;i<dm1;i++)
      {
        newobjs[i]=objs[i];
      }
      delete[] objs;
    }		
    objs = newobjs;
    nalloc = newsize;
  }
  dm1=newsize;
}

template<class T>
void Array1<T>::trim(size_type newsize)
{
  if (newsize < dm1 || newsize <= 0)
    newsize = dm1;
  if (newsize == nalloc)
    // We already have the correct number allocated
    return;
  T* newobjs = new T[newsize];
  if (objs) 
  {
    // Copy the data
    for(index_type i=0;i<dm1;i++)
    {
      newobjs[i]=objs[i];
    }
    // Delete the old bit of memory
    delete[] objs;
  }		
  objs = newobjs;
  nalloc = newsize;
}

template<class T>
void Array1<T>::initialize(const T& val) 
{
  for (index_type i=0;i<dm1;i++)objs[i]=val;
}

template<class T>
T* Array1<T>::get_objs()
{
  return objs;
}

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

} // End namespace SCIRun


#endif

