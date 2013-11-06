/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <Eigen/SparseCore>

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

    //TODO: need C++11
    //using Base::Base;

    SparseRowMatrixGeneric() : EigenBase() {}
    SparseRowMatrixGeneric(int nrows, int ncols) : EigenBase(nrows, ncols) {}

    // This constructor allows you to construct SparseRowMatrixGeneric from Eigen expressions
    template<typename OtherDerived>
    SparseRowMatrixGeneric(const Eigen::SparseMatrixBase<OtherDerived>& other)
      : EigenBase(other)
    { }

    // This method allows you to assign Eigen expressions to SparseRowMatrixGeneric
    template<typename OtherDerived>
    SparseRowMatrixGeneric& operator=(const Eigen::SparseMatrixBase<OtherDerived>& other)
    {
      this->EigenBase::operator=(other);
      return *this;
    }

    virtual SparseRowMatrixGeneric* clone() const 
    {
      return new SparseRowMatrixGeneric(*this);
    }

    virtual size_t nrows() const { return this->rows(); }
    virtual size_t ncols() const { return this->cols(); }

    virtual void accept(MatrixVisitorGeneric<T>& visitor)
    {
      visitor.visit(*this);
    }

    //TODO!
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

    const MatrixBase<T>& castForPrinting() const { return *this; } //TODO: lame...figure out a better way

    //! Persistent representation...
    virtual std::string dynamic_type_name() const { return type_id.type; }
    virtual void io(Piostream&);
    static PersistentTypeID type_id;

    static Persistent* SparseRowMatrixGenericMaker();

  private:
    virtual void print(std::ostream& o) const
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
#include <math.h>
#include <iostream>

template <typename SparseRowMatrixGeneric> inline bool ContainsValidValues (const SparseRowMatrixGeneric &matrix)
{
 for (SCIRun::index_type i=0; i<(*matrix).rows();i++)
     { 
       for (Eigen::SparseVector<double>::InnerIterator it((*matrix).row(i)); it; ++it)
       {
        it.value(); 
        it.index();	
	double tmp=(*matrix).coeff(i,it.index());
	if ( !isfinite(tmp) || isnan(tmp) ) return false;
       }
     } 
     return true;
}


template <typename SparseRowMatrixGeneric> inline bool isPositiveDefiniteMatrix (const SparseRowMatrixGeneric & matrix)
  {
     if ((*matrix).rows() != (*matrix).cols()) return false;   
         
     if ( !ContainsValidValues(matrix) ) return false; 
	 
     for (SCIRun::index_type i=0; i<(*matrix).rows();i++) //all diagonal elements positive?
        if ( (*matrix).coeff(i, i) <= 0 )  
                   return false;
       
     for (SCIRun::index_type i=0; i<(*matrix).rows();i++)
     { 
       double tmp1=0.0;
       for (Eigen::SparseVector<double>::InnerIterator it((*matrix).row(i)); it; ++it)
       {
        it.value(); 
        it.index();	
	if (i!=it.index()) tmp1+=fabs((*matrix).coeff(i,it.index())); //abs. sum over row
       }
       double tmp2=0.0;
       
       for (Eigen::SparseVector<double>::InnerIterator it((*matrix).col(i)); it; ++it)
       {
        it.value(); 
        it.index();	
	if (i!=it.index()) tmp2+=fabs((*matrix).coeff(it.index(),i)); //abs. sum over col	
       }    
       if (tmp1!=tmp2) return false; //probably not symetric since sums of abs. values are different
       if ((tmp1>(*matrix).coeff(i, i)) || (tmp2>(*matrix).coeff(i, i))) return false; 
     }
    
    return true;
  }


template <typename SparseRowMatrixGeneric> inline bool isSymmetricMatrix (const SparseRowMatrixGeneric & matrix)
  {
   
     if ((*matrix).rows() != (*matrix).cols()) return false;
     
     if ( !ContainsValidValues(matrix) ) return false; 
     
     for (SCIRun::index_type i=0; i<(*matrix).rows();i++)
     { 
       for (Eigen::SparseVector<double>::InnerIterator it((*matrix).row(i)); it; ++it)
       {
        it.value(); 
        it.index();
	
	if( (*matrix).coeff(i, it.index()) !=(*matrix).coeff(it.index(),i))
	   return false;
       }
     }
    
    return true;
  }


#include <Core/Datatypes/MatrixIO.h>

#endif
