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


#include <Core/Algorithms/Math/GenerateStandardMatrixAlgo.h>
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

ALGORITHM_PARAMETER_DEF(Math, MatrixType);
ALGORITHM_PARAMETER_DEF(Math, Rows);
ALGORITHM_PARAMETER_DEF(Math, Columns);

GenerateStandardMatrixAlgo::GenerateStandardMatrixAlgo()
{
    //set parameter defaults for UI
    addOption(Parameters::MatrixType,"One","Zero|One|NaN|Identity|Series");
    //auto state = get_state();
    //using namespace Parameters;
    addParameter(Parameters::Rows, 1);
    addParameter(Parameters::Columns, 1);


    
    }

AlgorithmOutput GenerateStandardMatrixAlgo::run(const AlgorithmInput& input) const{
    
    //pull parameter from UI
    std::string matrixType=getOption(Parameters::MatrixType);
    
    //int rows = state->getValue(Parameters::Rows).toInt();
    //int columns = state->getValue(Parameters::Columns).toInt();
    
    int rows = get(Parameters::Rows).toInt();
    int columns = get(Parameters::Columns).toInt();
    auto outputMatrix=generateMatrix(matrixType, rows, columns);
     AlgorithmOutput output;
    output[Variables::OutputMatrix]=outputMatrix;
    
    return output;
}

Datatypes::DenseMatrixHandle GenerateStandardMatrixAlgo::generateMatrix(std::string matrixType, int rows, int columns) const
{
    if(rows<=0 || columns<=0)
    {
        error("Invalid Input for the no. of Rows or Columns");
        DenseMatrixHandle output;
        return output;
    }
    
    DenseMatrix outputArray;
    
    if(matrixType=="Zero")
    {
        outputArray=MatrixXd::Constant(rows, columns, 0);
    }
    
    if(matrixType=="One")
    {
        outputArray=MatrixXd::Constant(rows, columns, 1);
    }
    
    if(matrixType=="NaN")
    {
        error("Not yet developed");
    }

    if(matrixType=="Identity")
    {
        outputArray=MatrixXd::Identity(rows,columns);
    }

    if(matrixType=="Series")
    {
        
        error("Not yet developed");
    }
    
    
    DenseMatrixHandle outputMatrix(new DenseMatrix(outputArray.matrix()));
    return outputMatrix;
    
    
}

