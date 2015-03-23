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
///@file  SparseRowMatrix.h
///@brief Sparse Row Matrices
/// 
///@author
///    Steven G. Parker
///    Department of Computer Science
///    University of Utah
///@date  November 1994
///
///@details See http://math.nist.gov/MatrixMarket/formats.html#MMformat
/// for more information about this sparse matrix format.
/// 


#ifndef CORE_DATATYPES_SPARSEROWMATRIX_H
#define CORE_DATATYPES_SPARSEROWMATRIX_H 1

#include <Core/Datatypes/Matrix.h>

#include <Core/Math/MiscMath.h>
#include <Core/Util/Assert.h>
#include <Core/Util/FancyAssert.h>
#include <Core/Util/MemoryUtil.h>

#include <boost/shared_array.hpp>

#include <algorithm>

#include <float.h>
#include <stdio.h>
#include <memory.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

template <typename T>
class SparseRowMatrixGeneric : public Matrix<T> 
{
public:
  typedef boost::shared_array<index_type> Rows;
  typedef boost::shared_array<index_type> Columns;
  typedef boost::shared_array<T> Storage;
private:
  SparseRowMatrixGeneric(); // This is only used by the maker function.

  Rows rows_;
  Columns columns_;
  size_type nnz_;
  Storage data_;
 
  void clear_memory();

public:
  bool validate();
  void order_columns();

  class Data
  {
  public:
    Data(size_type rowSize, size_type columnSize, size_type dataSize) :
      rows_(new index_type[rowSize]),
      columns_(new index_type[columnSize]),
      data_(new T[dataSize])
    { }
    Data(size_type rowSize, size_type columnAndDataSize) :
      rows_(new index_type[rowSize]),
      columns_(new index_type[columnAndDataSize]),
      data_(new T[columnAndDataSize])
    { }
    Data(const std::vector<index_type>& rows, const std::vector<index_type>& columns, const std::vector<T>& data) :
      rows_(make_deep_copy(rows)),
      columns_(make_deep_copy(columns)),
      data_(make_deep_copy(data))
    { }
    //shallow copy for Builder class
    Data(const Rows& rows, const Columns& columns, const Storage& data) :
          rows_(rows), columns_(columns), data_(data)
    { }

    bool allocated() const { return rows_ && columns_ && data_; }

    const Rows& rows() const { return rows_; }
    const Columns& columns() const { return columns_; }
    const Storage& data() const { return data_; }

  private:
    const Rows rows_;
    const Columns columns_;
    const Storage data_;
  };

  class Builder : boost::noncopyable
  {
  public:
    Builder() {}
    const Rows& allocate_rows(size_type rowSize)
    {
      rows_.reset(new index_type[rowSize]);
      return rows_;
    }
    const Columns& allocate_columns(size_type colSize)
    {
      columns_.reset(new index_type[colSize]);
      return columns_;
    }
    const Storage& allocate_data(size_type dataSize)
    {
      data_.reset(new T[dataSize]);
      return data_;
    }
    Data build()
    {
      return Data(rows_, columns_, data_);
    }
  private:
    Rows rows_;
    Columns columns_;
    Storage data_;
  };

  /// Constructors
  // Here's what the arguments for the constructor should be:
  //   r   = number of rows
  //   c   = number of columns
  //
  //   rr  = row accumulation buffer containing r+1 entries where
  //         rr[N+1]-rr[N] is the number of non-zero entries in row N
  //         The last entry contains the total number of non-zero entries.
  //
  //   cc  = column number for each nonzero data entry.
  //         Sorted by row/col order and corresponds with the spaces in the
  //         rr array.
  //
  //   nnz = number of non zero entries.
  //
  //   d   = non zero data values.
  //
  //   sort_columns = do we need to sort columns of the matrix
  //
  //SparseRowMatrixGeneric(size_type r, size_type c, index_type *rr, index_type *cc, 
  //                size_type nnz, T *data, bool sort_columns = false);

  SparseRowMatrixGeneric(size_type r, size_type c, const Data& data, 
    size_type nnz, bool sort_columns = false);

  SparseRowMatrixGeneric(const SparseRowMatrixGeneric&);

  virtual SparseRowMatrixGeneric* clone() const;
  SparseRowMatrixGeneric& operator=(const SparseRowMatrixGeneric&);

