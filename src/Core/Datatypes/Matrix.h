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


/// @todo Documentation Core/Datatypes/Matrix.h

#ifndef CORE_DATATYPES_MATRIX_H
#define CORE_DATATYPES_MATRIX_H

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <iosfwd>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  /// @todo: use Eigen's Visitor type?
  template <typename T>
  struct SCISHARE MatrixVisitorGeneric
  {
    virtual ~MatrixVisitorGeneric() {}
    virtual void visit(DenseMatrixGeneric<T>&) = 0;
    virtual void visit(SparseRowMatrixGeneric<T>&) = 0;
    virtual void visit(DenseColumnMatrixGeneric<T>&) = 0;
  };

  class SCISHARE MatrixIOBase : public Datatype
  {
  public:
    // Persistent representation.
    MatrixIOBase() : separate_raw_(false) {}
    virtual std::string dynamic_type_name() const override { return "MatrixIOBase"; }
    virtual void io(Piostream&) override;
    static PersistentTypeID type_id;
  protected:
    bool           separate_raw_;
    std::string    raw_filename_;
  };

  template <typename T>
  class MatrixBase : public MatrixIOBase, public HasPropertyManager
  {
  public:
    using value_type = T;
    virtual size_t nrows() const = 0;
    virtual size_t ncols() const = 0;
    size_t get_dense_size() const { return nrows() * ncols(); }

    using Visitor = MatrixVisitorGeneric<T>;
    virtual void accept(Visitor& visitor) = 0;

    bool empty() const { return 0 == nrows() || 0 == ncols(); }

    friend std::ostream& operator<<(std::ostream& o, const MatrixBase<T>& m)
    {
      m.print(o);
      return o;
    }

    virtual T get(int i, int j) const = 0;
    virtual void put(int i, int j, const T& val) = 0;

    static PersistentTypeID type_id;

    virtual MatrixBase<T>* clone() const override = 0;

  private:
    virtual void print(std::ostream&) const = 0;
      /// @todo: not much to go here for now.
  };

  template <typename T>
  PersistentTypeID MatrixBase<T>::type_id("MatrixBase", "MatrixIOBase", nullptr);

  enum SCISHARE MatrixTypeCode
  {
    NULL_MATRIX = -1,
    DENSE = 0,
    SPARSE_ROW,
    COLUMN,
    UNKNOWN
  };

}}}


#endif
