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
///    David Weinstein,
///    Department of Computer Science,
///    University of Utah
/// @date  June 1999

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace SCIRun {

/// @class GetColumnOrRowFromMatrix
/// @brief Select a row or column of a matrix
///
/// This module allows users to select a singe row or column from a matrix.
/// The user can also select a range of rows or columns and send them to the
/// output port one at a time.

class GetColumnOrRowFromMatrix : public Module {
  GuiString row_or_col_;
  GuiDouble selectable_min_;
  GuiDouble selectable_max_;
  GuiInt    selectable_inc_;
  GuiString selectable_units_;
  GuiInt    range_min_;
  GuiInt    range_max_;
  GuiString playmode_;
  GuiString dependence_;
  GuiInt    current_;
  GuiString execmode_;
  GuiInt    delay_;
  GuiInt    inc_amount_;
  GuiInt    send_amount_;
  GuiInt    data_series_done_;
  int       inc_;
  bool      loop_;
  bool       use_row_;
  int       last_gen_;

  void send_selection(MatrixHandle mh, int which, int ncopy, bool cache);
  int increment(int which, int lower, int upper);

public:
  GetColumnOrRowFromMatrix(GuiContext* ctx);
  virtual ~GetColumnOrRowFromMatrix();
  virtual void execute();
  virtual void tcl_command(GuiArgs&, void*);
};


DECLARE_MAKER(GetColumnOrRowFromMatrix)

GetColumnOrRowFromMatrix::GetColumnOrRowFromMatrix(GuiContext* ctx)
  : Module("GetColumnOrRowFromMatrix", ctx, Filter,"Math", "SCIRun"),
    row_or_col_(get_ctx()->subVar("row_or_col")),
    selectable_min_(get_ctx()->subVar("selectable_min")),
    selectable_max_(get_ctx()->subVar("selectable_max")),
    selectable_inc_(get_ctx()->subVar("selectable_inc")),
    selectable_units_(get_ctx()->subVar("selectable_units")),
    range_min_(get_ctx()->subVar("range_min")),
    range_max_(get_ctx()->subVar("range_max")),
    playmode_(get_ctx()->subVar("playmode")),
    dependence_(get_ctx()->subVar("dependence")),
    current_(get_ctx()->subVar("current")),
    execmode_(get_ctx()->subVar("execmode")),
    delay_(get_ctx()->subVar("delay")),
    inc_amount_(get_ctx()->subVar("inc-amount")),
    send_amount_(get_ctx()->subVar("send-amount")),
    data_series_done_(get_ctx()->subVar("data_series_done")),
    inc_(1),
    loop_(false),
    use_row_(false),
    last_gen_(-1)
{
}


GetColumnOrRowFromMatrix::~GetColumnOrRowFromMatrix()
{
}


void
GetColumnOrRowFromMatrix::send_selection(MatrixHandle mh, int which,
				   int ncopy, bool /*cache*/)
{
  MatrixHandle matrix(0);
  if (use_row_)
  {
    if (ncopy == 1)
    {
      ColumnMatrix *cm = new ColumnMatrix(mh->ncols());
      double *data = cm->get_data_pointer();
      for (int c = 0; c<mh->ncols(); c++)
      {
        data[c] = mh->get(which, c);
      }
      matrix = cm;
    }
    else
    {
      DenseMatrix *dm = new DenseMatrix(ncopy, mh->ncols());
      for (int i = 0; i < ncopy; i++)
        for (int c = 0; c < mh->ncols(); c++)
          dm->put(i, c, mh->get(which + i, c));

      matrix = dm;
    }
  }
  else
  {
    if (ncopy == 1)
    {
      ColumnMatrix *cm = new ColumnMatrix(mh->nrows());
      double *data = cm->get_data_pointer();
      for (int r = 0; r<mh->nrows(); r++)
        data[r] = mh->get(r, which);
      matrix = cm;
    }
    else
    {
      DenseMatrix *dm = new DenseMatrix(mh->nrows(), ncopy);
      for (int r = 0; r < mh->nrows(); r++)
        for (int i = 0; i < ncopy; i++)
            dm->put(r, i, mh->get(r, which + i));

      matrix = dm;
    }
  }

  //ovec->set_cache( cache );
  send_output_handle("Vector", matrix);

  ColumnMatrix *selected = new ColumnMatrix(1);
  selected->put(0, 0, (double)which);

  MatrixHandle stmp(selected);
  send_output_handle("Selected Index", stmp);
}


int
GetColumnOrRowFromMatrix::increment(int which, int lower, int upper)
{
  data_series_done_.reset();
  if (playmode_.get() == "autoplay" && data_series_done_.get()) {
    return which;
  }
  // Do nothing if no range.
  if (upper == lower) {
    if (playmode_.get() == "once")
      execmode_.set( "stop" );
    return upper;
  }
  const int inc_amount = Max(1, Min(upper, inc_amount_.get()));

  which += inc_ * inc_amount;

  if (which > upper) {
    if (playmode_.get() == "bounce1") {
      inc_ *= -1;
      return increment(upper, lower, upper);
    } else if (playmode_.get() == "bounce2") {
      inc_ *= -1;
      return upper;
    } else if (playmode_.get() == "autoplay") {
      data_series_done_.set(1);
      return lower;
    } else {
      if (playmode_.get() == "once")
	execmode_.set( "stop" );
      return lower;
    }
  }
  if (which < lower) {
    if (playmode_.get() == "bounce1") {
      inc_ *= -1;
      return increment(lower, lower, upper);
    } else if (playmode_.get() == "bounce2") {
      inc_ *= -1;
      return lower;
    } else {
      if (playmode_.get() == "once")
	execmode_.set( "stop" );
      return upper;
    }
  }
  return which;
}



void
GetColumnOrRowFromMatrix::execute()
{
  update_state(NeedData);

  MatrixHandle mh;
  if (!get_input_handle("Matrix", mh)) return;

  update_state(JustStarted);
  if (playmode_.get() == "autoplay") {
    data_series_done_.reset();
    while (last_gen_ == mh->generation && data_series_done_.get())
    {
      //cerr << "waiting" << std::endl;
      //want_to_execute();
      return;
    }
    last_gen_ = mh->generation;
    data_series_done_.set(0);
  }


  bool changed_p = false;

  bool use_row = (row_or_col_.get() == "row");

  if( use_row_ != use_row ) {
    use_row_ = use_row;
    changed_p = true;
  }

  if (use_row_){
    std::string units;
    if (units != selectable_units_.get()) {
      selectable_units_.set(units);
      changed_p = true;
    }

    double minlabel = 0.0;
    if (!mh->get_property("row_min", minlabel))
      minlabel = 0.0;

    if (minlabel != selectable_min_.get()) {
      selectable_min_.set(minlabel);
      changed_p = true;
    }

    double maxlabel = 0.0;
    if (!mh->get_property("row_max", maxlabel))
      maxlabel = mh->nrows() - 1.0;

    if (maxlabel != selectable_max_.get()) {
      selectable_max_.set(maxlabel);
      changed_p = true;
    }

    int increments = mh->nrows();
    if (increments != selectable_inc_.get()) {
      selectable_inc_.set(increments);
      changed_p = true;
    }
  } else {
    std::string units;
    if (units != selectable_units_.get()) {
      selectable_units_.set(units);
      changed_p = true;
    }

    double minlabel = 0.0;
    if (!mh->get_property("col_min", minlabel))
      minlabel = 0.0;

    if (minlabel != selectable_min_.get()) {
      selectable_min_.set(minlabel);
      changed_p = true;
    }

    double maxlabel = 0.0;
    if (!mh->get_property("col_max", maxlabel))
      maxlabel = mh->ncols() - 1.0;

    if (maxlabel != selectable_max_.get()) {
      selectable_max_.set(maxlabel);
      changed_p = true;
    }

    int increments = mh->ncols();
    if (increments != selectable_inc_.get()) {
      selectable_inc_.set(increments);
      changed_p = true;
    }
  }

  if (changed_p)
    TCLInterface::execute(get_id() + " update_range");

  reset_vars();

  int which;

  // Specialized matrix multiply, with Weight Vector given as a sparse
  // matrix.  It's not clear what this has to do with GetColumnOrRowFromMatrix.
  MatrixHandle weightsH;
  if (get_input_handle("Weight Vector", weightsH, false))
  {
    // Some how some one removed the sparse matrix version
    // We change back to the system of using a sparse matrix

    // Convert matrix to sparse matrix
    SparseRowMatrixHandle spH = weightsH->sparse();
    SparseRowMatrix* w = spH.get_rep();

    if (w == 0)  {
      error("Could not obtain weight matrix");
      return;
    }

    DenseMatrix *dm;

    if (use_row_)
    {
      int nnrows = w->nrows();
      int nncols = mh->ncols();

      dm = new DenseMatrix(nnrows,nncols);
      if (dm == 0)
      {
        error("Could not obtain enough memory for output matrix");
        return;
      }

      dm->zero();

      if (!w->is_valid())
      {
        error("Encountered an invalid sparse matrix");
        return;
      }

      index_type *rr = w->get_rows();
      index_type *cc = w->get_cols();
      double *a =  w->get_vals();
      double *data = dm->get_data_pointer();
      for (int p = 0; p < nnrows; p++)
      {
        for (int r = rr[p]; r<rr[p+1]; r++)
        {
          for (int q = 0;  q< nncols; q++)
          {
            data[q+p*nncols]+=mh->get(cc[r],q)*a[r];
          }
        }
      }
    }
    else
    {

      int nncols = w->nrows();
      int nnrows = mh->nrows();

      dm = new DenseMatrix(nnrows,nncols);
      if (dm == 0)
      {
        error("Could not obtain enough memory for output matrix");
        return;
      }

      dm->zero();

      if (!w->is_valid())
      {
        error("Encountered an invalid sparse matrix");
        return;
      }

      index_type *rr = w->get_rows();
      index_type *cc = w->get_cols();
      double *a =  w->get_vals();
      double *data = dm->get_data_pointer();
      for (int p = 0; p < nncols; p++)
      {
        for (int r = rr[p]; r<rr[p+1]; r++)
        {
          for (int q = 0;  q< nnrows; q++)
          {
            data[p+q*nncols]+=mh->get(q,cc[r])*a[r];
          }
        }
      }
    }

    MatrixHandle cmtmp(dm);
    send_output_handle("Vector", cmtmp);
    return;
  }

  const int maxsize = (use_row_?mh->nrows():mh->ncols())-1;
  const int send_amount = Max(1, Min(maxsize, send_amount_.get()));

  // If there is a current index matrix, use it.
  MatrixHandle currentH;
  if (get_input_handle("Current Index", currentH, false))
  {
    which = (int)(currentH->get(0, 0));
    send_selection(mh, which, send_amount, true);
  }
  else
  {
    // Cache var
    bool cache = (playmode_.get() != "inc_w_exec");

    // Get the current start and end.
    const int start = range_min_.get();
    const int end = range_max_.get();

    int lower = start;
    int upper = end;
    if (lower > upper) {int tmp = lower; lower = upper; upper = tmp; }


    // Update the increment.
    if (changed_p || playmode_.get() == "once" ||
	playmode_.get() == "autoplay" || playmode_.get() == "loop") {
      inc_ = (start>end)?-1:1;
    }

    // If the current value is invalid, reset it to the start.
    if (current_.get() < lower || current_.get() > upper) {
      current_.set(start);
      inc_ = (start>end)?-1:1;
    }

    // Cash execmode and reset it in case we bail out early.
    const std::string execmode = execmode_.get();

    which = current_.get();

    // If updating, we're done for now.
    if (execmode == "update") {

    } else if (execmode == "step") {
      which = increment(current_.get(), lower, upper);
      send_selection(mh, which, send_amount, cache);

    } else if (execmode == "stepb") {
      inc_ *= -1;
      which = increment(current_.get(), lower, upper);
      inc_ *= -1;
      send_selection(mh, which, send_amount, cache);

    } else if (execmode == "play") {
      if( !loop_ ) {
	if (playmode_.get() == "once" && which >= end)
	  which = start;
	if (playmode_.get() == "autoplay" && which >= end)
	{
	  which = start;
	  std::cerr << "setting to wait" << std::endl;
	  data_series_done_.set(1);
	}
    }

      send_selection(mh, which, send_amount, cache);

      // User may have changed the execmode to stop so recheck.
      execmode_.reset();
      if ( loop_ = (execmode_.get() == "play") ) {
	const int delay = delay_.get();

	if( delay > 0) {
#ifndef _WIN32
	  const unsigned int secs = delay / 1000;
	  const unsigned int msecs = delay % 1000;
	  if (secs)  { sleep(secs); }
	  if (msecs) { usleep(msecs * 1000); }
#else
	  Sleep(delay);
#endif
	}

	int next = increment(which, lower, upper);

	// Incrementing may cause a stop in the execmode so recheck.
	execmode_.reset();
	if(loop_ = (execmode_.get() == "play")) {
	  which = next;
	  want_to_execute();
	}
      }
    } else {
      if( execmode == "rewind" )
	which = start;

      else if( execmode == "fforward" )
	which = end;

      send_selection(mh, which, send_amount, cache);

      if (playmode_.get() == "inc_w_exec") {
	which = increment(which, lower, upper);
      }
    }
  }
  current_.set(which);
}


void
GetColumnOrRowFromMatrix::tcl_command(GuiArgs& args, void* userdata)
{
  if (args.count() < 2) {
    args.error("GetColumnOrRowFromMatrix needs a minor command");
    return;

  }

  if (args[1] == "restart") {

  } else Module::tcl_command(args, userdata);
}


} // End namespace SCIRun