  /// Destructor
  virtual ~SparseRowMatrixGeneric();

  virtual DenseMatrix *dense();
  virtual SparseRowMatrixGeneric *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrix *dense_col_maj();

  virtual T*   get_data_pointer() const;
  virtual size_type get_data_size() const;

  index_type getIdx(index_type, index_type);
  size_type get_nnz() const { return nnz_; }
  
  T* get_vals() { return data_.get(); }
  const T* get_vals() const { return data_.get(); }
  index_type* get_rows() { return rows_.get(); }
  index_type* get_cols() { return columns_.get(); }
  const index_type* get_rows() const { return rows_.get(); }
  const index_type* get_cols() const { return columns_.get(); }
  index_type get_row(index_type i) const { return rows_[i]; }
  index_type get_col(index_type i) const { return columns_[i]; }
  T get_value(index_type i) const { return data_[i]; }
  bool is_valid() const { return data_ && rows_ && columns_; }
  virtual bool is_zero() const
  {
    return 0 == nnz_;
  }

  inline T& operator()(index_type r, index_type c)
  {
    throw "not implemented";
  }

  inline const T& operator()(index_type r, index_type c) const
  {
    throw "not implemented";
  }

  virtual void zero();
  virtual T get(index_type, index_type) const;
  virtual void put(index_type row, index_type col, T val);
  virtual void add(index_type row, index_type col, T val);
  
  virtual T min();
  virtual T max();
  virtual int compute_checksum();
    
  virtual void getRowNonzerosNoCopy(index_type r, size_type &size, 
                                    size_type &stride,
                                    index_type *&cols, T *&vals);

  virtual SparseRowMatrixGeneric *make_transpose() const;
  virtual void mult(const ColumnMatrix& x, ColumnMatrix& b,
                    index_type beg=-1, index_type end=-1,
                    int spVec=0) const;
  virtual void mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                              index_type beg=-1, index_type end=-1, 
                              int spVec=0) const;
  virtual void scalar_multiply(T s);                              
                              
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);


  static SparseRowMatrixGeneric *identity(size_type size);

  virtual void print(std::string&) const;
 
  /// Persistent representation...
  virtual std::string dynamic_type_name() const { return type_id.type; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

  static Persistent *SparseRowMatrixGenericMaker();
};

template <typename T>
Persistent*
SparseRowMatrixGeneric<T>::SparseRowMatrixGenericMaker()
{
  return new SparseRowMatrixGeneric<T>;
}

template <typename T>
PersistentTypeID SparseRowMatrixGeneric<T>::type_id("SparseRowMatrix", "Matrix",
                                                    SparseRowMatrixGeneric<T>::SparseRowMatrixGenericMaker);

template <typename T>
int
SparseRowMatrixGeneric<T>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(rows_.get(), this->nrows_+1);
  sum += SCIRun::compute_checksum(columns_.get(), nnz_);
  sum += SCIRun::compute_checksum(data_.get(), nnz_);
  return (sum);
}


template <typename T>
SparseRowMatrixGeneric<T>*
SparseRowMatrixGeneric<T>::clone() const
{
  return new SparseRowMatrixGeneric(*this);
}

template <typename T>
SparseRowMatrixGeneric<T>::SparseRowMatrixGeneric() :
rows_(0),
columns_(0),
nnz_(0),
data_(0)
{
  DEBUG_CONSTRUCTOR("SparseRowMatrix")  
}

template <typename T>
void SparseRowMatrixGeneric<T>::clear_memory()
{
  data_.reset();
  rows_.reset();
  columns_.reset();
}

class matrix_sort_type : public std::binary_function<index_type,index_type,bool>
{
public:
  explicit matrix_sort_type(const index_type* cols) :
      cols_(cols)
      {}

      bool operator()(index_type i1, index_type i2) const
      {
        return(cols_[i1] < cols_[i2]);
      }

private:
  const index_type*      cols_;
};

