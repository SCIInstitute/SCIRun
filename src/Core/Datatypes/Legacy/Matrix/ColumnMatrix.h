/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
///@file  ColumnMatrix.h
///@brief for RHS and LHS
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1994
/// 

#ifndef CORE_DATATYPES_COLUMNMATRIX_H
#define CORE_DATATYPES_COLUMNMATRIX_H 1

#include <Core/Util/FancyAssert.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <cfloat>

#include <Core/Datatypes/share.h>

namespace SCIRun {

template <typename T>
class ColumnMatrixGeneric : public Matrix<T> {

public:
  explicit ColumnMatrixGeneric(size_type rows = 0);
  ColumnMatrixGeneric(const ColumnMatrixGeneric&);
  /// @todo in C++11--move ctors for all matrix types.
  //ColumnMatrixGeneric(ColumnMatrixGeneric&& move);
  ColumnMatrixGeneric& operator=(const ColumnMatrixGeneric&);
  virtual ColumnMatrixGeneric* clone() const;
  virtual ~ColumnMatrixGeneric();

  virtual DenseMatrix* dense();
  virtual SparseRowMatrixGeneric<T>* sparse();
  virtual ColumnMatrixGeneric<T>* column();
  virtual DenseColMajMatrixGeneric<T>* dense_col_maj();

  virtual T *get_data_pointer() const;
  virtual size_type get_data_size() const;

  inline T& operator[](int r) const
  {
    ASSERTRANGE(r, 0, this->nrows_)
    return data_[r];
  }
  /// @todo: remove
  void set_data(T* d) {data_ = d;} 

  T  get(index_type r) const      
    { ASSERTRANGE(r, 0, this->nrows_); return data_[r]; };
  void    put(index_type r, T val) 
    { ASSERTRANGE(r, 0, this->nrows_); data_[r] = val; };

  // destroys data
  void resize(size_type);

  virtual void zero();
  virtual T get(index_type, index_type) const;
  virtual void put(index_type row, index_type col, T val);
  virtual void add(index_type row, index_type col, T val);
  
  virtual T& operator()(index_type i, index_type j);
  virtual const T& operator()(index_type i, index_type j) const;

  virtual T min();
  virtual T max();
  virtual int compute_checksum();
    
  virtual void getRowNonzerosNoCopy(index_type r, size_type &size, 
                                    index_type &stride,
                                    index_type *&cols, T *&vals);

  virtual Matrix<T>* make_transpose() const;
  virtual void mult(const ColumnMatrix& x, ColumnMatrix& b,
                    index_type beg=-1, index_type end=-1, 
                    int spVec=0) const;
  virtual void mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                              index_type beg=-1, index_type end=-1, 
                              int spVec=0) const;
  virtual void scalar_multiply(T s);
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);

  T sumOfCol(index_type);

  DenseMatrix exterior(const ColumnMatrixGeneric &) const;
  T vector_norm() const;
  T vector_norm(index_type beg, index_type end) const;

  T infinity_norm() const;

  virtual void print(std::string&) const;
  
  // Persistent representation...
  virtual std::string dynamic_type_name() const { return type_id.type; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
                          
private:
  T* data_;
};


static Persistent* ColumnMatrixMaker()
{
  return new ColumnMatrix(0);
}

template <typename T>
PersistentTypeID ColumnMatrixGeneric<T>::type_id("ColumnMatrix", "Matrix", ColumnMatrixMaker);

template <typename T>
int
ColumnMatrixGeneric<T>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(data_,this->nrows_);
  return (sum);
}

template <typename T>
ColumnMatrixGeneric<T>::ColumnMatrixGeneric(size_type rows) :
Matrix<double>(rows, 1)
{
  DEBUG_CONSTRUCTOR("ColumnMatrix")   

    if (this->nrows_)
      data_ = new double[this->nrows_];
    else
      data_ = 0;
}

template <typename T>
ColumnMatrixGeneric<T>::ColumnMatrixGeneric(const ColumnMatrixGeneric& c) :
Matrix<double>(c.nrows_, 1)
{
  DEBUG_CONSTRUCTOR("ColumnMatrix")   

    if (this->nrows_)
    {
      data_ = new double[this->nrows_];
      for (index_type i = 0; i < this->nrows_; i++)
        data_[i] = c.data_[i];
    }
    else 
    {
      data_ = 0;
    }
}

template <typename T>
T& ColumnMatrixGeneric<T>::operator()(size_type i, size_type j)
{
  ASSERT(j == 0);
  return data_[i];
}

template <typename T>
const T& ColumnMatrixGeneric<T>::operator()(size_type i, size_type j) const
{
  ASSERT(j == 0);
  return data_[i];
}

template <typename T>
T*
ColumnMatrixGeneric<T>::get_data_pointer() const
{
  return data_;
}

template <typename T>
size_type
ColumnMatrixGeneric<T>::get_data_size() const
{
  return this->nrows();
}

template <typename T>
Matrix<T>*
ColumnMatrixGeneric<T>::make_transpose() const
{
  DenseMatrix *dm = new DenseMatrix(1, this->nrows_);
  for (index_type i=0; i<this->nrows_; i++)
  {
    (*dm)[0][i] = data_[i];
  }
  return dm;
}

template <typename T>
ColumnMatrixGeneric<T>* ColumnMatrixGeneric<T>::clone() const
{
  return new ColumnMatrixGeneric(*this);
}

