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
///@file  DenseColMajMatrix.h
///@brief DenseColMaj matrices
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  October 1994
/// 

#ifndef CORE_DATATYPES_DENSECOLMAJMATRIX_H
#define CORE_DATATYPES_DENSECOLMAJMATRIX_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Geometry/Transform.h>
#include <Core/Math/MiscMath.h>

#include <vector>

#include <stdio.h>

#include <sci_defs/lapack_defs.h>
#include <sci_defs/blas_defs.h>

#include <Core/Util/Assert.h>
#include <Core/Util/FancyAssert.h>

#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Math/MiscMath.h>
#include <Core/Exceptions/FileNotFound.h>

#include <iostream>
#include <sstream>
#include <float.h>
#include <string.h>


#include <Core/Datatypes/share.h>

namespace SCIRun {

template <typename T>
class DenseColMajMatrixGeneric : public Matrix<T>
{

public:
  /// Constructors
  DenseColMajMatrixGeneric();
  DenseColMajMatrixGeneric(size_type r, size_type c);
  DenseColMajMatrixGeneric(const DenseColMajMatrixGeneric&);

  /// Destructor
  virtual ~DenseColMajMatrixGeneric();
  
  /// Public member functions
  virtual DenseColMajMatrixGeneric* clone() const;
  DenseColMajMatrixGeneric& operator=(const DenseColMajMatrixGeneric&);
  
  virtual DenseMatrix *dense();
  virtual SparseRowMatrix *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrixGeneric *dense_col_maj();

  virtual T*   get_data_pointer() const;
  virtual size_type get_data_size() const;

  /// slow setters/getter for polymorphic operations
  virtual void    zero();
  virtual T  get(index_type r, index_type c) const;
  virtual void    put(index_type r, index_type c, T val);
  virtual void    add(index_type r, index_type c, T val);
  
  virtual T min();
  virtual T max();
  virtual int compute_checksum();

  virtual void    getRowNonzerosNoCopy(index_type r, size_type &size, 
                                       size_type &stride,
                                       index_type *&cols, T *&vals);

  virtual DenseColMajMatrixGeneric* make_transpose() const;
  virtual void    mult(const ColumnMatrix& x, ColumnMatrix& b,
                       index_type beg=-1, index_type end=-1, 
                       int spVec=0) const;
  virtual void    mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                 index_type beg=-1, index_type end=-1, 
                                 int spVec=0) const;
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);


  T  sumOfCol(size_type);
  T  sumOfRow(size_type);
  

  /// fast accessors
  inline T &iget(index_type r, index_type c)
  {
    return dataptr_[c * this->nrows_ + r];
  }

  /// fast accessors
  inline const T &iget(index_type r, index_type c) const
  {
    return dataptr_[c * this->nrows_ + r];
  }

  inline T& operator()(index_type r, index_type c)
  {
    return iget(r, c);
  }

  inline const T& operator()(index_type r, index_type c) const
  {
    return iget(r, c);
  }

  /// Throws an assertion if not square
  T determinant();

  static DenseColMajMatrixGeneric* identity(size_type size);

  virtual void print(std::string&) const;
  
  /// Persistent representation...
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  
private:      
  T*  dataptr_;
};

namespace
{
  Persistent* DenseColMajMatrixMaker()
  {
    return new DenseColMajMatrix;
  }
}

template <typename T>
PersistentTypeID DenseColMajMatrixGeneric<T>::type_id("DenseColMajMatrix", "Matrix", DenseColMajMatrixMaker);

template <typename T>
DenseColMajMatrixGeneric<T>*
DenseColMajMatrixGeneric<T>::clone() const
{
  return new DenseColMajMatrixGeneric(*this);
}

template <typename T>
int
DenseColMajMatrixGeneric<T>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(dataptr_, this->nrows_ * this->ncols_);
  return (sum);
}

/// constructors
template <typename T>
DenseColMajMatrixGeneric<T>::DenseColMajMatrixGeneric() :
dataptr_(0)
{
  DEBUG_CONSTRUCTOR("DenseColMajMatrix")  
}