template <typename T>
void
SparseRowMatrixGeneric<T>::order_columns()
{

  /// This code should reorder the columns of a sparse matrix so they are
  /// in ascending order. This code will purge duplicates as well by merging
  /// them together.

  /// Calculate the size of the buffer we need to reorder the data
  /// This way we can use the stl sorting algorithm  
  size_type max_num_cols = 0;
  for (index_type j = 0; j< this->nrows_; j++)
    if (rows_[j+1]-rows_[j] > max_num_cols)
      max_num_cols = rows_[j+1]- rows_[j];

  std::vector<index_type> order(max_num_cols);
  std::vector<T>     databuffer(max_num_cols);
  std::vector<index_type> indexbuffer(max_num_cols);

  matrix_sort_type sortmatrix(columns_.get());

  index_type rr = rows_[0];

  /// Sorting columns and removing duplicates
  for (index_type j = 0; j<this->nrows_; j++)
  {
    size_type num_cols = rows_[j+1]-rr;
    order.resize(num_cols);
    for (index_type p=rr; p<rows_[j+1]; p++) order[p-rr] = p;
    std::sort(order.begin(),order.end(),sortmatrix);

    for (index_type q=0; q<num_cols; q++)
    {
      databuffer[q] = data_[order[q]];
      indexbuffer[q] = columns_[order[q]];
    }

    index_type p = rows_[j];
    index_type q = 0;
    if (q < num_cols)
    {
      data_[p] = databuffer[q];
      columns_[p] = indexbuffer[q];
      q++;
    }

    while(q < num_cols)
    {
      if (columns_[p] == indexbuffer[q])
      {
        data_[p] += databuffer[q];
        q++;
      }
      else
      {
        p++;
        columns_[p] = indexbuffer[q];
        data_[p] = databuffer[q];
        q++;
      }
    }
    p++;

    rr = rows_[j+1];
    rows_[j+1] = p;
  }

  nnz_ = rows_[this->nrows_];
}

template <typename T>
SparseRowMatrixGeneric<T>::SparseRowMatrixGeneric(size_type nnrows, size_type nncols,
                                                  const Data& data,
                                                  size_type nnz, 
                                                  bool sort_columns) :
Matrix<T>(nnrows, nncols),
rows_(data.rows()),
columns_(data.columns()),
nnz_(nnz),
data_(data.data())
{
  DEBUG_CONSTRUCTOR("SparseRowMatrix")  
  if (data_ == 0) 
  { 
    data_.reset(new T[nnz_]);
  }
  if (sort_columns) order_columns();
}

template <typename T>
SparseRowMatrixGeneric<T>& SparseRowMatrixGeneric<T>::operator=(const SparseRowMatrixGeneric& copy) 
{
  clear_memory();

  this->nrows_ = copy.nrows();
  this->ncols_ = copy.ncols();
  nnz_ = copy.nnz_;

  rows_ = make_deep_copy(copy.rows_, this->nrows_+1);
  columns_ = make_deep_copy(copy.columns_, nnz_);
  data_ = make_deep_copy(copy.data_, nnz_);

  return *this;
}

template <typename T>
SparseRowMatrixGeneric<T>::SparseRowMatrixGeneric(const SparseRowMatrixGeneric& copy) :
Matrix<T>(copy.nrows_, copy.ncols_),
nnz_(copy.nnz_)
{
  DEBUG_CONSTRUCTOR("SparseRowMatrix")  

  rows_ = make_deep_copy(copy.rows_, this->nrows_+1);
  columns_ = make_deep_copy(copy.columns_, nnz_);
  data_ = make_deep_copy(copy.data_, nnz_);
}

template <typename T>
SparseRowMatrixGeneric<T>::~SparseRowMatrixGeneric()
{
  DEBUG_DESTRUCTOR("SparseRowMatrix")  
}

template <typename T>
bool
SparseRowMatrixGeneric<T>::validate()
{
  index_type i, j;

  ASSERTMSG(rows_[0] == 0, "Row start is nonzero.");
  for (i = 0; i< this->nrows_; i++)
  {
    ASSERTMSG(rows_[i] <= rows_[i+1], "Malformed rows, not increasing.");
    for (j = rows_[i]; j < rows_[i+1]; j++)
    {
      ASSERTMSG(columns_[j] >= 0 && columns_[j] < this->ncols_, "Column out of range.");
      if (j != rows_[i])
      {
        ASSERTMSG(columns_[j-1] != columns_[j], "Column doubled.");
        ASSERTMSG(columns_[j-1] < columns_[j], "Column out of order.");
      }
    }
  }
  ASSERTMSG(rows_[this->nrows_] == nnz_, "Row end is incorrect.");
  return true;
}

