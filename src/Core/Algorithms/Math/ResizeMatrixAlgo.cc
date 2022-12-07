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


#include <Core/Algorithms/Math/ResizeMatrixAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Math/MiscMath.h>

using namespace Eigen;
using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::Math::Parameters;

ALGORITHM_PARAMETER_DEF(Math, NoOfRows);
ALGORITHM_PARAMETER_DEF(Math, NoOfColumns);
ALGORITHM_PARAMETER_DEF(Math, Major);

ResizeMatrixAlgo::ResizeMatrixAlgo()
{
//set parameter defaults for UI
  addParameter(Parameters::NoOfRows, 1);
  addParameter(Parameters::NoOfColumns, 1);
  addOption(Parameters::Major,"Row","Row|Column");
}

AlgorithmOutput ResizeMatrixAlgo::run(const AlgorithmInput& input) const
{
  auto inputMatrix=input.get<Datatypes::Matrix>(Variables::InputMatrix);

  //pull parameter from UI
  int rows = get(Parameters::NoOfRows).toInt();
  int columns = get(Parameters::NoOfColumns).toInt();
  std::string major=getOption(Parameters::Major);
  DenseMatrixHandle dense=castMatrix::toDense(inputMatrix);
  DenseMatrixHandle denseMat(new DenseMatrix(*dense));

  if(major=="Column")
  {
    denseMat->transposeInPlace();
    std::swap(rows,columns);
  }

  Map<MatrixXd> result(denseMat->data(),columns,rows);
  DenseMatrixHandle outputArray(new DenseMatrix(result.matrix()));

  outputArray->transposeInPlace();

  if(denseMat->rows()*denseMat->cols() != rows*columns)
  {
    warning("Input size does not match the size of the Output Matrix, Matrix will be padded with zeros or cropped up.");

    double *resultMatrix=outputArray->data();
    for(int i=denseMat->rows()*denseMat->cols();i<rows*columns;i++)
    {
      resultMatrix[i]=0;
    }
  }

  if(major=="Column")
    outputArray->transposeInPlace();

  AlgorithmOutput output;
  output[Variables::OutputMatrix]=outputArray;
  return output;
}
