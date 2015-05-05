/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun;

EvaluateLinearAlgebraBinaryAlgorithm::EvaluateLinearAlgebraBinaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
  addParameter(Variables::FunctionString, std::string("x+y"));
}

namespace impl //TODO: break out; useful in general
{
  class BinaryVisitor : public MatrixVisitor
  {
  protected:
    explicit BinaryVisitor(MatrixHandle operand) : typeCode_(matrix_is::typeCode(operand)) {}
    static Matrix* cloneIfNotNull(MatrixHandle m)
    {
      ENSURE_NOT_NULL(m, "Addend");
      return m->clone();
    }

    MatrixTypeCode typeCode_;
  };

  class AddMatrices : public BinaryVisitor
  {
  public:
    explicit AddMatrices(MatrixHandle addend) : BinaryVisitor(addend),
      sum_(cloneIfNotNull(addend))
    {
    }

    virtual void visit(DenseMatrixGeneric<double>& dense) override
    {
      switch (typeCode_)
      {
      case DENSE:
        *matrix_cast::as_dense(sum_) += dense;
        break;
      case COLUMN:
        *matrix_cast::as_column(sum_) += dense;
        break;
      case SPARSE_ROW:
        *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + *matrix_convert::denseToSparse(dense);
        break;
      }
    }
    virtual void visit(SparseRowMatrixGeneric<double>& sparse) override
    {
      switch (typeCode_)
      {
      case DENSE:
        sum_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(sum_) + sparse));
        typeCode_ = SPARSE_ROW;
        break;
      case COLUMN:
        sum_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(sum_) + sparse));
        typeCode_ = SPARSE_ROW;
        break;
      case SPARSE_ROW:
        *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + sparse;
        break;
      }
    }
    virtual void visit(DenseColumnMatrixGeneric<double>& column) override
    {
      switch (typeCode_)
      {
      case DENSE:
        *matrix_cast::as_dense(sum_) += column;
        break;
      case COLUMN:
        *matrix_cast::as_column(sum_) += column;
        break;
      case SPARSE_ROW:
        *matrix_cast::as_sparse(sum_) = *matrix_cast::as_sparse(sum_) + *matrix_convert::denseToSparse(column);
        break;
      }
    }

    MatrixHandle sum_;
  };

  class MultiplyMatrices : public BinaryVisitor
  {
  public:
    explicit MultiplyMatrices(MatrixHandle factor) : BinaryVisitor(factor),
      product_(cloneIfNotNull(factor))
      {
      }

      virtual void visit(DenseMatrixGeneric<double>& dense) override
      {
        switch (typeCode_)
        {
        case DENSE:
          *matrix_cast::as_dense(product_) *= dense;
          break;
        case COLUMN:
          *matrix_cast::as_column(product_) *= dense;
          break;
        case SPARSE_ROW:
          *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * *matrix_convert::denseToSparse(dense);
          break;
        }
      }
      virtual void visit(SparseRowMatrixGeneric<double>& sparse) override
      {
        switch (typeCode_)
        {
        case DENSE:
          product_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(product_) * sparse));
          typeCode_ = SPARSE_ROW;
          break;
        case COLUMN:
          product_.reset(new SparseRowMatrix(*matrix_convert::to_sparse(product_) * sparse));
          typeCode_ = SPARSE_ROW;
          break;
        case SPARSE_ROW:
          *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * sparse;
          break;
        }
      }
      virtual void visit(DenseColumnMatrixGeneric<double>& column) override
      {
        switch (typeCode_)
        {
        case DENSE:
          *matrix_cast::as_dense(product_) *= column;
          break;
        case COLUMN:
          *matrix_cast::as_column(product_) *= column;
          break;
        case SPARSE_ROW:
          *matrix_cast::as_sparse(product_) = *matrix_cast::as_sparse(product_) * *matrix_convert::denseToSparse(column);
          break;
        }
      }

      MatrixHandle product_;
  };
}

EvaluateLinearAlgebraBinaryAlgorithm::Outputs EvaluateLinearAlgebraBinaryAlgorithm::run(const EvaluateLinearAlgebraBinaryAlgorithm::Inputs& inputs, const EvaluateLinearAlgebraBinaryAlgorithm::Parameters& params) const
{
  MatrixHandle result;
  auto lhs = inputs.get<0>();
  auto rhs = inputs.get<1>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(lhs, "lhs");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(rhs, "rhs");

  Operator oper = params.get<0>();
  switch (oper)
  {
  case ADD:
  {
    if (lhs->nrows() != rhs->nrows() || lhs->ncols() != rhs->ncols())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to add matrices.");
    impl::AddMatrices add(lhs);
    rhs->accept(add);
    return add.sum_;
  }
  case SUBTRACT:
  {
    if (lhs->nrows() != rhs->nrows() || lhs->ncols() != rhs->ncols())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to subtract matrices.");
    result.reset(rhs->clone());
    detail::NegateMatrix neg;
    result->accept(neg);
    impl::AddMatrices add(lhs);
    result->accept(add);
    return add.sum_;
  }
  case MULTIPLY:
  {
    if (lhs->ncols() != rhs->nrows())
      THROW_ALGORITHM_INPUT_ERROR("Invalid dimensions to multiply matrices.");
    impl::MultiplyMatrices mult(lhs);
    rhs->accept(mult);
    return mult.product_;
  }
  case FUNCTION:
  {
    NewArrayMathEngine engine;
    MatrixHandle lhsInput(lhs->clone()), rhsInput(rhs->clone());

    if (!(engine.add_input_fullmatrix("x", lhsInput)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
    if (!(engine.add_input_fullmatrix("y", rhsInput)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");

    boost::optional<std::string> func = params.get<1>();
    std::string function_string = func.get();

    function_string = "RESULT=" + function_string;
    engine.add_expressions(function_string);

    //bad API: how does it know what type/size the output matrix should be? Here are my guesses:
    MatrixHandle omatrix;
    if (matrix_is::sparse(lhs))
      omatrix.reset(lhs->clone());
    else if (matrix_is::sparse(rhs))
      omatrix.reset(rhs->clone());
    else if (matrix_is::dense(lhs) && matrix_is::dense(rhs))
      omatrix.reset(lhs->clone());
    else
      omatrix = matrix_convert::to_sparse(lhs);

    if (!(engine.add_output_fullmatrix("RESULT", omatrix)))
      THROW_ALGORITHM_INPUT_ERROR("Error setting up parser");
    if (!(engine.run()))
      THROW_ALGORITHM_INPUT_ERROR("Error running math engine");
    result = omatrix;
  }
  break;
  default:
    THROW_ALGORITHM_INPUT_ERROR("ERROR: unknown binary operation");
    break;
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraBinaryAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto LHS = input.get<Matrix>(Variables::LHS);
  auto RHS = input.get<Matrix>(Variables::RHS);
  auto func = boost::make_optional(get(Variables::FunctionString).toString());

  auto result = run(boost::make_tuple(LHS, RHS), boost::make_tuple(Operator(get(Variables::Operator).toInt()), func));

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}
