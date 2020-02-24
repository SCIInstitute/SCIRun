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


#ifndef CORE_DATATYPES_DENSE_MATRIX_H
#define CORE_DATATYPES_DENSE_MATRIX_H

#include <Core/Datatypes/Matrix.h>
#include <Core/GeometryPrimitives/Transform.h> /// @todo
#define register
#include <Eigen/Dense>
#undef register
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <typename T>
  class DenseMatrixGeneric : public MatrixBase<T>, public Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor | Eigen::AutoAlign>
  {
  public:
    typedef T value_type;
    typedef DenseMatrixGeneric<T> this_type;
    typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor | Eigen::AutoAlign> EigenBase;

    /// @todo: don't like this default ctor, since it doesn't act like a vector.
    //DenseMatrixGeneric() : Base() {}
    explicit DenseMatrixGeneric(size_t nrows = 0, size_t ncols = 0) : EigenBase(nrows, ncols) {}

    DenseMatrixGeneric(size_t nrows, size_t ncols, const T& val) : EigenBase(nrows, ncols)
    {
      this->fill(val);
    }

    /// This constructor allows you to construct DenseMatrixGeneric from Eigen expressions
    template<typename OtherDerived>
    DenseMatrixGeneric(const Eigen::MatrixBase<OtherDerived>& other)
      : EigenBase(other)
    { }

    explicit DenseMatrixGeneric(const Geometry::Transform& t);

    /// This method allows you to assign Eigen expressions to DenseMatrixGeneric
    template<typename OtherDerived>
    DenseMatrixGeneric& operator=(const Eigen::MatrixBase<OtherDerived>& other)
    {
      this->EigenBase::operator=(other);
      return *this;
    }

    template<typename OtherDerived>
    DenseMatrixGeneric& operator=(const Eigen::ArrayBase<OtherDerived>& other)
    {
      this->EigenBase::operator=(other);
      return *this;
    }

    virtual DenseMatrixGeneric* clone() const override
    {
      return new DenseMatrixGeneric(*this);
    }

    virtual size_t nrows() const override { return this->rows(); }
    virtual size_t ncols() const override { return this->cols(); }

    virtual void accept(MatrixVisitorGeneric<T>& visitor) override
    {
      visitor.visit(*this);
    }

    /// Persistent representation...
    virtual std::string dynamic_type_name() const override;
    virtual void io(Piostream&) override;
    static PersistentTypeID type_id;
    static PersistentMaker0 maker0;

    template <typename XYZ>
    static this_type fromPoint(const XYZ& xyz)
    {
      this_type p(3, 1);
      p(0, 0) = xyz.x();
      p(1, 0) = xyz.y();
      p(2, 0) = xyz.z();
      return p;
    }

    bool isSymmetric() const
    {
      if (this->nrows() != this->ncols())
        return false;
      return this->isApprox(this->transpose());
    }

    virtual T get(int i, int j) const override
    {
      return (*this)(i,j);
    }
    virtual void put(int i, int j, const T& val) override
    {
      (*this)(i,j) = val;
    }

  private:
    virtual void print(std::ostream& o) const override
    {
      /// @todo!!
      //o << static_cast<const EigenBase&>(m);
      for (size_t i = 0; i < nrows(); ++i)
      {
        for (size_t j = 0; j < ncols(); ++j)
        {
          o << (*this)(i,j) << " ";
        }
        o << "\n";
      }
    }
  };

  namespace
  {
    template <typename T>
    Persistent* DenseMatrixGenericMaker()
    {
      return new DenseMatrixGeneric<T>;
    }
  }

  template <typename T>
  PersistentMaker0 DenseMatrixGeneric<T>::maker0(DenseMatrixGenericMaker<T>);

  template <typename T>
  std::string DenseMatrixGeneric<T>::dynamic_type_name() const { return type_id.type; }

  template <typename T>
  PersistentTypeID DenseMatrixGeneric<T>::type_id("DenseMatrix", "MatrixBase", maker0);

  template <typename T>
  DenseMatrixGeneric<T>::DenseMatrixGeneric(const Geometry::Transform& t) : EigenBase(4, 4)
  {
    T data[16];
    t.get(data);
    T* ptr = data;

    for (auto i = 0; i < this->rows(); ++i)
    {
      for (auto j = 0; j < this->cols(); ++j)
      {
        (*this)(i,j) = static_cast<T>(*ptr++);
      }
    }
  }

  SCISHARE ComplexDenseMatrix makeComplexMatrix(const DenseMatrix& real, const DenseMatrix& imag);

}}}

// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/MatrixIO.h>

#endif
