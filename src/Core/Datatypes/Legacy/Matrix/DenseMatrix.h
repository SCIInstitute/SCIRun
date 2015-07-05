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
///@file  DenseMatrix.h
///@brief Dense matrices
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  October 1994
/// 

#ifndef CORE_DATATYPES_DENSEMATRIX_H
#define CORE_DATATYPES_DENSEMATRIX_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Util/FancyAssert.h>

#include <Core/Exceptions/FileNotFound.h>
#include <Core/Exceptions/LapackError.h>

#if defined(HAVE_LAPACK)
#include <Core/Math/sci_lapack.h>
#endif

#include <cfloat>
#include <vector>

#include <Core/Datatypes/share.h>

namespace SCIRun {

  class Transform;
  class Vector;
  class Point;
  class Tensor;

template <typename T>
class DenseMatrixGeneric : public Matrix<T> {

public:
  /// Constructors
  DenseMatrixGeneric();
  DenseMatrixGeneric(size_type r, size_type c);
  DenseMatrixGeneric(size_type r, size_type c, T value);
  DenseMatrixGeneric(const DenseMatrixGeneric&);
  explicit DenseMatrixGeneric(T scalar);
  explicit DenseMatrixGeneric(const Transform &t);
  explicit DenseMatrixGeneric(const Vector& vec);
  explicit DenseMatrixGeneric(const Point& pnt);
  explicit DenseMatrixGeneric(const Tensor& tens);

  /// Destructor
  virtual ~DenseMatrixGeneric();
  
  /// Public member functions
  virtual DenseMatrixGeneric* clone() const;
  DenseMatrixGeneric& operator=(const DenseMatrixGeneric&);
  
  virtual DenseMatrix *dense();
  virtual SparseRowMatrix *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrix *dense_col_maj();

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
                                       index_type &stride,
                                       index_type *&cols, T *&vals);

  virtual DenseMatrixGeneric* make_transpose() const;
  virtual void    mult(const ColumnMatrix& x, ColumnMatrix& b,
                      index_type beg=-1, index_type end=-1, 
                      int spVec=0) const;
  virtual void    mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                 index_type beg=-1, index_type end=-1, 
                                 int spVec=0) const;
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);

	void multiply(ColumnMatrix& x, ColumnMatrix& b) const;

  T  sumOfCol(index_type);
  T  sumOfRow(index_type);
  
 

  /// fast accessors
  inline T* operator[](index_type r) 
  {
    return data[r];
  }
  inline T const* operator[](index_type r) const
  {
    return data[r];
  }
  
  inline T** get_raw_2D_pointer() const
  {
    return data;
  }

  inline T& operator()(index_type i, index_type j) { return data[i][j]; }
  inline const T& operator()(index_type i, index_type j) const { return data[i][j]; }

  /// return false if not invertible.
  virtual bool invert();

  void transpose_square_in_place();

  /// throws an assertion if not square
  T determinant();

  static DenseMatrixGeneric *identity(size_type size);
  static DenseMatrixGeneric *zero_matrix(size_type rows, size_type cols);
  static DenseMatrix *make_diagonal_from_column(const ColumnMatrix& column, size_type rows, size_type cols);

  virtual void print(std::string&) const;
  
  /// Persistent representation...
  virtual std::string dynamic_type_name() const;
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

  /// Friend functions
  SCISHARE friend void Mult(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);

private:
  void init();
  T** data;
  T*  dataptr_;
};


namespace LinearAlgebra
{
  SCISHARE void svd(const DenseMatrix&, DenseMatrix&, SparseRowMatrixHandle&, DenseMatrix&);
  SCISHARE void svd(const DenseMatrix&, DenseMatrix&, ColumnMatrix&, DenseMatrix&);
  SCISHARE void svd(const DenseMatrix&, DenseMatrix&, DenseMatrix&, DenseMatrix&);

  SCISHARE void eigenvalues(const DenseMatrix&, ColumnMatrix&, ColumnMatrix&);
  SCISHARE void eigenvectors(const DenseMatrix&, ColumnMatrix&, ColumnMatrix&, DenseMatrix&);

