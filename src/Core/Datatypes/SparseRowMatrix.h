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


#ifndef CORE_DATATYPES_SPARSE_MATRIX_H
#define CORE_DATATYPES_SPARSE_MATRIX_H

#include <Core/Datatypes/Matrix.h>
#include <Core/Math/MiscMath.h>
#define register
#include <Eigen/SparseCore>
#undef register

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class SparseRowMatrixGeneric : public MatrixBase<T>, public Eigen::SparseMatrix<T, Eigen::RowMajor, index_type>
  {
  public:
    typedef T value_type;
    typedef SparseRowMatrixGeneric<T> this_type;
    typedef Eigen::SparseMatrix<T, Eigen::RowMajor, index_type> EigenBase;
    typedef Eigen::Triplet<T> Triplet;

    /// @todo: need C++11
    //using Base::Base;

    SparseRowMatrixGeneric() : EigenBase() {}
    SparseRowMatrixGeneric(int nrows, int ncols) : EigenBase(nrows, ncols) {}

    ///Legacy construction compatibility. Useful for converting old code, but should be avoided in new code.
    SparseRowMatrixGeneric(int nrows, int ncols, const index_type* rowCounter, const index_type* columnCounter, size_t nnz) : EigenBase(nrows, ncols)
    {
      if (rowCounter[nrows] != nnz)
        THROW_INVALID_ARGUMENT("Invalid sparse row matrix array: row accumulator array does not match number of non-zero elements.");
      std::vector<Triplet> triplets;
      triplets.reserve(nnz);

      int i = 0;
      int j = 0;
      while (i < nrows)
      {
        while (j < rowCounter[i + 1])
        {
          index_type column = columnCounter[j];
          if (column >= ncols)
            THROW_INVALID_ARGUMENT("Invalid sparse row matrix array: column index out of bounds.");
          triplets.push_back(Triplet(i, columnCounter[j], 0));
          j++;
        }
        i++;
      }
      this->setFromTriplets(triplets.begin(), triplets.end());
    }

    SparseRowMatrixGeneric(int nrows, int ncols, const index_type* rowCounter, const index_type* columnCounter, const T* data, size_t nnz) : EigenBase(nrows, ncols)
    {
      if (rowCounter[nrows] != nnz)
        THROW_INVALID_ARGUMENT("Invalid sparse row matrix array: row accumulator array does not match number of non-zero elements.");
      std::vector<Triplet> triplets;
      triplets.reserve(nnz);

      int i = 0;
      int j = 0;
      while (i < nrows)
      {
        while (j < rowCounter[i + 1])
        {
          index_type column = columnCounter[j];
          if (column >= ncols)
            THROW_INVALID_ARGUMENT("Invalid sparse row matrix array: column index out of bounds.");
          triplets.push_back(Triplet(i, columnCounter[j], data[j]));
          j++;
        }
        i++;
      }
      this->setFromTriplets(triplets.begin(), triplets.end());
      this->reserve(nnz);
      this->makeCompressed();
    }

    /// This constructor allows you to construct SparseRowMatrixGeneric from Eigen expressions
    template<typename OtherDerived>
    SparseRowMatrixGeneric(const Eigen::SparseMatrixBase<OtherDerived>& other)
      : EigenBase(other)
    { }

    /// This method allows you to assign Eigen expressions to SparseRowMatrixGeneric
    template<typename OtherDerived>
    SparseRowMatrixGeneric& operator=(const Eigen::SparseMatrixBase<OtherDerived>& other)
    {
      this->EigenBase::operator=(other);
      return *this;
    }

    SparseRowMatrixGeneric(const SparseRowMatrixGeneric& other) : EigenBase(other) {}
    SparseRowMatrixGeneric& operator=(const SparseRowMatrixGeneric& other)
    {
      if (this != &other)
      {
        this->EigenBase::operator=(other);
      }
      return *this;
    }
    SparseRowMatrixGeneric(SparseRowMatrixGeneric&& other) : EigenBase(std::move(other)) {}

    virtual SparseRowMatrixGeneric* clone() const override
    {
      return new SparseRowMatrixGeneric(*this);
    }

    virtual size_t nrows() const override { return this->rows(); }
    virtual size_t ncols() const override { return this->cols(); }

    typedef index_type RowsData;
    typedef index_type ColumnsData;
    typedef RowsData* RowsPtr;
    typedef ColumnsData* ColumnsPtr;
    typedef const RowsData* ConstRowsPtr;
    typedef const ColumnsData* ConstColumnsPtr;
    typedef T* Storage;

    ConstRowsPtr get_rows() const { return this->outerIndexPtr(); }
    ConstColumnsPtr get_cols() const { return this->innerIndexPtr(); }
    RowsPtr get_rows() { return this->outerIndexPtr(); }
    ColumnsPtr get_cols() { return this->innerIndexPtr(); }

    this_type getColumn(int i) const
    {
#ifdef _WIN32
      // TODO: Eigen compiler bug I think!
      auto tr = this->transpose().eval();
#else
      auto tr = this->transpose();
#endif
      return tr.block(i, 0, 1, nrows()).transpose();
    }

    virtual void accept(MatrixVisitorGeneric<T>& visitor) override
    {
      visitor.visit(*this);
    }

    bool isSymmetric() const
    {
      if (this->cols() != this->rows())
        return false;

      if (this->rows() * this->cols() > 1e7)
        THROW_INVALID_ARGUMENT("Dangerous call! This poorly implememented method will convert your sparse matrix to dense. It needs to be rewritten. To avoid memory wastage, throwing an exception here.");

      return this->isApprox(this->transpose(),1e-16);
    }

    virtual T get(int i, int j) const override
    {
      return this->coeff(i,j);
    }
    virtual void put(int i, int j, const T& val) override
    {
      this->coeffRef(i,j) = val;
      //TODO: not sure this is best place for this call: it's the slowest but also the safest since this is a virtual Matrix function. Users should know to avoid calling this with known sparse matrices.
      this->makeCompressed();
    }

    // legacy support
    void getRowNonzerosNoCopy(index_type row, size_type& size, index_type*& cols, T*& vals)
    {
      auto rows = this->get_rows();
      size = rows[row+1] - rows[row];
      cols = this->get_cols() + rows[row];
      vals = this->valuePtr() + rows[row];
    }

    /// @todo!
#if 0
    class NonZeroIterator : public std::iterator<std::forward_iterator_tag, value_type>
    {
    public:
      typedef NonZeroIterator my_type;
      typedef this_type matrix_type;
      typedef typename std::iterator<std::forward_iterator_tag, value_type> my_base;
      //typedef typename my_base::value_type value_type;
      typedef typename my_base::pointer pointer;

    private:
      const matrix_type* matrix_;
      typedef typename matrix_type::EigenBase::InnerIterator IteratorPrivate;
      boost::scoped_ptr<IteratorPrivate> impl_;
      //size_type block_i_, block_j_, rowInBlock_;

      //typedef typename ElementType::ImplType block_impl_type;
      //typedef typename boost::numeric::ublas::matrix_row<const block_impl_type> block_row_type;
      //typedef typename block_row_type::iterator block_row_iterator;

      //boost::shared_ptr<block_row_type> currentRow_;
      //block_row_iterator rowIter_;
      //size_type numRowPartitions_, numColPartitions_;
      int currentRow_;
      bool isEnd_;

    public:
      explicit NonZeroIterator(const matrix_type* matrix = 0) : matrix_(matrix),
        currentRow_(0)
        //block_i_(0), block_j_(0), rowInBlock_(0)
      {
        if (matrix && matrix->nonZeros() > 0)
        {
          impl_.reset(new IteratorPrivate(matrix,0));
          isEnd_ = !*impl_;
        }
        else
        {
          isEnd_ = true;
        }
      }
    //private:
    //  void resetRow()
    //  {
    //    currentRow_.reset(new block_row_type((*blockMatrix_)(block_i_, block_j_).impl_, rowInBlock_));
    //    rowIter_ = currentRow_->begin();
    //  }

    //  void setCurrentBlockRows()
    //  {
    //    currentBlockRows_ = (*blockMatrix_)(block_i_, block_j_).rows();
    //  }

    public:
      const value_type& operator*() const
      {
        if (isEnd_)
          throw std::out_of_range("Cannot dereference end iterator");

        if (impl_)
          return impl_->value();
        throw std::logic_error("null iterator impl");
      }

      const pointer operator->() const
      {
        return &(**this);
      }

      my_type& operator++()
      {
        if (!isEnd_)
        {
          ++(*impl_);
          if (!*impl_ && )
          {
            // move to next row.

          }
        }

        return *this;
      }

      my_type operator++(int)
      {
        my_type orig(*this);
        ++(*this);
        return orig;
      }

      bool operator==(const my_type& rhs)
      {
        if (isEnd_ && rhs.isEnd_)
          return true;

        if (isEnd_ != rhs.isEnd_)
          return false;

        return matrix_ == rhs.matrix_
          && impl_ == rhs.impl_;
      }

      bool operator!=(const my_type& rhs)
      {
        return !(*this == rhs);
      }

      /*SparseMatrix<double> mat(rows,cols);
      for (int k=0; k<mat.outerSize(); ++k)
      for (SparseMatrix<double>::InnerIterator it(mat,k); it; ++it)
      {
      it.value();
      }*/
    };

    NonZeroIterator nonZerosBegin() { return NonZeroIterator(this); }
    NonZeroIterator nonZerosEnd() { return NonZeroIterator(); }
#endif

    const MatrixBase<T>& castForPrinting() const { return *this; } /// @todo: lame...figure out a better way

    /// Persistent representation...
    virtual std::string dynamic_type_name() const override { return type_id.type; }
    virtual void io(Piostream&) override;
    static PersistentTypeID type_id;

    static Persistent* SparseRowMatrixGenericMaker();

  private:
    virtual void print(std::ostream& o) const override
    {
      o << static_cast<const EigenBase&>(*this);
    }
  };

  template <typename T>
  Persistent* SparseRowMatrixGeneric<T>::SparseRowMatrixGenericMaker()
  {
    return new SparseRowMatrixGeneric<T>;
  }


  template <typename T>
  PersistentTypeID SparseRowMatrixGeneric<T>::type_id("SparseRowMatrix", "MatrixBase",
    SparseRowMatrixGeneric<T>::SparseRowMatrixGenericMaker);

}}}

