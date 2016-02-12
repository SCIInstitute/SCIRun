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


///
///@class SetSubmatrix
///@brief Clip out a subregion from a Matrix
///
///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  January 2002
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

#if 0
namespace SCIRun {

class SetSubmatrix : public Module
{

private:
  GuiInt startcol_;
  GuiInt startrow_;
  GuiInt nrow_;
  GuiInt ncol_;
  GuiInt srow_;
  GuiInt scol_;

public:
  SetSubmatrix(GuiContext* ctx);
  virtual ~SetSubmatrix();

  virtual void execute();
};
#endif

ALGORITHM_PARAMETER_DEF(Math, StartColumn);
ALGORITHM_PARAMETER_DEF(Math, StartRow);
ALGORITHM_PARAMETER_DEF(Math, MatrixDims);
ALGORITHM_PARAMETER_DEF(Math, SubmatrixDims);

const ModuleLookupInfo SetSubmatrix::staticInfo_("SetSubmatrix", "Math", "SCIRun");

SetSubmatrix::SetSubmatrix()
  : Module(staticInfo_)
  // ,
  //   startcol_(get_ctx()->subVar("startcol"), 0),
  //   startrow_(get_ctx()->subVar("startrow"), 0),
  //   nrow_(get_ctx()->subVar("nrow"), 0),
  //   ncol_(get_ctx()->subVar("ncol"), 0),
  //   srow_(get_ctx()->subVar("srow"), 0),
  //   scol_(get_ctx()->subVar("scol"), 0)
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
  #if 0
  MatrixHandle imatrix;
  if (!get_input_handle("Input Matrix", imatrix))
    return;

  MatrixHandle smatrix;
  if (!get_input_handle("Input Submatrix", smatrix))
    return;

  if( inputs_changed_ )
  {
    nrow_.set(imatrix->nrows());
    ncol_.set(imatrix->ncols());

    srow_.set(smatrix->nrows());
    scol_.set(smatrix->ncols());

    nrow_.reset();
    ncol_.reset();

    srow_.reset();
    scol_.reset();
  }

  MatrixHandle cmatrix;

  if (get_input_handle("Optional Start Bounds", cmatrix, false))
  {
    if( cmatrix->nrows() == 2 && cmatrix->ncols() == 1 )
    {
      startrow_.set( (int) (size_type)cmatrix->get(0, 0) );
      startcol_.set( (int) (size_type)cmatrix->get(1, 0) );

      startrow_.reset();
      startcol_.reset();
    }
    else if( cmatrix->nrows() == 1 && cmatrix->ncols() == 2 )
    {
      startrow_.set( (int) (size_type)cmatrix->get(0, 0) );
      startcol_.set( (int) (size_type)cmatrix->get(0, 1) );

      startrow_.reset();
      startcol_.reset();
    }
    else
    {
      error( "Input start matrix is not a 2x1 or 1x2 matrix" );
      return;
    }
  }

  if (startrow_.get() + srow_.get() > nrow_.get() ||
      startcol_.get() + scol_.get() > ncol_.get() )
  {
    error("Start plus submatrix range must be less than or equal to max range.");
    return;
  }

  if( inputs_changed_ ||

      !oport_cached("Output Matrix") ||

      startrow_.changed( true ) ||
      startcol_.changed( true ) )
  {

    MatrixHandle omatrix = imatrix->clone();

    for( int row=0; row<smatrix->nrows(); ++row)
    {
      for( int col=0; col<smatrix->ncols(); ++col)
      {
	omatrix->put( startrow_.get()+row, startcol_.get()+col,
		      smatrix->get(row, col) );
      }
    }

    send_output_handle("Output Matrix", omatrix);
  }
  #endif
}
