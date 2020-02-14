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


#ifndef SPIRE_ENTITY_SYSTEM_COMPONENTGROUP_HPP
#define SPIRE_ENTITY_SYSTEM_COMPONENTGROUP_HPP

#include <es-log/trace-log.h>
#include <iterator>
#include "ComponentContainer.hpp"
#include <spire/scishare.h>

namespace spire {

template <typename T>
struct ComponentGroup
{
  class const_iterator;

  // See: http://stackoverflow.com/questions/16434681/stl-compatible-iterators-for-custom-containers
  class iterator: public std::iterator<std::random_access_iterator_tag, T>
  {
  public:
    iterator(): p_(nullptr) {}
    iterator(typename ComponentContainer<T>::ComponentItem* p): p_(p) {}
    iterator(const iterator& other): p_(other.p_) {}
    const iterator& operator=(const iterator& other) {p_ = other.p_; return other;}

    iterator& operator++()    {p_++; return *this;} // prefix++
    iterator  operator++(int) {iterator tmp(*this); ++(*this); return tmp;} // postfix++
    iterator& operator--()    {p_--; return *this;} // prefix--
    iterator  operator--(int) {iterator tmp(*this); --(*this); return tmp;} // postfix--

    void     operator+=(const std::size_t& n)  {p_ += n;}
    void     operator+=(const iterator& other) {p_ += other.p_;}
    iterator operator+ (const std::size_t& n)  {iterator tmp(*this); tmp += n; return tmp;}
    iterator operator+ (const iterator& other) {iterator tmp(*this); tmp += other; return tmp;}

    void        operator-=(const std::size_t& n)  {p_ -= n;}
    void        operator-=(const iterator& other) {p_ -= other.p_;}
    iterator    operator- (const std::size_t& n)  {iterator tmp(*this); tmp -= n; return tmp;}
    std::size_t operator- (const iterator& other) {return p_ - other.p_;}

    bool operator==(const iterator& other) {return  p_ == other.p_; }
    bool operator!=(const iterator& other) {return  p_ != other.p_; }

    T& operator[](const int& n) {return (p_+n)->component;}
    T& operator*() {return  p_->component;}
    T* operator->(){return &p_->component;}

    uint64_t getEntityID() const {return p_->sequence;}

  private:
    typename ComponentContainer<T>::ComponentItem* p_;

    friend class const_iterator;
  };

  class const_iterator: public std::iterator<std::random_access_iterator_tag, T>
  {
  public:
    const_iterator(): p_(nullptr) {}
    const_iterator(const typename ComponentContainer<T>::ComponentItem* p): p_(p) {}
    const_iterator(const iterator& other): p_(other.p_) {}
    const_iterator(const const_iterator& other): p_(other.p_) {}
    const const_iterator& operator=(const const_iterator& other) {p_ = other.p_; return other;}
    const const_iterator& operator=(const iterator& other) {p_ = other.p_; return other;}

    const_iterator& operator++()    {p_++; return *this;} // prefix++
    const_iterator  operator++(int) {const_iterator tmp(*this); ++(*this); return tmp;} // postfix++
    const_iterator& operator--()    {p_--; return *this;} // prefix--
    const_iterator  operator--(int) {const_iterator tmp(*this); --(*this); return tmp;} // postfix--

    void           operator+=(const std::size_t& n)              {p_ += n;}
    void           operator+=(const const_iterator& other)       {p_ += other.p_;}
    const_iterator operator+ (const std::size_t& n)        const {const_iterator tmp(*this); tmp += n; return tmp;}
    const_iterator operator+ (const const_iterator& other) const {const_iterator tmp(*this); tmp += other; return tmp;}

    void           operator-=(const std::size_t& n)              {p_ -= n;}
    void           operator-=(const const_iterator& other)       {p_ -= other.p_;}
    const_iterator operator- (const std::size_t& n)        const {const_iterator tmp(*this); tmp -= n; return tmp;}
    std::size_t    operator- (const const_iterator& other) const {return p_ - other.p_;}

    bool operator==(const const_iterator& other) const {return p_ == other.p_; }
    bool operator!=(const const_iterator& other) const {return p_ != other.p_; }

    const T& operator[](const int& n) const {return (p_+n)->component;}
    const T& operator*()  const {return p_->component;}
    const T* operator->() const {return  &p_->component;}

    uint64_t getEntityID() const {return p_->sequence;}

  private:
    const typename ComponentContainer<T>::ComponentItem* p_;
  };

  const T& operator[](std::size_t idx) const {return components[idx];}
  //T& operator[](std::size_t& idx) {return component[idx];}
  std::size_t size() const {return numComponents;}

  iterator begin() { return iterator(components); }
  iterator end()   { return iterator(components+numComponents); }
  const_iterator begin() const{ return const_iterator(components); }
  const_iterator end() const  { return const_iterator(components+numComponents);}

  const T& front() const
  {
    if (numComponents != 0)
    {
      return components->component;
    }
    else
    {
      std::cerr << "Attempt made to access front of empty ComponentGroup." << std::endl;
      throw std::runtime_error("Attempt made to access front of empty ComponentGroup.");
    }
  }
  const T& back() const
  {
    if (numComponents != 0)
    {
      return components[numComponents - 1].component;
    }
    else
    {
      std::cerr << "Attempt made to access back of empty ComponentGroup." << std::endl;
      throw std::runtime_error("Attempt made to access back of empty ComponentGroup.");
    }
  }

  // Should be able to override the equality operator to perform this function,
  // but I want modification to be explicit.
  void modify(const T& val, size_t componentNum = 0, int priority = 1) const
  {
    // Modify value by storing index, in raw array, to modified component.
    // This index will be used when re-integrating changes made at the end
    // of the frame.
    if (container != nullptr)
    {
      container->modifyIndex(val, containerIndex + componentNum, priority);
    }
    else
    {
      std::cerr << "Attempted to modify non-existante container!" << std::endl;
    }
  }

  //void modify(T&& val, size_t componentNum = 0, int priority = 1) const
  //{
  //  // Modify value by storing index, in raw array, to modified component.
  //  // This index will be used when re-integrating changes made at the end
  //  // of the frame.
  //  if (container != nullptr)
  //  {
  //    container->modifyIndex(std::move(val), containerIndex + componentNum, priority);
  //  }
  //  else
  //  {
  //    std::cerr << "Attempted to modify non-existante container!" << std::endl;
  //  }
  //}

  std::size_t numComponents;
  typename ComponentContainer<T>::ComponentItem* components;

  // The following two variables are only used when modify is called. Used
  // to modify the value of the component.
  size_t containerIndex;
  ComponentContainer<T>* container; // Can be a nullptr.
};

} // namespace spire

#endif