  SCISHARE void solve_lapack(const DenseMatrix& matrix, const ColumnMatrix& rhs, ColumnMatrix& lhs);

  // NOTE: returns 1 if successful, or 0 if unsuccessful (i.e. ignore the solution vector)
  SCISHARE int solve(const DenseMatrix&, ColumnMatrix&);
  SCISHARE int solve(const DenseMatrix&, const ColumnMatrix& rhs, ColumnMatrix& lhs);
  SCISHARE int solve(const DenseMatrix&, std::vector<double>& sol);
  SCISHARE int solve(const DenseMatrix&, const std::vector<double>& rhs, std::vector<double>& lhs);
}


SCISHARE void Sub(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Add(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Add(DenseMatrix&, double, const DenseMatrix&, double, const DenseMatrix&);
SCISHARE void Add(double, DenseMatrix&, const DenseMatrix&);
SCISHARE void Mult_trans_X(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Mult_X_trans(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Concat_rows(DenseMatrix&, const DenseMatrix&, const DenseMatrix&); // Added by Saeed Babaeizadeh, Jan. 2006
SCISHARE void Concat_cols(DenseMatrix&, const DenseMatrix&, const DenseMatrix&); // Added by Saeed Babaeizadeh, Jan. 2006

namespace
{
  Persistent* DenseMatrixGenericMaker()
  {
    return new DenseMatrix;
  }
}

template <typename T>
PersistentTypeID DenseMatrixGeneric<T>::type_id("DenseMatrix", "Matrix", DenseMatrixGenericMaker);

template <typename T>
int
DenseMatrixGeneric<T>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(data,this->nrows_);
  sum += SCIRun::compute_checksum(dataptr_,this->nrows_*this->ncols_);
  return (sum);
}

template <typename T>
DenseMatrixGeneric<T>*
DenseMatrixGeneric<T>::clone() const
{
  return new DenseMatrixGeneric(*this);
}


/// constructors
template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric() :
data(0),
dataptr_(0)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(size_type r, size_type c) :
Matrix<T>(r, c)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  
  init();
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(T scalar) :
Matrix<T>(1, 1)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  init();
  dataptr_[0] = scalar;
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const Vector& vec) :
Matrix<double>(1, 3)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  init();
  dataptr_[0] = vec.x();
  dataptr_[1] = vec.y();
  dataptr_[2] = vec.z();
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const Point& pnt) :
Matrix<T>(1, 3)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  init();
  dataptr_[0] = pnt.x();
  dataptr_[1] = pnt.y();
  dataptr_[2] = pnt.z();
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const Tensor& tens) :
Matrix<T>(1, 6)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  init();
  dataptr_[0] = tens.xx();
  dataptr_[1] = tens.xy();
  dataptr_[2] = tens.xz();
  dataptr_[3] = tens.yy();
  dataptr_[4] = tens.yz();
  dataptr_[5] = tens.zz();
}

template <typename T>
void DenseMatrixGeneric<T>::init()
{
  data = new T*[this->nrows_];
  T* tmp = new T[this->nrows_ * this->ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i < this->nrows_; i++)
  {
    data[i] = tmp;
    tmp += this->ncols_;
  }
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const DenseMatrixGeneric& m) :
Matrix<T>(m.nrows_, m.ncols_)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  data = new T*[this->nrows_];
  T* tmp = new T[this->nrows_ * this->ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i<static_cast<index_type>(this->nrows_); i++)
  {
    data[i] = tmp;
    T* p = m.data[i];
    for (int j=0; j<this->ncols_; j++)
    {
      *tmp++ = *p++;
    }
  }
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(size_type r, size_type c, T value) :
Matrix<T>(r, c)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  
  init();
  std::fill(this->begin(), this->end(), value);
}

template <typename T>
DenseMatrixGeneric<T>::DenseMatrixGeneric(const Transform& t) :
Matrix<T>(4, 4)
{
  DEBUG_CONSTRUCTOR("DenseMatrix")  

  T dummy[16];
  t.get(dummy);
  data = new T*[this->nrows_];
  T* tmp = new T[this->nrows_ * this->ncols_];
  dataptr_ = tmp;
  T* p=dummy;

  for (index_type i=0; i<this->nrows_; i++)
  {
    data[i] = tmp;
    for (index_type j=0; j<this->ncols_; j++)
    {
      *tmp++ = static_cast<T>(*p++);
    }
  }
}