template <typename T>
bool ContainsValidValues(const SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>& m)
{
  for (int k = 0; k < m.outerSize(); ++k)
  {
    for (typename SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>::InnerIterator it(m,k); it; ++it)
    {
      double tmp = it.value();
      if (!SCIRun::IsFinite(tmp) || SCIRun::IsNan(tmp))
        return false;
    }
  }
  return true;
}

template <typename T>
bool isSymmetricMatrix(const SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>& m)
{
  if (m.rows() != m.cols())
    return false;

  for (int k = 0; k < m.outerSize(); ++k)
  {
    for (typename SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>::InnerIterator it(m,k); it; ++it)
    {
      if (m.coeff(it.col(), it.row()) != it.value())
        return false;
    }
  }
  return true;
}

template <typename T>
bool isSymmetricMatrix(const SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>& m, double bound)
{

  if (m.rows() != m.cols())
    return false;

  for (int k = 0; k < m.outerSize(); ++k)
  {
    for (typename SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>::InnerIterator it(m,k); it; ++it)
    {
      if (std::fabs(m.coeff(it.col(), it.row())-it.value()) > bound)
      {
         return false;
      }
    }
  }
  return true;
}

template <typename T>
bool isPositiveDefiniteMatrix(const SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>& m)
{
  if (!isSymmetricMatrix(m))
    return false;   //a matrix must be symmetric to be positive definite

  if ( !ContainsValidValues(m) )
    return false;

  for (int k = 0; k < m.outerSize(); ++k)  //all diagonal elements are positive?
    if ( m.coeff(k, k) <= 0 )
      return false;

  for (int k = 0; k < m.outerSize(); ++k)
  {
    double tmp1 = 0.0, tmp2 = 0.0;
    for (typename SCIRun::Core::Datatypes::SparseRowMatrixGeneric<T>::InnerIterator it(m,k); it; ++it)
    {
      if (it.col() != it.row())
      {
        tmp1 += std::fabs(it.value()); //abs. sum over col
        tmp2 += std::fabs(m.coeff(it.col(),it.row())); //abs. sum over row
      }
    }

    if ( !((tmp1 < m.coeff(k, k)) && (tmp2 < m.coeff(k, k))) )
      return false;
  }

  return true;
}

#include <Core/Datatypes/MatrixIO.h>

#endif
