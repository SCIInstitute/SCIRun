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


#include <Core/Algorithms/Math/BooleanCompareAlgo.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Math/MiscMath.h>
#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

ALGORITHM_PARAMETER_DEF(Math, Value_Option_1);
ALGORITHM_PARAMETER_DEF(Math, Value_Option_2);
ALGORITHM_PARAMETER_DEF(Math, Then_Option);
ALGORITHM_PARAMETER_DEF(Math, Else_Option);
ALGORITHM_PARAMETER_DEF(Math, Comparison_Option);

BooleanCompareAlgo::BooleanCompareAlgo()
{
  //set parameter defaults for UI
  using namespace Parameters;

  addOption(Comparison_Option, "boolop", "boolop|andop|orop|lessop|lesseqop|eqop|greateqop|greatop");
  addOption(Value_Option_1, "value", "value|size|norm");
  addOption(Value_Option_2, "value", "value|size|norm");
  addOption(Then_Option, "first", "null|first|second|third|quit");
  addOption(Else_Option, "null", "null|first|second|third|quit");
}


AlgorithmOutput BooleanCompareAlgo::run(const AlgorithmInput& input) const
{

  auto matrixa = input.get<Matrix>(Variables::FirstMatrix);
  auto matrixb = input.get<Matrix>(Variables::SecondMatrix);
  auto possout = input.get<Matrix>(Variables::InputMatrix);

  AlgorithmOutput output;

  //sparse support not fully implemented yet.
  if (!matrixIs::dense(matrixa) || (matrixb && !matrixIs::dense(matrixb)))
  {
    //TODO implement something with sparse
    error("BooleanCompare: Currently only works with dense matrices");
    output[Variables::OutputMatrix] = 0;
    output[Variables::Solution] = 0;
    return output;
  }

  auto mata  = castMatrix::toDense (matrixa);
  auto matb  = castMatrix::toDense (matrixb);

  MatrixHandle out_matrix;
  DenseMatrixHandle cond_matrix;
  cond_matrix.reset(new DenseMatrix(1,1,0.0));

  int cond_state = 0;

  //pull parameter from UI
  std::string valoptA = getOption(Parameters::Value_Option_1);
  std::string valoptB = getOption(Parameters::Value_Option_2);
  std::string cond_statement = getOption(Parameters::Comparison_Option);
  std::string then_result = getOption(Parameters::Then_Option);
  std::string else_result = getOption(Parameters::Else_Option);

  LOG_DEBUG("valoptA = {}", valoptA);
  LOG_DEBUG("valoptB = {}", valoptB);
  LOG_DEBUG("cond_statement = {}",cond_statement);
  LOG_DEBUG("then_result = {}" , then_result);
  LOG_DEBUG("else_result = {}" , else_result);

  if (!matb || cond_statement == "boolop")
  {
//    if no second input, only run boolean on first input
    if (!runImpl(mata, valoptA, cond_statement, cond_state))
      THROW_ALGORITHM_PROCESSING_ERROR("Error running conditional matrix algorithm. Probable bad combination of parameters");
  }
  else
  {
    if (!runImpl(mata, matb, valoptA, valoptB, cond_statement, cond_state))
      THROW_ALGORITHM_PROCESSING_ERROR("Error running conditional matrix algorithm. Probable bad combination of parameters");
  }

  //  after testing the statement, determine the module output
  return_check(cond_state,cond_matrix,out_matrix,then_result,else_result,matrixa,matrixb,possout);

  output[Variables::OutputMatrix] = out_matrix;
  output[Variables::Solution] = cond_matrix;
  return output;
}

bool BooleanCompareAlgo::runImpl(DenseMatrixHandle mata, std::string valoptA, std::string& cond_statement, int& cond_state) const
{
//  if only need to check one matrix, run this version

  size_t nrA = mata->nrows();
  size_t ncA = mata->ncols();

  if (cond_statement!="boolop")
  {
    warning("no second input, only checking for non-zero comparison of A");
    cond_statement="boolop";
  }

  DenseMatrixHandle compa;
  if (valoptA == "size")
  {
    compa.reset(new DenseMatrix(1,1,0.0));
    double *data = compa->data();
    data[0] = static_cast<double>(nrA);
    data[1] = static_cast<double>(ncA);
  }
  else if (valoptA == "norm")
  {
    compa.reset(new DenseMatrix(1,1,0.0));
    double *data = compa->data();
    data[0] = ComputeNorm(mata);
  }
  else if (valoptA == "value")
  {
    compa=mata;
  }
  else THROW_ALGORITHM_PROCESSING_ERROR("Choosen options do not make sense.");

  if (!CompareMatrix(compa,cond_state)) return false;

  return true;
}




