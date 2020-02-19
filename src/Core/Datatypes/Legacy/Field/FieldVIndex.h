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


#ifndef CORE_DATATYPES_FIELDVINDEX_H
#define CORE_DATATYPES_FIELDVINDEX_H 1


#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>

namespace SCIRun {

/// Base type for index types.
template <class T>
class FieldVIndexBase {
public:
  typedef T value_type;

  FieldVIndexBase(T i) :
    index_(i) {}

  /// Required interface for an Index.
  operator T const &() const { return index_; }

  inline FieldVIndexBase<T> operator+(const int) const;
  inline FieldVIndexBase<T> operator+(const long long) const;
  inline FieldVIndexBase<T> operator+(const FieldVIndexBase<T>&) const;
  inline FieldVIndexBase<T> operator-(const int) const;
  inline FieldVIndexBase<T> operator-(const long long) const;
  inline FieldVIndexBase<T> operator-(const FieldVIndexBase<T>&) const;
  inline FieldVIndexBase<T>& operator++();
  inline void                operator++(int);
  inline FieldVIndexBase<T>& operator--();
  inline void                operator--(int);
  inline FieldVIndexBase<T>& operator+=(const int);
  inline FieldVIndexBase<T>& operator+=(const long long);
  inline FieldVIndexBase<T>& operator+=(const FieldVIndexBase<T>&);
  inline FieldVIndexBase<T>& operator-=(const int);
  inline FieldVIndexBase<T>& operator-=(const long long);
  inline FieldVIndexBase<T>& operator-=(const FieldVIndexBase<T>&);

  T index_;
};

template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator+(const int i) const
{
  return (FieldVIndexBase<T>(index_+i));
}

template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator+(const long long i) const
{
  return (FieldVIndexBase<T>(index_+i));
}


template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator+(const FieldVIndexBase<T>& i) const
{
  return (FieldVIndexBase<T>(index_+i.index_));
}

template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator-(const int i) const
{
  return (FieldVIndexBase<T>(index_-i));
}

template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator-(const long long i) const
{
  return (FieldVIndexBase<T>(index_-i));
}

template<class T>
inline FieldVIndexBase<T> FieldVIndexBase<T>::operator-(const FieldVIndexBase<T>& i) const
{
  return (FieldVIndexBase<T>(index_-i.index_));
}

template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator+=(const int i)
{
  index_ += static_cast<T>(i);
  return *this;
}

template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator+=(const long long i)
{
  index_ += static_cast<T>(i);
  return *this;
}


template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator++()
{
  index_++;
  return *this;
}

template<class T>
inline void FieldVIndexBase<T>::operator++(int)
{
  index_++;
}


template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator--()
{
  index_--;
  return *this;
}

template<class T>
inline void FieldVIndexBase<T>::operator--(int)
{
  index_--;
}

template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator+=(const FieldVIndexBase<T>& i)
{
  index_ += i.index_;
  return (*this);
}

template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator-=(const int i)
{
  index_ -= static_cast<T>(i);
  return (*this);
}

template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator-=(const long long i)
{
  index_ -= static_cast<T>(i);
  return (*this);
}


template<class T>
inline FieldVIndexBase<T>& FieldVIndexBase<T>::operator-=(const FieldVIndexBase<T>& i)
{
  index_ -= i.index_;
  return (*this);
}


/// Distinct type for node index.
template <class T>
struct VNodeIndex : public FieldVIndexBase<T> {
  VNodeIndex() :
    FieldVIndexBase<T>(0) {}
  VNodeIndex(T index) :
    FieldVIndexBase<T>(index) {}
};

/// Distinct type for additional lagrangian node.
template <class T>
struct VENodeIndex : public FieldVIndexBase<T> {
  VENodeIndex() :
    FieldVIndexBase<T>(0) {}
  VENodeIndex(T index) :
    FieldVIndexBase<T>(index) {}
};


/// Distinct type for edge index.
template <class T>
struct VEdgeIndex : public FieldVIndexBase<T> {
  VEdgeIndex() :
    FieldVIndexBase<T>(0) {}
  VEdgeIndex(T index) :
    FieldVIndexBase<T>(index) {}
};


/// Distinct type for face index.
template <class T>
struct VFaceIndex : public FieldVIndexBase<T> {
  VFaceIndex() :
    FieldVIndexBase<T>(0) {}
  VFaceIndex(T index) :
    FieldVIndexBase<T>(index) {}
};

/// Distinct type for cell index.
template <class T>
struct VCellIndex : public FieldVIndexBase<T> {
  VCellIndex() :
    FieldVIndexBase<T>(0) {}
  VCellIndex(T index) :
    FieldVIndexBase<T>(index) {}
};

/// Distinct type for elem index.
template <class T>
struct VElemIndex : public FieldVIndexBase<T> {
  VElemIndex() :
    FieldVIndexBase<T>(0) {}
  VElemIndex(T index) :
    FieldVIndexBase<T>(index) {}
};

/// Distinct type for elem index.
template <class T>
struct VDElemIndex : public FieldVIndexBase<T> {
  VDElemIndex() :
    FieldVIndexBase<T>(0) {}
  VDElemIndex(T index) :
    FieldVIndexBase<T>(index) {}
};


} // end namespace SCIRun

#endif
