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


#include <Core/Algorithms/Math/RootMeanSquareError/RootMeanSquareError.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/GuiInterface/GuiVar.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRun {

/// @class ReportRootMeanSquareError
/// @brief This module computes the Root Mean Square Error from two input
/// matrices and stores the result in the output matrix.

class ReportRootMeanSquareError : public Module
{
  GuiInt     guiinput_is_vector_;
  GuiInt     guimatrix_order_;
  GuiDouble  guirmse_result_;

  SCIRunAlgo::RootMeanSquareErrorAlgo algo_;

  enum MATRIX_ORDER
  {
    ROW_ORDER = 0,
    COLUMN_ORDER
  };

  void update_gui(bool input_is_vector);

public:
  ReportRootMeanSquareError(GuiContext* ctx);
  virtual ~ReportRootMeanSquareError() {}
  virtual void execute();
};

DECLARE_MAKER(ReportRootMeanSquareError)

ReportRootMeanSquareError::ReportRootMeanSquareError(GuiContext* ctx)
: Module("ReportRootMeanSquareError", ctx, Source, "Math", "SCIRun"),
  guiinput_is_vector_(get_ctx()->subVar("input_is_vector", false), 1),
  guimatrix_order_(get_ctx()->subVar("matrix_order", false), ROW_ORDER),
  guirmse_result_(get_ctx()->subVar("rmse", false), 0)
{
  algo_.set_progress_reporter(this);
}

void
ReportRootMeanSquareError::execute()
{
  MatrixHandle inputMatrix, estimateMatrix;
  if (! get_input_handle("Input", inputMatrix) )
    return;

  if (! get_input_handle("Estimate", estimateMatrix) )
    return;

  if ( inputs_changed_ || guimatrix_order_.changed() || !oport_cached("Output") )
  {
    if (! (inputMatrix->nrows() == estimateMatrix->nrows() && inputMatrix->ncols() == estimateMatrix->ncols()) )
    {
      error("Input matrix dimensions must match.");
      return;
    }

    // assumes 'vector' is a column matrix
    bool is_vector = inputMatrix->ncols() == 1 || matrix_is::column(inputMatrix);
    guiinput_is_vector_.set(is_vector);

    // force GUI update in case UI window is already open (a bit messy)
    update_gui(is_vector);

    if (is_vector)
    {
      algo_.set_option("mode", "vector");
    }
    else
    {
      if (guimatrix_order_.get() == ROW_ORDER)
      {
        algo_.set_option("mode", "roworder");
      }
      else // COLUMN_ORDER
      {
        algo_.set_option("mode", "columnorder");
      }
    }

    MatrixHandle outputMatrix;
    if (! algo_.run(inputMatrix, estimateMatrix, outputMatrix) )
    {
      return;
    }

    guirmse_result_.set(outputMatrix->get(0, 0));
    send_output_handle("RMSE", outputMatrix);
  }
}

void ReportRootMeanSquareError::update_gui(bool input_is_vector)
{
  std::ostringstream oss;
  oss << get_id();
  if (input_is_vector)
  {
    oss << " matrix_order_widgets_disabled";
  }
  else
  {
    oss << " matrix_order_widgets_enabled";
  }

  oss << "; update idletasks";
  TCLInterface::execute(oss.str());
}

}