template <typename T>
T *
DenseMatrixGeneric<T>::get_data_pointer() const
{
  return dataptr_;
}

template <typename T>
size_type
DenseMatrixGeneric<T>::get_data_size() const
{
  return this->nrows() * this->ncols();
}

template <typename T>
DenseMatrixGeneric<T>::~DenseMatrixGeneric()
{
  DEBUG_DESTRUCTOR("DenseMatrix")  

  delete[] dataptr_;
  delete[] data;
}


/// assignment operator
template <typename T>
DenseMatrixGeneric<T>&
DenseMatrixGeneric<T>::operator=(const DenseMatrixGeneric& m)
{
  delete[] dataptr_;
  delete[] data;

  this->nrows_ = m.nrows_;
  this->ncols_ = m.ncols_;
  data = new T*[this->nrows_];
  T* tmp = new T[this->nrows_ * this->ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i<this->nrows_; i++)
  {
    data[i]=tmp;
    T* p=m.data[i];
    for (index_type j=0; j<this->ncols_; j++)
    {
      *tmp++=*p++;
    }
  }
  return *this;
}

template <typename T>
T
DenseMatrixGeneric<T>::get(index_type r, index_type c) const
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  return data[r][c];
}

template <typename T>
void
DenseMatrixGeneric<T>::put(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  data[r][c]=d;
}

template <typename T>
void
DenseMatrixGeneric<T>::add(index_type r, index_type c, T d)
{
  ASSERTRANGE(r, 0, this->nrows_);
  ASSERTRANGE(c, 0, this->ncols_);
  data[r][c] += d;
}

template <typename T>
T
DenseMatrixGeneric<T>::min()
{
  T min = DBL_MAX;
  for (index_type k=0; k<this->nrows_*this->ncols_; k++)
    if (dataptr_[k] < min) min = dataptr_[k];
  return (min);
}

template <typename T>
T
DenseMatrixGeneric<T>::max()
{
  T max = -DBL_MAX;
  for (index_type k=0; k<this->nrows_*this->ncols_; k++)
    if (dataptr_[k] > max) max = dataptr_[k];
  return (max);
}

template <typename T>
DenseMatrixGeneric<T>*
DenseMatrixGeneric<T>::make_transpose() const
{
  DenseMatrixGeneric *m=new DenseMatrixGeneric(this->ncols_, this->nrows_);
  T *mptr = &((*m)[0][0]);
  for (index_type c=0; c<this->ncols_; c++)
    for (index_type r=0; r<this->nrows_; r++)
      *mptr++ = data[r][c];
  return m;
}

template <typename T>
void DenseMatrixGeneric<T>::transpose_square_in_place()
{
  ASSERT(this->nrows_ == this->ncols_);
  for(index_type i=0; i<this->nrows_; i++)
  {
    for(index_type j=0; j<this->ncols_; j++)
    {
      if (i > j)
      {
        std::swap(data[i][j], data[j][i]);
      }
    }
  }
}

template <typename T>
void
DenseMatrixGeneric<T>::getRowNonzerosNoCopy(index_type r, size_type &size, index_type &stride,
                                            index_type *&cols, T *&vals)
{
  size = this->ncols_;
  stride = 1;
  cols = 0;
  vals = data[r];
}

template <typename T>
void
DenseMatrixGeneric<T>::zero()
{
  std::fill(this->begin(), this->end(), 0);
}

template <typename T>
std::string DenseMatrixGeneric<T>::dynamic_type_name() const { return type_id.type; }

template <typename T>
DenseMatrixGeneric<T>*
DenseMatrixGeneric<T>::identity(size_type size)
{
  DenseMatrixGeneric *result = zero_matrix(size, size);

  double* d = result->get_data_pointer();
  size_type nrows = result->nrows();
  for (index_type i = 0; i < size; i++)
  {
    (*d) = 1.0;
    d += (nrows+1);
  }

  return result;
}