template <typename T>
ColumnMatrixGeneric<T>& ColumnMatrixGeneric<T>::operator=(const ColumnMatrixGeneric& c)
{
  if (this->nrows_ != c.nrows_)  {
    delete[] data_;
    this->nrows_ = c.nrows_;
    data_ = new T[this->nrows_];
  }
  for (index_type i = 0; i < this->nrows_; i++)
  {
    data_[i] = c.data_[i];
  }
  return *this;
}

template <typename T>
T
ColumnMatrixGeneric<T>::min()
{
  T min = DBL_MAX;
  for (index_type k = 0; k < this->nrows_; k++)
    if (data_[k] < min)
      min = data_[k];
  return (min);
}

template <typename T>
T
ColumnMatrixGeneric<T>::max()
{
  T max = -DBL_MAX;
  for (index_type k = 0; k < this->nrows_; k++)
    if (data_[k] > max)
      max = data_[k];
  return (max);
}

template <typename T>
ColumnMatrixGeneric<T>::~ColumnMatrixGeneric()
{
  DEBUG_DESTRUCTOR("ColumnMatrix")   
    delete[] data_;
}

template <typename T>
void ColumnMatrixGeneric<T>::resize(size_type new_rows)
{
  delete[] data_;
  if (new_rows) 
    data_ = new T[new_rows];
  else
    data_ = 0;
  this->nrows_ = new_rows;
}

template <typename T>
void ColumnMatrixGeneric<T>::zero()
{
  std::fill(data_, data_ + this->nrows_, 0.0);
}



template <typename T>
void 
ColumnMatrixGeneric<T>::print(std::string& str) const
{
  std::ostringstream oss;
  for(index_type i=0; i<this->nrows_; i++)
  {
    oss << data_[i] << "\n";
  }
  str.assign(oss.str());
}

template <typename T>
T 
ColumnMatrixGeneric<T>::get(index_type r, index_type c) const
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTEQ(c, 0);
  return data_[r];
}

template <typename T>
void 
ColumnMatrixGeneric<T>::put(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTEQ(c, 0);
  data_[r] = d;
}

template <typename T>
void 
ColumnMatrixGeneric<T>::add(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTEQ(c, 0);
  data_[r] += d;
}

template <typename T>
T 
ColumnMatrixGeneric<T>::sumOfCol(index_type c) 
{
  ASSERTEQ(c, 0);
  T sum = 0;
  for (index_type i=0; i< this->nrows_; i++)
  {
    sum+=data_[i];
  }
  return sum;
}

template <typename T>
void
ColumnMatrixGeneric<T>::getRowNonzerosNoCopy(index_type r, size_type &size, 
                                             index_type &stride,
                                             index_type *&cols, 
                                             T *&vals)
{
  size = 1;
  stride = 1;
  cols = 0;
  vals = data_ + r;
}

template <typename T>
void 
ColumnMatrixGeneric<T>::mult(const ColumnMatrix&, ColumnMatrix&,
                             index_type , index_type , int) const 
{
  ASSERTFAIL("Error - called mult on a columnmatrix.\n");
}

template <typename T>
void 
ColumnMatrixGeneric<T>::mult_transpose(const ColumnMatrix&, ColumnMatrix&,
                                       index_type, index_type, int) const 
{
  ASSERTFAIL("Error - called mult_transpose on a columnmatrix.\n");
}

#define COLUMNMATRIX_VERSION 3

template <typename T>
void ColumnMatrixGeneric<T>::io(Piostream& stream)
{
  int version=stream.begin_class("ColumnMatrix", COLUMNMATRIX_VERSION);

  if (version > 1)
  {
    // New version inherits from Matrix
    Matrix<T>::io(stream);
  }

  if (version < 3)
  {
    int nrows = static_cast<int>(this->nrows_);
    stream.io(nrows);
    this->nrows_ = static_cast<size_type>(nrows);
  }
  else
  {
    long long nrows= static_cast<long long>(this->nrows_);
    stream.io(nrows);
    this->nrows_ = static_cast<size_type>(nrows);  
  }

  if (stream.reading())
  {
    data_ = new T[this->nrows_];
  }

  if (!stream.block_io(data_, sizeof(T), this->nrows_))
  {
    for (index_type i=0; i<this->nrows_; i++)
      stream.io(data_[i]);
  }
  stream.end_class();
}


/// @todo: replace with for_each
template <typename T>
void 
ColumnMatrixGeneric<T>::scalar_multiply(T s)
{
  for (index_type i=0; i < this->nrows_; i++)
  {
    data_[i] *= s;
  }
}

template <typename T>
MatrixHandle
ColumnMatrixGeneric<T>::submatrix(index_type r1, index_type c1, 
                                  index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, this->nrows_);
  ASSERTEQ(c1, 0);
  ASSERTEQ(c2, 0);

  ColumnMatrixGeneric<T> *mat = new ColumnMatrixGeneric<T>(r2 - r1 + 1);
  memcpy(mat->data_, data_ + r1, (r2 - r1 + 1) * sizeof(T));

  return mat;
}

template <typename T>
DenseMatrix
ColumnMatrixGeneric<T>::exterior(const ColumnMatrixGeneric &m) const
{
  DenseMatrix ret(this->nrows_, this->nrows_);

  if (this->nrows_ != m.nrows())
  {
    ASSERTFAIL("Cannot compute exterior of two vectors of unequal dimensions.");
  }
  for (index_type i=0; i < this->nrows_; i++)
  {
    for (index_type j=0; j< this->nrows_; j++)
    {
      ret.put(i, j, get(j) * m.get(i));
    }
  }

  return ret;
}


} // End namespace SCIRun

#endif
