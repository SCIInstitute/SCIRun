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
///@file  FieldIterator.h
///@brief Some convenient simple iterators for fields.
///
///@author
///       Marty Cole
///       Department of Computer Science
///       University of Utah
///@date  January 2001
///

#ifndef Datatypes_FieldIterator_h
#define Datatypes_FieldIterator_h

#include <Core/Datatypes/Legacy/Field/FieldIndex.h>

#include <iterator>

namespace SCIRun {


/// Base type for FieldIterator types.
template <class T>
struct FieldIteratorBase {
  FieldIteratorBase(T i) :
    index_(i) {}

  /// Field Iterators need to be able to increment.
  inline
  T operator ++() { return ++index_; }
  T operator --() { return --index_; }

  bool operator ==(const FieldIteratorBase &a) const
  { return index_ == a.index_; }
  bool operator !=(const FieldIteratorBase &a) const
  { return index_ != a.index_; }

  inline T operator*() { return index_; }

  typedef std::bidirectional_iterator_tag iterator_category;
  typedef T difference_type;

protected:
  T index_;
#ifdef __digital__
public:
#else
private:
#endif
  /// Hide this in private to prevent it from being called.
  FieldIteratorBase<T> operator ++(int) {
    FieldIteratorBase<T> tmp(*this); ++index_; return tmp; }
  FieldIteratorBase<T> operator --(int) {
    FieldIteratorBase<T> tmp(*this); --index_; return tmp; }
};

/// Distinct type for node FieldIterator.
template <class T>
struct NodeIterator : public FieldIteratorBase<T> {
  NodeIterator() :
    FieldIteratorBase<T>(0) {}
  NodeIterator(T iter) :
    FieldIteratorBase<T>(iter) {}

  /// Required interface for an FieldIterator.
  inline
  NodeIndex<T> operator*() { return NodeIndex<T>(this->index_); }
  typedef NodeIndex<T> value_type;
  typedef NodeIndex<T>* pointer;
  typedef NodeIndex<T>& reference;
};

/// Distinct type for edge Iterator.
template <class T>
struct EdgeIterator : public FieldIteratorBase<T> {
  EdgeIterator() :
    FieldIteratorBase<T>(0) {}
  EdgeIterator(T index) :
    FieldIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  EdgeIndex<T> operator*() { return EdgeIndex<T>(this->index_); }
  typedef EdgeIndex<T> value_type;
  typedef EdgeIndex<T>* pointer;
  typedef EdgeIndex<T>& reference;
};

/// Distinct type for face Iterator.
template <class T>
struct FaceIterator : public FieldIteratorBase<T> {
  FaceIterator() :
    FieldIteratorBase<T>(0) {}
  FaceIterator(T index) :
    FieldIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  FaceIndex<T> operator*() { return FaceIndex<T>(this->index_); }
};

/// Distinct type for cell Iterator.
template <class T>
struct CellIterator : public FieldIteratorBase<T> {
  CellIterator() :
    FieldIteratorBase<T>(0) {}
  CellIterator(T index) :
    FieldIteratorBase<T>(index) {}

  /// Required interface for an FieldIterator.
  inline
  CellIndex<T> operator*() { return CellIndex<T>(this->index_); }
};


}

#endif // Datatypes_FieldIterator_h