bool BooleanCompareAlgo::runImpl(DenseMatrixHandle mata, DenseMatrixHandle matb, std::string valoptA, std::string valoptB, std::string& cond_statement, int& cond_state) const
{
// this is for two marix imputs. the comparison is trickier


  size_t nrA = mata->nrows();
  size_t ncA = mata->ncols();
  size_t nrB = matb->nrows();
  size_t ncB = matb->ncols();

  DenseMatrixHandle compa, compb;

//  check the various quantities to check make sure the checks make since.
  if ((valoptA == "size" && valoptB == "norm") || (valoptB == "size" && valoptA == "norm"))
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Cannot compare size of one matrix to norm of the other");
  }
  else if (valoptA == "size" && valoptB == "size")
  {
    compa.reset(new DenseMatrix(2,1,0.0));
    compb.reset(new DenseMatrix(2,1,0.0));

    double *data = compa->data();
    double *datb = compb->data();

    data[0] = static_cast<double>(nrA);
    data[1] = static_cast<double>(ncA);

    datb[0] = static_cast<double>(nrB);
    datb[1] = static_cast<double>(ncB);
  }
  else if (valoptA == "size" && valoptB == "value")
  {
    if (nrB*ncB != 2)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
    }
    compa.reset(new DenseMatrix(nrB,ncB,0.0));
    compb=matb;

    double *data = compa->data();
    data[0] = static_cast<double>(nrA);
    data[1] = static_cast<double>(ncA);
  }
  else if (valoptB == "size" && valoptA == "value")
  {
    if (nrA*ncA != 2)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
    }
    compa=mata;
    compb.reset(new DenseMatrix(nrA,ncA,0.0));

    double *datb = compb->data();
    datb[0] = static_cast<double>(nrB);
    datb[1] = static_cast<double>(ncB);

  }
  else if (valoptA == "norm" && valoptB == "value")
  {
    if (nrB*ncB != 1)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare norm of one matrix to matrix with 1 elements");
    }
    compa.reset(new DenseMatrix(1,1,0.0));
    compb=matb;

    double *data = compa->data();
    data[0] = ComputeNorm(mata);
  }
  else if (valoptB == "norm" && valoptA == "value")
  {
    if (nrA*ncA != 1)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of norm matrix to matrix with 1 elements");
    }

    compa=mata;
    compb.reset(new DenseMatrix(1,1,0.0));

    double *datb = compb->data();
    datb[0] = ComputeNorm(matb);
  }
  else if (valoptA == "norm" && valoptB == "norm")
  {
    compa.reset(new DenseMatrix(1,1,0.0));
    compb.reset(new DenseMatrix(1,1,0.0));

    double *data = compa->data();
    double *datb = compb->data();

    data[0] = ComputeNorm(mata);
    datb[0] = ComputeNorm(matb);
  }
  else if (valoptA == "value" && valoptB == "value")
  {
    if ((nrA!=nrB) || (ncA!=ncB))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Matrix must be of the same size when comparing values");
    }
    compa=mata;
    compb=matb;
  }
  else return false;
  if (!CompareMatrix(compa, compb, cond_statement,cond_state)) return false;

  return true;

}


double BooleanCompareAlgo::ComputeNorm(DenseMatrixHandle mat) const
{
//  calculate the second norm for a magnitude comparison.
  size_type n = mat->nrows();
  size_type m = mat->ncols();
  double *data = mat->data();

  double sumation = 0;

  for (index_type k=0;  k<(m*n);k++)
  {
    sumation+=data[k]*data[k];
  }
  double norm = sqrt(sumation);
  return norm;
}

bool BooleanCompareAlgo::CompareMatrix(DenseMatrixHandle mata, int& cond_state) const
{
  //   check for non-zero entries of single matrix

  size_type n = mata->nrows();
  size_type m = mata->ncols();
  double *data = mata->data();

  for (index_type k=0; k<(m*n);k++)
  {
    if (data[k]!=0)
    {
      cond_state = 1;
      return true;
    }
  }
  cond_state = 0;
  return true;
}


