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

// This implements a subclass of the std::vector class, except that
// the vector is statically allocated on the stack for performance.

#ifndef CORE_CONTAINERS_STACKBASEDVECTOR_H
#define CORE_CONTAINERS_STACKBASEDVECTOR_H 1

#include <memory>
#include <vector>

//! This vector type is similar in preformance to the StackVector, but checks
//! for overflow. In case of overflow memory will be reserved to store data_ 


// TODO: replace with STL container

namespace SCIRun {

template<class T, int CAPACITY>
class StackBasedVector {
public:
  typedef T                 value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type*       iterator;
  typedef const value_type* const_iterator;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;

  StackBasedVector() 
  { 
    initialize(); 
  }

  ~StackBasedVector() 
  {
    if(data_ != &(fixed_[0]))
      delete[] data_;
  }

  StackBasedVector(size_type n) 
  { 
    initialize(); 
    resize(n); 
  }
  
  StackBasedVector(size_type n, T val) 
  { 
    initialize(); 
    resize(n); 
    for (size_t i=0;i<n;i++) 
    data_[i] = val; 
  }

  void initialize() 
  { 
    curalloc_=CAPACITY; 
    cursize_=0; 
    data_=&(fixed_[0]); 
  }

  inline iterator begin() 
  { 
    return data_; 
  }
  
  inline const_iterator begin() const 
  { 
    return data_; 
  }
  
  inline iterator end() 
  { 
    return data_+cursize_; 
  }
  
  inline const_iterator end() const 
  { 
    return data_+cursize_;
  }

  inline void reserve(size_type s) 
  { 
    enlarge(s); 
  }
    

  inline void clear()
  { 
    cursize_ = 0; 
  }

  inline void push_back(const T& x) 
  {
    if(cursize_==curalloc_)
    {
      enlarge();
    }
    data_[cursize_]=x;
    cursize_++;
  }

  template<class InputIterator>
  void insert(iterator pos,InputIterator first,InputIterator last)
  { 
    size_type count = 0;
    for (InputIterator iter = first; iter != last; iter++)
      count++;

    if (curalloc_ < cursize_+count)
      enlarge(cursize_+count);
      
    // adjust (start at end and go to the beginning to not overwrite self)
    for (iterator iter = end()-1; iter != pos-1; iter--) 
    {
      *(iter + count) = *iter;
    }
    // insert
    for (;first != last; first++, pos++)
    {
      *pos = *first;
    }
    cursize_ += count;
  }
    


  void enlarge(size_type new_size = 0) 
  {
    if (new_size == 0)
    {
      curalloc_=(curalloc_<<1);
    }
    else 
    {
      if (curalloc_ < new_size) curalloc_ = new_size;
    }
    
    if (curalloc_ > CAPACITY)
    {
      T* newdata = new T[curalloc_];
      for(size_type i=0;i<cursize_;i++)
      {
        newdata[i]=data_[i];
      }
      if(data_ != &(fixed_[0]))
        delete[] data_;
      data_=newdata;
    }
    else
    {
      data_ = &(fixed_[0]);
    }
  }

  inline size_type size() const 
  { 
    return cursize_; 
  }
  
  inline void resize(size_type newsize) 
  { 
    cursize_ = newsize; 
    if (newsize > curalloc_) enlarge(newsize);
  }
  
  inline reference operator[](size_type n) 
  { 
    return data_[n]; 
  }
  
  inline const_reference operator[](size_type n) const 
  {
    return data_[n]; 
  }
  
  StackBasedVector(const StackBasedVector<T,CAPACITY>& copy)
  { 
    initialize();
    resize(copy.cursize_);
    for (size_type j=0; j<cursize_; j++) data_[j] = copy.data_[j];
  }
  
  StackBasedVector<T,CAPACITY>& operator=(const StackBasedVector<T,CAPACITY>& copy)
  { 
    clear(); 
    resize(copy.cursize_);
    for (size_type j=0; j<cursize_; j++) data_[j] = copy.data_[j];
    return *this; 
  }

private:
  
  T*        data_;
  T         fixed_[CAPACITY];
  size_type cursize_;
  size_type curalloc_;
};


} // end namespace

#endif