template <typename T>
T *
SparseRowMatrixGeneric<T>::get_data_pointer() const
{
  return data_.get();
}

template <typename T>
size_type
SparseRowMatrixGeneric<T>::get_data_size() const
{
  return nnz_;
}

template <typename T>
SparseRowMatrixGeneric<T>*
SparseRowMatrixGeneric<T>::make_transpose() const
{
  Data transposeData(this->ncols_+1, nnz_);
  size_type t_nnz = nnz_;
  size_type t_nncols = this->nrows_;
  size_type t_nnrows = this->ncols_;
  const SparseRowMatrix::Rows& t_rows = transposeData.rows();
  const SparseRowMatrix::Columns& t_columns = transposeData.columns();
  const SparseRowMatrix::Storage& t_a = transposeData.data();
  std::vector<index_type> at(t_nnrows+1);
  for (index_type i=0; i<t_nnz;i++)
  {
    at[columns_[i]+1]++;
  }
  t_rows[0] = 0;
  for (index_type i=1; i<t_nnrows+1; i++)
  {
    at[i] += at[i-1];
    t_rows[i] = at[i];
  }
  index_type c = 0;
  for (index_type r=0; r<this->nrows_; r++)
  {
    for (; c<rows_[r+1]; c++)
    {
      index_type mcol = columns_[c];
      t_columns[at[mcol]] = r;
      t_a[at[mcol]] = data_[c];
      at[mcol]++;
    }
  }
  return new SparseRowMatrixGeneric(t_nnrows, t_nncols, transposeData, t_nnz);
}

template <typename T>
index_type
SparseRowMatrixGeneric<T>::getIdx(index_type i, index_type j)
{
  index_type row_idx=rows_[i];
  index_type next_idx=rows_[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      return -1;
    }
    index_type m=(l+h)/2;
    if (j<columns_[m])
    {
      h=m-1;
    }
    else if (j>columns_[m])
    {
      l=m+1;
    }
    else
    {
      return m;
    }
  }
}

template <typename T>
T
SparseRowMatrixGeneric<T>::get(index_type i, index_type j) const
{
  index_type row_idx=rows_[i];
  index_type next_idx=rows_[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      return 0.0;
    }
    index_type m=(l+h)/2;
    if (j<columns_[m])
    {
      h=m-1;
    }
    else if (j>columns_[m])
    {
      l=m+1;
    }
    else
    {
      return data_[m];
    }
  }
}

template <typename T>
void
SparseRowMatrixGeneric<T>::put(index_type i, index_type j, T d)
{
  index_type row_idx=rows_[i];
  index_type next_idx=rows_[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      ASSERTFAIL("SparseRowMatrix::put into invalid(dataless) location.");
      return;
    }
    index_type m=(l+h)/2;
    if (j<columns_[m])
    {
      h=m-1;
    }
    else if (j>columns_[m])
    {
      l=m+1;
    }
    else
    {
      data_[m] = d;
      return;
    }
  }
}

template <typename T>
void
SparseRowMatrixGeneric<T>::add(index_type i, index_type j, T d)
{
  index_type row_idx=rows_[i];
  index_type next_idx=rows_[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      ASSERTFAIL("SparseRowMatrix::add into invalid(dataless) location.");
      return;
    }
    index_type m=(l+h)/2;
    if (j<columns_[m])
    {
      h=m-1;
    }
    else if (j>columns_[m])
    {
      l=m+1;
    }
    else
    {
      data_[m] += d;
      return;
    }
  }
}

template <typename T>
T
SparseRowMatrixGeneric<T>::min()
{
  T min = DBL_MAX;
  for (index_type k=0; k<nnz_; k++)
    if (data_[k] < min) min = data_[k];
  return (min);
}

template <typename T>
T
SparseRowMatrixGeneric<T>::max()
{
  T max = -DBL_MAX;
  for (index_type k=0; k<nnz_; k++)
    if (data_[k] > max) max = data_[k];
  return (max);
}