bool BooleanCompareAlgo::CompareMatrix(DenseMatrixHandle mata, DenseMatrixHandle matb,std::string cond_statement, int& cond_state) const
{
//  compare values of two matrices
  size_type na = mata->nrows();
  size_type ma = mata->ncols();
  size_type nb = matb->nrows();
  size_type mb = matb->ncols();

  double *data = mata->data();
  double *datb = matb->data();

  LOG_DEBUG("mata = {}", *mata);
  LOG_DEBUG("matb = {}", *matb);

  if ((na!=nb) || (ma!=mb)) THROW_ALGORITHM_PROCESSING_ERROR("Matrices must be the same size");

  if (cond_statement == "eqop")
  {
    //   check for non-equal values entries of matrix
    for (index_type k=0; k<(ma*na);k++)
    {
      if (data[k]!=datb[k])
      {
        cond_state=0;
        return true;
      }
    }
    cond_state=1;
    return true;
  }
  else if (cond_statement == "andop")
  {
    //   check for both for non-zero values entries of matrix
    for (index_type k=0; k<(ma*na);k++)
    {
      if (data[k]!=0 && datb[k]!=0)
      {
        cond_state=1;
        return true;
      }
    }
    cond_state=0;
    return true;
  }
  else if (cond_statement == "orop")
  {
    //   check for both for non-zero values entries of matrix
    for (index_type k=0; k<(ma*na);k++)
    {
      if (data[k]!=0 || datb[k]!=0)
      {
        cond_state=1;
        return true;
      }
    }
    cond_state=0;
    return true;
  }
  else if (cond_statement == "greatop" || cond_statement == "greateqop" || cond_statement == "lesseqop"|| cond_statement == "lessop")
  {
    //  These cases are tricky to compare with multiple entries, because some entries could
    //  greater, less, or equal to the the corresponding etries.


    int great=0;
    int less=0;
    int eq=0;
    double sumation=0;

    for (index_type k=0; k<(ma*na);k++)
    {
      if (data[k]<datb[k]) less+=1;
      if (data[k]==datb[k]) eq+=1;
      if (data[k]>datb[k]) great+=1;
      sumation += data[k]-datb[k];
    }

    LOG_DEBUG("greater = {}", great);
    LOG_DEBUG("less = {}", less);
    LOG_DEBUG("equal = {}", eq);
    LOG_DEBUG("sumation = {}", sumation);

//    if there are both greater and less than, need some extra checking. compare the number of
//    entries that fulfill the condition, and the magnitude of the difference.

    if (great>0 && less>0)
    {
      warning("Matrices have entries that are both greater and less than the other.  Attempting to compare based on number of elements and magnitude of the difference.");
      if (cond_statement == "greatop")
      {
        if (great>less && sumation>0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "greateqop")
      {
        if (great>=less && sumation>=0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "lesseqop")
      {
        if (great<=less && sumation<=0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "lessop")
      {
        if (great<less && sumation<0) cond_state=1;
        else cond_state=0;
      }
      else return false;
    }
    else if (great+less+eq ==0)
    {
      return false;
    }
    else
    {
      if (cond_statement == "greatop")
      {
        if (great>0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "greateqop")
      {
        if (less == 0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "lesseqop")
      {
        if (great == 0) cond_state=1;
        else cond_state=0;
      }
      else if (cond_statement == "lessop")
      {
        if (less>0) cond_state=1;
        else cond_state=0;
      }
      else return false;

    }
  }
  else return false;

  return true;
}


// determine the output
bool BooleanCompareAlgo::return_check(int& cond_state, DenseMatrixHandle& cond_matrix, MatrixHandle& out_matrix, std::string then_result, std::string else_result, MatrixHandle matrixa, MatrixHandle matrixb, MatrixHandle possout) const
{
  double *data = cond_matrix->data();

  if (cond_state==1)
  {
    data[0] = 1;
    if (!return_value(out_matrix,then_result,matrixa,matrixb,possout)) return false;
  }
  else if (cond_state==0)
  {
    data[0] = 0;
    if (!return_value(out_matrix,else_result,matrixa,matrixb,possout)) return false;
  }
  else return false;
  return true;
}


bool BooleanCompareAlgo::return_value(MatrixHandle& out_matrix,std::string result_statement, MatrixHandle first, MatrixHandle second, MatrixHandle possout) const
{
  if (result_statement == "second" && !second)
  {
    result_statement = "null";
    warning("no input for second matrix, cannot use it as an output.");
  }
  if (result_statement == "third" && !possout)
  {
    result_statement = "null";
    warning("no input for third matrix, cannot use it as an output.");
  }

  if (result_statement == "first") out_matrix = first;
  else if (result_statement == "second") out_matrix = second;
  else if (result_statement == "third") out_matrix = possout;
  else if (result_statement == "null" || result_statement == "quit") out_matrix = 0;
  else return false;

  return true;
}
