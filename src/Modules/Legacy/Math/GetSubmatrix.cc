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


/// @author
///    Michael Callahan,
///    Department of Computer Science,
///    University of Utah
/// @date  January 2002

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Util/StringUtil.h>

namespace SCIRun {

///@class GetSubmatrix
///@brief This module selects a subset of a matrix.

class GetSubmatrix : public Module
{
private:
  GuiString mincol_;
  GuiString maxcol_;
  GuiString minrow_;
  GuiString maxrow_;
  GuiString nrow_;
  GuiString ncol_;

public:
  GetSubmatrix(GuiContext* ctx);
  virtual ~GetSubmatrix();

  virtual void execute();
};


DECLARE_MAKER(GetSubmatrix)

GetSubmatrix::GetSubmatrix(GuiContext* ctx)
  : Module("GetSubmatrix", ctx, Filter,"Math", "SCIRun"),
    mincol_(get_ctx()->subVar("mincol"), "---"),
    maxcol_(get_ctx()->subVar("maxcol"), "---"),
    minrow_(get_ctx()->subVar("minrow"), "---"),
    maxrow_(get_ctx()->subVar("maxrow"), "---"),
    nrow_(get_ctx()->subVar("nrow"), "??"),
    ncol_(get_ctx()->subVar("ncol"), "??")
{
}


GetSubmatrix::~GetSubmatrix()
{
}


void
GetSubmatrix::execute()
{
  MatrixHandle imatrix;
  if (!get_input_handle("Input Matrix", imatrix))

  nrow_.set(to_string(imatrix->nrows()));
  ncol_.set(to_string(imatrix->ncols()));
  get_ctx()->reset();

  MatrixHandle cmatrix;
  size_type mincol, maxcol, minrow, maxrow;
  if (get_input_handle("Optional Range Bounds", cmatrix, false))
  {
    // Grab the bounds from the clip matrix, check them, and update the gui.
    if (cmatrix->nrows() > 1)
    {
      minrow = (size_type)cmatrix->get(0, 0);
      maxrow = (size_type)cmatrix->get(1, 0);

      if (cmatrix->ncols() > 1)
      {
        mincol = (size_type)cmatrix->get(0, 1);
        maxcol = (size_type)cmatrix->get(1, 1);
      }
      else
      {
        mincol = 0;
        maxcol = imatrix->ncols() - 1;
      }
    }
    else
    {
      minrow = 0;
      maxrow = imatrix->ncols() - 1;
      mincol = 0;
      maxcol = imatrix->ncols() - 1;
    }
  }
  else
  {
    if (!from_string(minrow_.get(), minrow)) minrow = 0;
    if (!from_string(maxrow_.get(), maxrow)) maxrow = imatrix->nrows()-1;
    if (!from_string(mincol_.get(), mincol)) mincol = 0;
    if (!from_string(maxcol_.get(), maxcol)) maxcol = imatrix->ncols()-1;
  }

  minrow = Min(Max(static_cast<size_type>(0), minrow), imatrix->nrows()-1);
  maxrow = Min(Max(static_cast<size_type>(0), maxrow), imatrix->nrows()-1);
  mincol = Min(Max(static_cast<size_type>(0), mincol), imatrix->ncols()-1);
  maxcol = Min(Max(static_cast<size_type>(0), maxcol), imatrix->ncols()-1);

  minrow_.set(to_string(minrow));
  maxrow_.set(to_string(maxrow));
  mincol_.set(to_string(mincol));
  maxcol_.set(to_string(maxcol));

  if (mincol > maxcol || minrow > maxrow)
  {
    warning("Max range must be greater than or equal to min range, disregarding.");
    return;
  }

  // No need to clip if the matrices are identical.
  if (minrow == 0 && maxrow == (imatrix->nrows()-1) &&
      mincol == 0 && maxcol == (imatrix->ncols()-1))
  {
    send_output_handle("Output Matrix", imatrix);
  }
  else
  {
    MatrixHandle omatrix(imatrix->submatrix(minrow, mincol, maxrow, maxcol));
    send_output_handle("Output Matrix", omatrix);
  }
}


} // End namespace SCIRun
