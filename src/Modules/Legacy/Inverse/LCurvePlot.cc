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


///  File       : LCurvePlot.cc
///  Author     : Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White, Jess Tate
///  Date       : Oct 25th, 2017 (last update)

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Legacy/Inverse/LCurvePlot.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Inverse;

LCurvePlot::LCurvePlot()
{
}

std::string LCurvePlot::update_lcurve_gui(const std::string& module_id,
  const DenseMatrixHandle& lambda, const DenseMatrixHandle& input, const DenseMatrixHandle& lambda_index)
{
  int lam_ind = static_cast<int>(lambda_index->get(0,0));

  size_t nLambda = input->rows();

  auto eta = input->col(1);
  auto rho = input->col(2);

  //estimate L curve corner
  const double lower_y = std::min(eta[0] / 10.0, eta[nLambda - 1]);

  std::ostringstream str;
  str << module_id << " plot_graph \" ";
  for (int k = 0; k < nLambda; k++)    str << log10(rho[k]) << " " << log10(eta[k]) << " ";

  cornerPlot_.assign({log10(rho[0] / 10.0), log10(eta[lam_ind]), log10(rho[lam_ind]), log10(eta[lam_ind]), log10(rho[lam_ind]), log10(lower_y)});

  str << "\" \" " << cornerPlot_[0] << " " << cornerPlot_[1] << " ";
  str << cornerPlot_[2] << " " << cornerPlot_[3] << " ";
  str << cornerPlot_[4] << " " << cornerPlot_[5] << " \" ";
  str << lambda->get(0,0) << " " << lam_ind << " ; \n";

  return str.str();
}