template <typename T>
DenseColMajMatrixGeneric<T>::DenseColMajMatrixGeneric(size_type r, size_type c) :
Matrix<T>(r, c)
{
  DEBUG_CONSTRUCTOR("DenseColMajMatrix")  

  dataptr_ = new double[this->nrows_ * this->ncols_];
}

template <typename T>
DenseColMajMatrixGeneric<T>::DenseColMajMatrixGeneric(const DenseColMajMatrixGeneric& m) :
Matrix<T>(m.nrows_, m.ncols_)
{
  DEBUG_CONSTRUCTOR("DenseColMajMatrix")  

  dataptr_ = new double[this->nrows_ * this->ncols_];
  memcpy(dataptr_, m.dataptr_, sizeof(double) * this->nrows_ * this->ncols_);
}

template <typename T>
T*
DenseColMajMatrixGeneric<T>::get_data_pointer() const
{
  return dataptr_;
}

template <typename T>
size_type
DenseColMajMatrixGeneric<T>::get_data_size() const
{
  return this->nrows() * this->ncols();
}


/// destructor
template <typename T>
DenseColMajMatrixGeneric<T>::~DenseColMajMatrixGeneric()
{
  DEBUG_DESTRUCTOR("DenseColMajMatrix")  

    delete[] dataptr_;
}


/// assignment operator
template <typename T>
DenseColMajMatrixGeneric<T>&
DenseColMajMatrixGeneric<T>::operator=(const DenseColMajMatrixGeneric& m)
{
  delete[] dataptr_;

  this->nrows_ = m.nrows_;
  this->ncols_ = m.ncols_;
  dataptr_ = new double[this->nrows_ * this->ncols_];
  memcpy(dataptr_, m.dataptr_, sizeof(double) * this->nrows_ * this->ncols_);

  return *this;
}

