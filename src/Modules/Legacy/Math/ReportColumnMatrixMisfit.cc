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


///@file  ReportColumnMatrixMisfit.cc
///
///@author
///   David Weinstein
///   University of Utah
///@date  June 1999

#include <Modules/Legacy/Math/ReportColumnMatrixMisfit.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Algorithms/Legacy/Math/ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.h>

#include <Core/Math/MiscMath.h>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;

const ModuleLookupInfo ReportColumnMatrixMisfit::staticInfo_("ReportColumnMatrixMisfit", "Math", "SCIRun");

ReportColumnMatrixMisfit::ReportColumnMatrixMisfit() : Module(staticInfo_)
{
  INITIALIZE_PORT(Vec1);
  INITIALIZE_PORT(Vec2);
  INITIALIZE_PORT(Error_Out);
}

void ReportColumnMatrixMisfit::setStateDefaults()
{
  //todo

}


#if 0
namespace SCIRun {

/// @class ReportColumnMatrixMisfit
/// @brief This module computes and visualizes the error between two vectors.

class ReportColumnMatrixMisfit : public Module
{
public:
  explicit ReportColumnMatrixMisfit(GuiContext* ctx);
  virtual void execute();
private:
  GuiInt       have_ui_;
  GuiString    methodTCL_;
  GuiString    pTCL_;

  void showGraph(const ColumnMatrix& v1, const ColumnMatrix& v2, double ccInv, double rmsRel);
  bool containsInfiniteComponent(const ColumnMatrix& v);
};

DECLARE_MAKER(ReportColumnMatrixMisfit)

ReportColumnMatrixMisfit::ReportColumnMatrixMisfit(GuiContext* ctx)
  : Module("ReportColumnMatrixMisfit", ctx, Filter, "Math", "SCIRun"),
    have_ui_(get_ctx()->subVar("have_ui")),
    methodTCL_(get_ctx()->subVar("methodTCL")),
    pTCL_(get_ctx()->subVar("pTCL"))
{
}
#endif

void ReportColumnMatrixMisfit::execute()
{
  auto ivec1 = getRequiredInput(Vec1);
  auto ivec2 = getRequiredInput(Vec2);

  if (ivec1->nrows() != ivec2->nrows())
  {
    std::ostringstream ostr;
    ostr << "Can't compute error on vectors of different lengths!" <<
      "vec1 length = " << ivec1->nrows() << "vec2 length = " << ivec2->nrows();
    error(ostr.str());
    return;
  }

  auto state = get_state();
  const double pp = 2;//state->getValue(...).toDouble();

  ColumnMatrixMisfitCalculator calc(*ivec1, *ivec2, pp);

  const double cc = calc.getCorrelationCoefficient();
  const double ccInv = calc.getInverseCorrelationCoefficient();
  const double rms = calc.getRMS();
  const double rmsRel = calc.getRelativeRMS();

  #if 0
  if (have_ui_.get())
  {
    showGraph(*ivec1, *ivec2, ccInv, rmsRel);
  }
  #endif

  const std::string meth = "CC";//state->getValue(...).toString();
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

  sendOutput(Error_Out, boost::make_shared<Double>(val));
}

#if 0
void ReportColumnMatrixMisfit::showGraph(const ColumnMatrix& v1, const ColumnMatrix& v2, double ccInv, double rmsRel)
{
  if (containsInfiniteComponent(v1) || containsInfiniteComponent(v2))
    return;

  std::ostringstream str;
  str << get_id() << " append_graph " << ccInv << " " << rmsRel << " \"";

  for (int i = 0; i < v1.nrows(); ++i)
    str << i << " " << v1[i] << " ";
  str << "\" \"";

  for (int i = 0; i < v2.nrows(); ++i)
    str << i << " " << v2[i] << " ";
  str << "\" ; update idletasks";

  TCLInterface::execute(str.str());
}

bool ReportColumnMatrixMisfit::containsInfiniteComponent(const ColumnMatrix& v)
{
  if (std::find_if(v.begin(), v.end(), IsInfinite) != v.end())
  {
    remark("Input vector contains infinite values, graph not updated.");
    return true;
  }
  return false;
}

} // End namespace SCIRun

#endif
