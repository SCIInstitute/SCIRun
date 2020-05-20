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


#ifndef CORE_DATATYPES_DENSE_COLUMN_MATRIX_H
#define CORE_DATATYPES_DENSE_COLUMN_MATRIX_H

#include <Core/Datatypes/Matrix.h>
#define register
#include <Eigen/Dense>
#undef register

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class DenseColumnMatrixGeneric : public MatrixBase<T>, public Eigen::Matrix<T, Eigen::Dynamic, 1>
  {
  public:
    typedef T value_type;
    typedef DenseColumnMatrixGeneric<T> this_type;
    typedef Eigen::Matrix<T, Eigen::Dynamic, 1> EigenBase;

    explicit DenseColumnMatrixGeneric(size_t nrows = 0) : EigenBase(nrows) {}

    /// This constructor allows you to construct DenseColumnMatrixGeneric from Eigen expressions
    template<typename OtherDerived>
    DenseColumnMatrixGeneric(const Eigen::MatrixBase<OtherDerived>& other)
      : EigenBase(other)
    { }

    /// This method allows you to assign Eigen expressions to DenseColumnMatrixGeneric
    template<typename OtherDerived>
    DenseColumnMatrixGeneric& operator=(const Eigen::MatrixBase<OtherDerived>& other)
    {
      this->EigenBase::operator=(other);
      return *this;
    }

    virtual DenseColumnMatrixGeneric* clone() const override
    {
      return new DenseColumnMatrixGeneric(*this);
    }

    virtual void accept(MatrixVisitorGeneric<T>& visitor) override
    {
      visitor.visit(*this);
    }

    virtual size_t nrows() const override { return this->rows(); }
    virtual size_t ncols() const override { return this->cols(); }
    virtual T get(int i, int j) const override
    {
      return (*this)(i,j);
    }
    virtual void put(int i, int j, const T& val) override
    {
      (*this)(i,j) = val;
    }

    /// Persistent representation...
    virtual std::string dynamic_type_name() const override { return type_id.type; }
    virtual void io(Piostream&) override;
    static PersistentTypeID type_id;

  private:
    virtual void print(std::ostream& o) const override
    {
      o << static_cast<const EigenBase&>(*this);
    }
  };

  template <typename T>
  static Persistent* ColumnMatrixMaker()
  {
    return new DenseColumnMatrixGeneric<T>();
  }

  template <typename T>
  PersistentTypeID DenseColumnMatrixGeneric<T>::type_id("ColumnMatrix", "MatrixBase", ColumnMatrixMaker<T>);

}}}


#endif