template <typename T>
DenseMatrixGeneric<T>*
DenseMatrixGeneric<T>::zero_matrix(size_type rows, size_type cols)
{
  DenseMatrixGeneric *result = new DenseMatrixGeneric(rows, cols);
  result->zero();

  return result;
}

template <typename T>
void
DenseMatrixGeneric<T>::print(std::string& str) const
{
  std::ostringstream oss;
  for (index_type i=0; i<this->nrows_; i++)
  {
    for (index_type j=0; j<this->ncols_; j++)
    {
      oss << data[i][j] << " ";
    }
    oss << std::endl;
  }
  str.assign(oss.str());
}

template <typename T>
MatrixHandle
DenseMatrixGeneric<T>::submatrix(index_type r1, index_type c1, index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, this->nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, this->ncols_);

  DenseMatrixGeneric *mat = new DenseMatrixGeneric(r2 - r1 + 1, c2 - c1 + 1);
  for (index_type i=r1; i <= r2; i++)
  {
    memcpy(mat->data[i-r1], data[i] + c1, (c2 - c1 + 1) * sizeof(double));
  }
  return mat;
}


#define DENSEMATRIX_VERSION 4

template <typename T>
void
DenseMatrixGeneric<T>::io(Piostream& stream)
{

  int version=stream.begin_class("DenseMatrix", DENSEMATRIX_VERSION);
  // Do the base class first...
  Matrix<double>::io(stream);

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

  if(stream.reading())
  {
    data=new double*[this->nrows_];
    double* tmp=new double[this->nrows_ * this->ncols_];
    dataptr_=tmp;
    for (index_type i=0; i<this->nrows_; i++)
    {
      data[i] = tmp;
      tmp += this->ncols_;
    }
  }
  stream.begin_cheap_delim();

  int split;
  if (stream.reading())
  {
    if (version > 2)
    {
      split = this->separate_raw_;
      Pio(stream, split);
      if (this->separate_raw_)
      {
        Pio(stream, this->raw_filename_);
        FILE *f=fopen(this->raw_filename_.c_str(), "r");
        if (f)
        {
          fread(data[0], sizeof(T), this->nrows_ * this->ncols_, f);
          fclose(f);
        }
        else
        {
          const std::string errmsg = "Error reading separated file '" +
            this->raw_filename_ + "'";
          std::cerr << errmsg << "\n";
          SCI_THROW(FileNotFound(errmsg, __FILE__, __LINE__));
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
      fwrite(data[0], sizeof(T), this->nrows_ * this->ncols_, f);
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


/// @todo: this needs to be refactored
template <typename T>
bool
DenseMatrixGeneric<T>::invert()
{
  if (this->nrows_ != this->ncols_) return false;

#if defined(HAVE_LAPACK)
  // transpose matrix order: Fortran vs C ordering

  transpose_square_in_place();
  
  try
  {
    lapackinvert(dataptr_, this->nrows_);
  }
  catch (const SCIRun::LapackError& exception)
  {
    std::ostringstream oss;
    oss << "Caught LapackError exception: " << exception.message();
    // in the absence of a logging service
    std::cerr << oss.str() << std::endl;
    return false;
  }

  // transpose back to C ordering
  transpose_square_in_place();

  return true;
#else

  T** newdata=new T*[this->nrows_];
  T* tmp=new T[this->nrows_ * this->ncols_];
  T* newdataptr_=tmp;

  index_type i;
  for (i=0; i<this->nrows_; i++)
  {
    newdata[i]=tmp;
    for (index_type j=0; j<this->nrows_; j++)
    {
      tmp[j]=0;
    }
    tmp[i] = 1;
    tmp += this->ncols_;
  }

  // Gauss-Jordan with partial pivoting
  for (i=0;i<this->nrows_;i++)
  {
    T max=Abs(data[i][i]);
    index_type row=i;
    index_type j;
    for (j=i+1; j<this->nrows_; j++)
    {
      if(Abs(data[j][i]) > max)
      {
        max=Abs(data[j][i]);
        row=j;
      }
    }
    if(row != i)
    {
      // Switch row pointers for original matrix ...
      T* tmp=data[i];
      data[i]=data[row];
      data[row]=tmp;
      // ... but switch actual data values for inverse
      for (j=0; j<this->ncols_; j++) 
      {
        T ntmp=newdata[i][j];
        newdata[i][j]=newdata[row][j];
        newdata[row][j]=ntmp;
      }
    }
    T denom=1./data[i][i];
    T* r1=data[i];
    T* n1=newdata[i];
    for (j=i+1; j<this->nrows_; j++)
    {
      T factor=data[j][i]*denom;
      T* r2=data[j];
      T* n2=newdata[j];
      for (index_type k=0;k<this->nrows_;k++)
      {
        r2[k]-=factor*r1[k];
        n2[k]-=factor*n1[k];
      }
    }
  }

  // Back-substitution
  for (i=1; i<this->nrows_; i++)
  {
    T denom=1./data[i][i];
    T* r1=data[i];
    T* n1=newdata[i];
    for (index_type j=0;j<i;j++)
    {
      T factor=data[j][i]*denom;
      T* r2=data[j];
      T* n2=newdata[j];
      for (index_type k=0; k<this->nrows_; k++)
      {
        r2[k]-=factor*r1[k];
        n2[k]-=factor*n1[k];
      }
    }
  }

  // Normalize
  for (i=0;i<this->nrows_;i++)
  {
    T factor=1./data[i][i];
    for (index_type j=0;j<this->nrows_; j++)
    {
      data[i][j]*=factor;
      newdata[i][j]*=factor;
    }
  }

  delete[] dataptr_;
  delete[] data;
  dataptr_=newdataptr_;
  data=newdata;

  // Reorder data back in the right order
  for (index_type i=0;i<this->nrows_;i++)
  {
    T* old_ptr = data[i];
    T* new_ptr = dataptr_ + i*this->ncols_;

    T temp;
    // Swap rows
    for (index_type j=0; j<this->ncols_;j++)
    {
      temp = new_ptr[j];
      new_ptr[j] = old_ptr[j];
      old_ptr[j] = temp;
    }

    data[i] = new_ptr;
  }

  return true;
#endif
}

template <typename T>
T
DenseMatrixGeneric<T>::sumOfCol(size_type n)
{
  ASSERT(n<this->ncols_);
  ASSERT(n>=0);

  T sum = 0;

  for (index_type i=0; i<this->nrows_; i++)
  {
    sum+=data[i][n];
  }

  return sum;
}

template <typename T>
T
DenseMatrixGeneric<T>::sumOfRow(size_type n)
{
  ASSERT(n < this->nrows_);
  ASSERT(n >= 0);
  T* rp = data[n];
  T sum = 0;
  index_type i=0;
  while (i<this->ncols_) sum+=rp[i++];
  return sum;
}

template <typename T>
T
DenseMatrixGeneric<T>::determinant()
{
  T a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
  ASSERTMSG(((this->nrows_ == 4) && (this->ncols_ == 4)),
    "Number of Rows and Colums for Determinant must equal 4! (Code not completed)");
  a=data[0][0]; b=data[0][1]; c=data[0][2]; d=data[0][3];
  e=data[1][0]; f=data[1][1]; g=data[1][2]; h=data[1][3];
  i=data[2][0]; j=data[2][1]; k=data[2][2]; l=data[2][3];
  m=data[3][0]; n=data[3][1]; o=data[3][2]; p=data[3][3];

  T q=a*f*k*p - a*f*l*o - a*j*g*p + a*j*h*o + a*n*g*l - a*n*h*k
    - e*b*k*p + e*b*l*o + e*j*c*p - e*j*d*o - e*n*c*l + e*n*d*k
    + i*b*g*p - i*b*h*o - i*f*c*p + i*f*d*o + i*n*c*h - i*n*d*g
    - m*b*g*l + m*b*h*k + m*f*c*l - m*f*d*k - m*j*c*h + m*j*d*g;

  return q;
}


} // End namespace SCIRun

#endif
