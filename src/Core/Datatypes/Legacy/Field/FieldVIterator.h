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


#ifndef CORE_DATATYPES_FIELDVITERATOR_H
#define CORE_DATATYPES_FIELDVITERATOR_H 1

#include <Core/Datatypes/Legacy/Field/FieldVIndex.h>

namespace SCIRun {


/// Base type for FieldVIterator types.
template <class T>
class FieldVIteratorBase
{
  public:
  FieldVIteratorBase(T i) :
    index_(i) {}

  /// Field Iterators need to be able to increment.
  inline
  T operator ++() { return ++index_; }
  T operator --() { return --index_; }

  bool operator ==(const FieldVIteratorBase &a) const
  { return this->index_ == a.index_; }
  bool operator !=(const FieldVIteratorBase &a) const
  { return this->index_ != a.index_; }

  inline T operator*() const { return this->index_; }

  typedef std::bidirectional_iterator_tag iterator_category;
  typedef T difference_type;

protected:
  T index_;
private:
  /// Hide this in private to prevent it from being called.
  FieldVIteratorBase<T> operator ++(int);
  FieldVIteratorBase<T> operator --(int);
};


/// Distinct type for node FieldIterator.
template <class T>
struct VNodeIterator : public FieldVIteratorBase<T> {
  VNodeIterator() :
    FieldVIteratorBase<T>(0) {}
  VNodeIterator(T iter) :
    FieldVIteratorBase<T>(iter) {}

  /// Required interface for an FieldIterator.
  inline
  VNodeIndex<T> operator*() const { return VNodeIndex<T>(this->index_); }

  typedef VNodeIndex<T> value_type;
  typedef VNodeIndex<T>* pointer;
  typedef VNodeIndex<T>& reference;

  template <class U> friend U operator+(const VNodeIndex<U> &a, const VNodeIndex<U> &b);
};

/// Distinct type for lagrangian node FieldIterator.
template <class T>
struct VENodeIterator : public FieldVIteratorBase<T> {
  VENodeIterator() :
    FieldVIteratorBase<T>(0) {}
  VENodeIterator(T iter) :
    FieldVIteratorBase<T>(iter) {}

  /// Required interface for an FieldIterator.
  inline
  VENodeIndex<T> operator*() const { return VENodeIndex<T>(this->index_); }
};

/// Distinct type for edge Iterator.
template <class T>
struct VEdgeIterator : public FieldVIteratorBase<T> {
  VEdgeIterator() :
    FieldVIteratorBase<T>(0) {}
  VEdgeIterator(T index) :
    FieldVIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  VEdgeIndex<T> operator*() const { return VEdgeIndex<T>(this->index_); }

  typedef VEdgeIndex<T> value_type;
  typedef VEdgeIndex<T>* pointer;
  typedef VEdgeIndex<T>& reference;
};

/// Distinct type for face Iterator.
template <class T>
struct VFaceIterator : public FieldVIteratorBase<T> {
  VFaceIterator() :
    FieldVIteratorBase<T>(0) {}
  VFaceIterator(T index) :
    FieldVIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  VFaceIndex<T> operator*() const { return VFaceIndex<T>(this->index_); }

  template <class U> friend U operator+(const VFaceIndex<U> &a, const VFaceIndex<U> &b);
};

/// Distinct type for cell Iterator.
template <class T>
struct VCellIterator : public FieldVIteratorBase<T> {
  VCellIterator() :
    FieldVIteratorBase<T>(0) {}
  VCellIterator(T index) :
    FieldVIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  VCellIndex<T> operator*() const { return VCellIndex<T>(this->index_); }
};

/// Distinct type for cell Iterator.
template <class T>
struct VElemIterator : public FieldVIteratorBase<T> {
  VElemIterator() :
    FieldVIteratorBase<T>(0) {}
  VElemIterator(T index) :
    FieldVIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  VElemIndex<T> operator*() const { return VElemIndex<T>(this->index_); }
};

/// Distinct type for cell Iterator.
template <class T>
struct VDElemIterator : public FieldVIteratorBase<T> {
  VDElemIterator() :
    FieldVIteratorBase<T>(0) {}
  VDElemIterator(T index) :
    FieldVIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  VDElemIndex<T> operator*() const { return VDElemIndex<T>(this->index_); }
};

template <class T>
T operator+(const VNodeIndex<T> &a, const VNodeIndex<T> &b)
{ return a.index_ + b.index_; }

template <class T>
T operator+(const VFaceIndex<T> &a, const VFaceIndex<T> &b)
{ return a.index_ + b.index_; }

}

#endif
