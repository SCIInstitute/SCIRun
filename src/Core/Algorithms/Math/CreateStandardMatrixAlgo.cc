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


#include <Core/Algorithms/Math/CreateStandardMatrixAlgo.h>
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
ALGORITHM_PARAMETER_DEF(Math, Size);
ALGORITHM_PARAMETER_DEF(Math, StartPointer);

CreateStandardMatrixAlgo::CreateStandardMatrixAlgo()
{
    //set parameter defaults for UI
    addOption(Parameters::MatrixType,"One","Zero|One|NaN|Identity|Series");
    //auto state = get_state();
    //using namespace Parameters;
    addParameter(Parameters::Rows, 1);
    addParameter(Parameters::Columns, 1);
    addParameter(Parameters::Size, 1);
    addParameter(Parameters::StartPointer,0);
    
    }

AlgorithmOutput CreateStandardMatrixAlgo::run(const AlgorithmInput& input) const{
    
    //pull parameter from UI
    std::string matrixType=getOption(Parameters::MatrixType);
    
    
    int rows = get(Parameters::Rows).toInt();
    int columns = get(Parameters::Columns).toInt();
    auto result=generateMatrix(matrixType, rows, columns);
     AlgorithmOutput output;
    output[Variables::OutputMatrix]=result;
    
    return output;
}

Datatypes::DenseMatrixHandle CreateStandardMatrixAlgo::generateMatrix(std::string matrixType, int rows, int columns) const
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
        DenseMatrix outputArray(rows,columns, std::numeric_limits<double>::quiet_NaN());
        DenseMatrixHandle output(new DenseMatrix(outputArray.matrix()));
        return output;
        
    }

    if(matrixType=="Identity")
    {
        outputArray=MatrixXd::Identity(rows,columns);
    }

    if(matrixType=="Series")
    {
        int size = get(Parameters::Size).toInt();
        int inc = get(Parameters::StartPointer).toInt();
        auto outputArray=boost::make_shared<DenseMatrix>(rows,columns);
        for(int i=0;i<rows;i++)
        {
            for(int j=0;j<columns;j++)
            {
                (*outputArray)(i,j)=inc;
                inc+=size;
            }
        }
        return outputArray;
    }
    
    
    DenseMatrixHandle output(new DenseMatrix(outputArray.matrix()));
    return output;
    
    
}

