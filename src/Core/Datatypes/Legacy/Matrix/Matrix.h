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
///@file  Matrix.h
///@brief Matrix definitions
/// 
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  July 1994
/// 

#ifndef CORE_DATATYPES_MATRIX_H
#define CORE_DATATYPES_MATRIX_H 1

#include <Core/Util/CheckSum.h>
#include <Core/Util/Debug.h>
#include <Core/Util/StringUtil.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Geometry/Transform.h>
#include <Core/Datatypes/Types.h>
#include <Core/Datatypes/PropertyManager.h>
#include <Core/Datatypes/MatrixFwd.h>

#include <string.h>
#include <iosfwd>

#include <Core/Datatypes/share.h>

namespace SCIRun {

template <typename T>
class SCISHARE MatrixInterface
{
public:
  virtual ~MatrixInterface() {}

  //Too slow to be virtual.  Consider implementing on each subclass though, to enable generic algorithms.
  //virtual T& operator()(index_type i, index_type j) = 0;
  //virtual const T& operator()(index_type i, index_type j) const = 0;
  virtual size_type nrows() const = 0;
  virtual size_type ncols() const = 0;
  virtual void zero() = 0;
  /// @todo
  //virtual MatrixInterface<T>* make_transpose() const = 0;
};

class SCISHARE MatrixBase : public PropertyManager
{
public:
  MatrixBase() : separate_raw_(false), raw_filename_("") {}
  // Separate raw files.
  void set_raw(bool v) { separate_raw_ = v; }
  bool get_raw() const { return separate_raw_; }
  void set_raw_filename( const std::string &f )
  { 
    raw_filename_ = f; 
    separate_raw_ = true; 
  }
  const std::string get_raw_filename() const { return raw_filename_; }

  virtual void print(std::string&) const {}

  // Persistent representation.
  virtual std::string dynamic_type_name() const { return "MatrixBase"; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

protected:
  bool           separate_raw_;
  std::string    raw_filename_;
};

template <typename T>
class Matrix : public MatrixBase, public MatrixInterface<T>
{
protected:
  Matrix(size_type nrows = 0, size_type ncols = 0) : MatrixBase(),
    nrows_(nrows), ncols_(ncols)
  {
    DEBUG_CONSTRUCTOR("Matrix")    
  }
  size_type nrows_;
  size_type ncols_;

public:
  virtual ~Matrix()
  {
    DEBUG_DESTRUCTOR("Matrix")
  }
  
  typedef T element_type;

  /// Make a duplicate, needed to support detach from LockingHandle
  virtual Matrix* clone() const = 0;

  /// Convert this matrix to the specified type.
  virtual DenseMatrix* dense() = 0;
  virtual SparseRowMatrixGeneric<T>* sparse() = 0;
  virtual ColumnMatrixGeneric<T>* column() = 0;
  virtual DenseColMajMatrixGeneric<T> *dense_col_maj() = 0;

  inline size_type nrows() const { return nrows_; }
  inline size_type ncols() const { return ncols_; }
  /// return false if not invertible.
  virtual inline bool invert() { return false; }

  virtual T   *get_data_pointer() const = 0;
  virtual size_type get_data_size() const = 0;

  inline T* begin() const { return get_data_pointer(); }
  inline T* end() const  { return get_data_pointer() + get_data_size(); }

  virtual void    zero() = 0;
  virtual T  get(index_type r, index_type c) const = 0;
  virtual void    put(index_type r, index_type c, T val) = 0;
  virtual void    add(index_type r, index_type c, T val) = 0;
  
  virtual T  min() = 0;
  virtual T  max() = 0;
  virtual int compute_checksum() = 0;
  size_type number_of_nonzero_elements() const
  {
    return std::count_if(this->begin(), this->end(), std::bind1st(std::not_equal_to<T>(), 0));
  }
  virtual bool is_zero() const
  {
    return std::find_if(this->begin(), this->end(), std::bind1st(std::not_equal_to<T>(), 0)) == this->end();
  }

  // getRowNonzerosNocopy returns:
  //   vals = The values.  They are not guaranteed 
  //     to be nonzero, but some of the zeros may be missing.
  //   cols = The columns associated with the vals.  This may be NULL, in
  //     which case the cols are 0-(size-1) (a full row).
  //   size = The number of entries in vals.
  //   stride = The matrix may not be in row order, so this is how far
  //     to walk in vals.  For example vals (and cols) should be
  //     accessed as vals[3*stride] to get the fourth value.  As of this
  //     time all the matrices return 1 for this value.
  //   For example usage see Dataflow/Modules/Fields/ApplyMappingMatrix.h
  virtual void getRowNonzerosNoCopy(index_type r, index_type &size, 
                                    index_type &stride,
                                    index_type *&cols, T *&vals) = 0;

  virtual Matrix<T>* make_transpose() const = 0;
  virtual void mult(const ColumnMatrix& x, ColumnMatrix& b,
		    index_type beg=-1, index_type end=-1, int spVec=0) const=0;
  virtual void mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
			      index_type beg=-1, index_type end=-1, int spVec=0) const=0;
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2) = 0;

  virtual std::string dynamic_type_name() const { return type_id.type; }

  virtual void scalar_multiply(T s)
  {
    for (T* p = begin(); p != end(); ++p)
    {
      *p *= s;
    }
  }

  static PersistentTypeID type_id;
};

template<typename T>
PersistentTypeID Matrix<T>::type_id("Matrix", "MatrixBase", 0);

#ifdef _WIN32
template<>
PersistentTypeID Matrix<double>::type_id("Matrix", "MatrixBase", 0);
#endif

SCISHARE void Mult(ColumnMatrix&, const Matrix<double>&, const ColumnMatrix&);

inline std::string matrix_to_string(const MatrixBase& mat)
{
  std::string str;
  mat.print(str);
  return (str);
}

inline std::string to_string(const MatrixHandle& mat)
{
  return matrix_to_string(*mat);
}

} // End namespace SCIRun

#endif
