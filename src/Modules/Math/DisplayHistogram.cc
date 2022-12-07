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


#include <Modules/Math/DisplayHistogram.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;

MODULE_INFO_DEF(DisplayHistogram, Math, SCIRun)

DisplayHistogram::DisplayHistogram() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
}

void DisplayHistogram::execute()
{
  auto input_matrix = getRequiredInput(InputMatrix);

  if (needToExecute())
  {
    if (input_matrix->empty())
    {
      THROW_INVALID_ARGUMENT("Empty matrix input.");
    }
    auto dense = castMatrix::toDense(input_matrix);
    if (!dense)
    {
      THROW_INVALID_ARGUMENT("Matrix input must be dense.");
    }
    std::vector<double> data;
    data.reserve(dense->size());
    for (auto i = 0; i < dense->nrows(); ++i)
      for (auto j = 0; j < dense->ncols(); ++j)
        data.push_back((*dense)(i,j));
    get_state()->setTransientValue(Variables::InputMatrix, data);
  }
}