template <typename T>
void
SparseRowMatrixGeneric<T>::getRowNonzerosNoCopy(index_type r, size_type &size, 
                                                index_type &stride,
                                                index_type *&cols, T *&vals)
{
  size = rows_[r+1] - rows_[r];
  stride = 1;
  cols = columns_.get() + rows_[r];
  vals = data_.get() + rows_[r];
}

template <typename T>
void
SparseRowMatrixGeneric<T>::zero()
{
  std::fill(data_.get(), data_.get() + nnz_, 0);
}

template <typename T>
void SparseRowMatrixGeneric<T>::print(std::string &str) const
{
  str.clear();
  std::ostringstream oss;
  oss.flags(std::ios::showpoint);
  index_type index = 0;
  oss << "nrows=" << this->nrows_ << " ncols=" << this->ncols_ << " nnz=" << nnz_ << std::endl;
  for (index_type r = 0; r < this->nrows_; r++) 
  {
    while(index < rows_[r+1]) 
    {
      oss << r << " " << columns_[index] << " " << data_[index] << std::endl;
      ++index;
    }
  }
  str = oss.str();
}


#define SPARSEROWMATRIX_VERSION 2

template <typename T>
void
SparseRowMatrixGeneric<T>::io(Piostream& stream)
{
  int version = stream.begin_class("SparseRowMatrix", SPARSEROWMATRIX_VERSION);
  // Do the base class first...
  Matrix<T>::io(stream);

  if (version <2)
  {
    int r = static_cast<int>(this->nrows_);
    int c = static_cast<int>(this->ncols_);
    int n = static_cast<int>(nnz_);
    stream.io(r);
    stream.io(c);
    stream.io(n);
    this->nrows_ = static_cast<size_type>(r);
    this->ncols_ = static_cast<size_type>(c);
    nnz_= static_cast<size_type>(n);
  }
  else
  {
    Pio_size(stream,this->nrows_);
    Pio_size(stream,this->ncols_);
    Pio_size(stream,nnz_);
  }

  if (stream.reading())
  {
    data_.reset(new T[nnz_]);
    columns_.reset(new index_type[nnz_]);
    rows_.reset(new index_type[this->nrows_+1]);
  }

  stream.begin_cheap_delim();  
  Pio_index(stream, rows_.get(), this->nrows_+1);
  stream.end_cheap_delim();

  stream.begin_cheap_delim();
  Pio_index(stream, columns_.get(), nnz_);
  stream.end_cheap_delim();

  stream.begin_cheap_delim();
  Pio(stream, data_.get(), nnz_);
  stream.end_cheap_delim();

  stream.end_class();
}

template <typename T>
MatrixHandle
SparseRowMatrixGeneric<T>::submatrix(index_type r1, index_type c1, 
                                     index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, this->nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, this->ncols_);

  index_type i, j;
  std::vector<index_type> rs(r2-r1+2);
  std::vector<index_type> csv;
  std::vector<T> valsv;

  rs[0] = 0;
  for (i = r1; i <= r2; i++)
  {
    rs[i-r1+1] = rs[i-r1];
    for (j = rows_[i]; j < rows_[i+1]; j++)
    {
      if (columns_[j] >= c1 && columns_[j] <= c2)
      {
        csv.push_back(columns_[j] - c1);
        valsv.push_back(data_[j]);
        rs[i-r1+1]++;
      }
    }
  }

  Data data(rs, csv, valsv);

  return new SparseRowMatrixGeneric(r2-r1+1, c2-c1+1, data,
    static_cast<size_type>(valsv.size()));
}

template <typename T>
SparseRowMatrixGeneric<T>*
SparseRowMatrixGeneric<T>::identity(size_type size)
{ 
  Data data(size+1, size);
  const Rows& r = data.rows();
  const Columns& c = data.columns();
  const Storage& d = data.data();

  index_type i;
  for (i=0; i<size; i++)
  {
    c[i] = r[i] = i;
    d[i] = 1.0;
  }
  r[i] = i;

  return new SparseRowMatrixGeneric(size, size, data, size);
}
  
/// @todo: replace with for_each
template <typename T>
void 
SparseRowMatrixGeneric<T>::scalar_multiply(T s)
{
  for (index_type i=0; i < this->nnz_; ++i)
  {
    data_[i] *= s;
  }
}
  

} // End namespace SCIRun

#endif
