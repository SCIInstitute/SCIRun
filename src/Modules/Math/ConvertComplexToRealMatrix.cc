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


/// author: Moritz Dannhauer & Kimia Shayestehfard
/// last change: 02/16/17

#include <Modules/Math/ConvertComplexToRealMatrix.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <deque>

using namespace SCIRun;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ConvertComplexToRealMatrix, Converters, SCIRun)

ConvertComplexToRealMatrix::ConvertComplexToRealMatrix() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputComplexMatrix);
  INITIALIZE_PORT(OutputRealPartMatrix);
  INITIALIZE_PORT(OutputComplexPartMatrix);
  INITIALIZE_PORT(Magnitude);
  INITIALIZE_PORT(Phase);
}

namespace
{
  complex Arg(const complex& c)
  {
    return { std::arg(c), 0 };
  }

  struct RealPart
  {
    template <class M>
    auto operator()(const M& matrix) const -> decltype(matrix.real())
    {
      return matrix.real();
    }
  };

  struct ImagPart
  {
    template <class M>
    auto operator()(const M& matrix) const -> decltype(matrix.imag())
    {
      return matrix.imag();
    }
  };

  struct MagnitudeFunc
  {
    template <class M>
    auto operator()(const M& matrix) const -> decltype(matrix.cwiseAbs())
    {
      return matrix.cwiseAbs();
    }
  };

  struct PhaseFunc
  {
    template <class M>
    auto operator()(const M& matrix) const -> decltype(matrix.unaryExpr(&Arg).real())
    {
      return matrix.unaryExpr(&Arg).real();
    }
  };

  template <class M>
  struct MoveToHeap
  {
    template <class Expr>
    MatrixHandleGeneric<typename M::value_type> operator()(Expr&& expr) const
    {
      return boost::make_shared<M>(expr);
    }
  };

  template <class Func, class T1, class T2>
  struct MatrixFuncEvaluator
  {
    Func f;

    MatrixHandleGeneric<T2> operator()(const MatrixTuple<T1>& mats)
    {
      auto a = std::get<DENSE>(mats);
      if (a)
      {
        MoveToHeap<DenseMatrixGeneric<T2>> m;
        return m(f(*a));
      }
      auto b = std::get<SPARSE_ROW>(mats);
      if (b)
      {
        MoveToHeap<SparseRowMatrixGeneric<T2>> m;
        return m(f(*b));
      }
      auto c = std::get<COLUMN>(mats);
      if (c)
      {
        MoveToHeap<DenseColumnMatrixGeneric<T2>> m;
        return m(f(*c));
      }
      return nullptr;
    }
  };

  template <class Func>
  using ComplexToRealMatrixFuncEvaluator = MatrixFuncEvaluator<Func, complex, double>;
}

void ConvertComplexToRealMatrix::execute()
{
  auto complexMatrix = getRequiredInput(InputComplexMatrix);

  if (needToExecute())
  {
    if (!isKnownMatrixType(complexMatrix))
    {
      error("Unknown matrix type");
      return;
    }

    auto subtypes = explodeBySubtype(complexMatrix);
    computeOutputAndSendIfConnected(OutputRealPartMatrix,
      [subtypes]()
      {
        return ComplexToRealMatrixFuncEvaluator<RealPart>()(subtypes);
      });
    computeOutputAndSendIfConnected(OutputComplexPartMatrix,
      [subtypes]()
      {
        return ComplexToRealMatrixFuncEvaluator<ImagPart>()(subtypes);
      });
    computeOutputAndSendIfConnected(Magnitude,
      [subtypes]()
      {
        return ComplexToRealMatrixFuncEvaluator<MagnitudeFunc>()(subtypes);
      });
    computeOutputAndSendIfConnected(Phase,
      [subtypes]()
      {
        return ComplexToRealMatrixFuncEvaluator<PhaseFunc>()(subtypes);
      });
  }
}