template <typename T>
T
DenseColMajMatrixGeneric<T>::get(index_type r, index_type c) const
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  return iget(r, c);
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::put(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  iget(r, c) = d;
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::add(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  iget(r, c) += d;
}

template <typename T>
T
DenseColMajMatrixGeneric<T>::min()
{
  T min = DBL_MAX;
  for (index_type k=0; k<this->nrows_*this->ncols_; k++)
    if (dataptr_[k] < min) min = dataptr_[k];
  return (min);
}

template <typename T>
T
DenseColMajMatrixGeneric<T>::max()
{
  T max = -DBL_MAX;
  for (index_type k=0; k<this->nrows_*this->ncols_; k++)
    if (dataptr_[k] > max) max = dataptr_[k];
  return (max);
}


template <typename T>
DenseColMajMatrixGeneric<T>*
DenseColMajMatrixGeneric<T>::make_transpose() const
{
  DenseColMajMatrixGeneric<T>* m = new DenseColMajMatrixGeneric(this->ncols_, this->nrows_);
  for (index_type c=0; c<this->ncols_; c++)
    for (index_type r=0; r<this->nrows_; r++)
      m->iget(c, r) = iget(r, c);
  return m;
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::getRowNonzerosNoCopy(index_type r, size_type &size,
                                           index_type &stride,
                                           index_type *&cols, T *&vals)
{
  size = this->ncols_;
  stride = this->nrows_;
  cols = 0;
  vals = dataptr_ + r;
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::zero()
{
  memset(dataptr_, 0, sizeof(double) * this->nrows_ * this->ncols_);
}

template <typename T>
DenseColMajMatrixGeneric<T>*
DenseColMajMatrixGeneric<T>::identity(size_type size)
{
  DenseColMajMatrixGeneric<T>* result = new DenseColMajMatrixGeneric(size, size);
  result->zero();
  for (index_type i = 0; i < size; i++)
  {
    result->iget(i, i) = 1.0;
  }

  return result;
}

template <typename T>
void
DenseColMajMatrixGeneric<T>::print(std::string& str) const
{
  std::ostringstream oss;
  for (index_type i=0; i<this->nrows_; i++)
  {
    for (index_type j=0; j<this->ncols_; j++)
    {
      oss << iget(i, j) << " ";
    }
    oss << std::endl;
  }
  str.assign(oss.str());
}

template <typename T>
MatrixHandle
DenseColMajMatrixGeneric<T>::submatrix(index_type r1, index_type c1, 
                                index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, this->nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, this->ncols_);
  DenseColMajMatrixGeneric *mat = new DenseColMajMatrixGeneric(r2 - r1 + 1, c2 - c1 + 1);
  for (index_type i = c1; i <= c2; i++)
  {
    /// @todo: Test this.
    memcpy(mat->dataptr_ + (i - c1) * (r2 - r1 + 1),
      dataptr_ + c1 * this->nrows_ + r1,
      (r2 - r1 + 1) * sizeof(double));
  }
  return mat;
}

#define DENSECOLMAJMATRIX_VERSION 3

template <typename T>
void
DenseColMajMatrixGeneric<T>::io(Piostream& stream)
{
  int version=stream.begin_class("DenseColMajMatrix", DENSECOLMAJMATRIX_VERSION);

  // Do the base class first.
  Matrix<T>::io(stream);

  if (version < 4)
  {
    int nrows = static_cast<int>(this->nrows_);
    int ncols = static_cast<int>(this->ncols_);
    stream.io(nrows);
    stream.io(ncols);
    this->nrows_ = static_cast<size_type>(nrows);
    this->ncols_ = static_cast<size_type>(ncols);
  }
  else
  {
    long long nrows = static_cast<long long>(this->nrows_);
    long long ncols = static_cast<long long>(this->ncols_);
    stream.io(nrows);
    stream.io(ncols);
    this->nrows_ = static_cast<size_type>(nrows);
    this->ncols_ = static_cast<size_type>(ncols);  
  }

  if (stream.reading())
  {
    dataptr_ = new T[this->nrows_ * this->ncols_];
  }
  stream.begin_cheap_delim();

  int split;
  if (stream.reading())
  {
    if (version > 2)
    {
      Pio(stream, this->separate_raw_);
      if (this->separate_raw_)
      {
        Pio(stream, this->raw_filename_);
        FILE *f = fopen(this->raw_filename_.c_str(), "r");
        if (f)
        {
          fread(dataptr_, sizeof(T), this->nrows_ * this->ncols_, f);
          fclose(f);
        }
        else
        {
          const std::string errmsg = "Error reading separated file '" +
            this->raw_filename_ + "'";
          std::cerr << errmsg << "\n";
          throw FileNotFound(errmsg, __FILE__, __LINE__);
        }
      }
    }
    else
    {
      this->separate_raw_ = false;
    }
    split = this->separate_raw_;
  }
  else
  {     // writing
    std::string filename = this->raw_filename_;
    split = this->separate_raw_;
    if (split)
    {
      if (filename == "")
      {
        if (stream.file_name.c_str())
        {
          size_t pos = stream.file_name.rfind('.');
          if (pos == std::string::npos) filename = stream.file_name + ".raw";
          else filename = stream.file_name.substr(0,pos) + ".raw";
        } 
        else 
        {
          split=0;
        }
      }
    }
    Pio(stream, split);
    if (split)
    {
      Pio(stream, filename);
      FILE *f = fopen(filename.c_str(), "w");
      fwrite(dataptr_, sizeof(T), this->nrows_ * this->ncols_, f);
      fclose(f);
    }
  }

  if (!split)
  {
    size_t block_size = this->nrows_ * this->ncols_;
    if (!stream.block_io(dataptr_, sizeof(T), block_size))
    {
      for (size_t i = 0; i < block_size; i++)
      {
        stream.io(dataptr_[i]);
      }
    }
  }
  stream.end_cheap_delim();
  stream.end_class();
}


} // End namespace SCIRun

#endif
