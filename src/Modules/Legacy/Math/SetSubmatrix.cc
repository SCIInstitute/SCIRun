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


///
/// @class SetSubmatrix
/// @brief Clip out a subregion from a Matrix
///
/// @author
///    Michael Callahan
///    Department of Computer Science
///    University of Utah
/// @date  January 2002
///

#include <Modules/Legacy/Math/SetSubmatrix.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Math, StartColumn);
ALGORITHM_PARAMETER_DEF(Math, StartRow);
ALGORITHM_PARAMETER_DEF(Math, MatrixDims);
ALGORITHM_PARAMETER_DEF(Math, SubmatrixDims);

MODULE_INFO_DEF(SetSubmatrix, Math, SCIRun)

SetSubmatrix::SetSubmatrix() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(Input_Submatrix);
  INITIALIZE_PORT(Optional_Start_Bounds);
  INITIALIZE_PORT(OutputMatrix);
}

void SetSubmatrix::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::MatrixDims, std::string("MxN"));
  state->setValue(Parameters::SubmatrixDims, std::string("MxN"));
  state->setValue(Parameters::StartColumn, 0);
  state->setValue(Parameters::StartRow, 0);
}

void
SetSubmatrix::execute()
{
  auto imatrix = getRequiredInput(InputMatrix);
  auto smatrix = getRequiredInput(Input_Submatrix);

  if (needToExecute())
  {
    auto state = get_state();
    state->setValue(Parameters::MatrixDims, boost::lexical_cast<std::string>(imatrix->nrows()) + "x" + boost::lexical_cast<std::string>(imatrix->ncols()));
    state->setValue(Parameters::SubmatrixDims, boost::lexical_cast<std::string>(smatrix->nrows()) + "x" + boost::lexical_cast<std::string>(smatrix->ncols()));

  auto cmatrixOpt = getOptionalInput(Optional_Start_Bounds);

  if (cmatrixOpt && *cmatrixOpt)
  {
    auto cmatrix = *cmatrixOpt;
    if( cmatrix->nrows() == 2 && cmatrix->ncols() == 1 )
    {
      state->setValue(Parameters::StartRow, (int) (size_type)cmatrix->get(0, 0));
      state->setValue(Parameters::StartColumn, (int) (size_type)cmatrix->get(1, 0));
    }
    else if( cmatrix->nrows() == 1 && cmatrix->ncols() == 2 )
    {
      state->setValue(Parameters::StartRow, (int) (size_type)cmatrix->get(0, 0));
      state->setValue(Parameters::StartColumn, (int) (size_type)cmatrix->get(0, 1));
    }
    else
    {
      error( "Input start matrix is not a 2x1 or 1x2 matrix" );
      return;
    }
  }

  const auto startRow = state->getValue(Parameters::StartRow).toInt();
  const auto startCol = state->getValue(Parameters::StartColumn).toInt();

  if (startRow + smatrix->nrows() > imatrix->nrows() ||
      startCol + smatrix->ncols() > imatrix->ncols() )
  {
    error("Start plus submatrix range must be less than or equal to max range.");
    return;
  }

    MatrixHandle omatrix(imatrix->clone());

    for( int row = 0; row < smatrix->nrows(); ++row)
    {
      for( int col = 0; col < smatrix->ncols(); ++col)
      {
	      omatrix->put( startRow+row, startCol+col, smatrix->get(row, col) );
      }
    }

    sendOutput(OutputMatrix, omatrix);
  }
}
