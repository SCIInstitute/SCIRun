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


/// @file  ReportColumnMatrixMisfit.cc
///
/// @author
///    David Weinstein
///    University of Utah
/// @date  June 1999

#include <Modules/Legacy/Math/ReportColumnMatrixMisfit.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Algorithms/Math/ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

#include <Core/Math/MiscMath.h>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;

/// @class ReportColumnMatrixMisfit
/// @brief This module computes and visualizes the error between two vectors.

MODULE_INFO_DEF(ReportColumnMatrixMisfit, Math, SCIRun)

ReportColumnMatrixMisfit::ReportColumnMatrixMisfit() : Module(staticInfo_)
{
  INITIALIZE_PORT(Vec1);
  INITIALIZE_PORT(Vec2);
  INITIALIZE_PORT(Error_Out);
}

void ReportColumnMatrixMisfit::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::PValue, 2.0);
  state->setValue(Parameters::MisfitMethod, std::string("CCinv"));
  state->setValue(Parameters::ccInv, 0.0);
  state->setValue(Parameters::rmsRel, 0.0);
}

void ReportColumnMatrixMisfit::execute()
{
  auto ivec1 = getRequiredInput(Vec1);
  auto ivec2 = getRequiredInput(Vec2);

  if (needToExecute())
  {
    if (ivec1->nrows() != ivec2->nrows())
    {
      std::ostringstream ostr;
      ostr << "Can't compute error on vectors of different lengths!" <<
        "vec1 length = " << ivec1->nrows() << "vec2 length = " << ivec2->nrows();
      error(ostr.str());
      return;
    }

    if (ivec1->ncols() != 1 || ivec2->ncols() != 1)
    {
      std::ostringstream ostr;
      ostr << "Can't compute error on vectors of different lengths!" <<
        "vec1 length = " << ivec1->nrows() << "vec2 length = " << ivec2->nrows();
      error(ostr.str());
      return;
    }

    auto state = get_state();
    const double pp = state->getValue(Parameters::PValue).toDouble();

    auto ivec1Col = convertMatrix::toColumn(ivec1);
    auto ivec2Col = convertMatrix::toColumn(ivec2);

    ColumnMatrixMisfitCalculator calc(*ivec1Col, *ivec2Col, pp);

    const double cc = calc.getCorrelationCoefficient();
    const double ccInv = calc.getInverseCorrelationCoefficient();
    const double rms = calc.getRMS();
    const double rmsRel = calc.getRelativeRMS();

    showGraph(*ivec1Col, *ivec2Col, ccInv, rmsRel);

    const std::string meth = state->getValue(Parameters::MisfitMethod).toString();
    double val;
    if (meth == "CC")
    {
      val = cc;
    }
    else if (meth == "CCinv")
    {
      val = ccInv;
    }
    else if (meth == "RMS")
    {
      val = rms;
    }
    else if (meth == "RMSrel")
    {
      val = rmsRel;
    }
    else
    {
      error("Unknown ReportColumnMatrixMisfit method - " + meth);
      val = 0;
    }
    std::cout << "Error val: " << val << std::endl;
    //sendOutput(Error_Out, boost::make_shared<Double>(val));
  }
}


void ReportColumnMatrixMisfit::showGraph(const DenseColumnMatrix& v1, const DenseColumnMatrix& v2, double ccInv, double rmsRel)
{
  if (containsInfiniteComponent(v1) || containsInfiniteComponent(v2))
    return;

  auto state = get_state();
  state->setValue(Parameters::ccInv, ccInv);
  state->setValue(Parameters::rmsRel, rmsRel);

#if SCIRUN4_CODE_TO_BE_ENABLED_LATER
  std::ostringstream str;
  str << get_id() << " append_graph " << ccInv << " " << rmsRel << " \"";

  for (int i = 0; i < v1.nrows(); ++i)
    str << i << " " << v1[i] << " ";
  str << "\" \"";

  for (int i = 0; i < v2.nrows(); ++i)
    str << i << " " << v2[i] << " ";
  str << "\" ; update idletasks";

  TCLInterface::execute(str.str());
#endif
}

bool ReportColumnMatrixMisfit::containsInfiniteComponent(const DenseColumnMatrix& v) const
{
  for (int i = 0; i < v.size(); i++)
  {
    if (IsInfinite(v[i]))
    {
      remark("Input vector contains infinite values, graph not updated.");
      return true;
    }
  }
  return false;
}
