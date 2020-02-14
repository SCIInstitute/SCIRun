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


/// @todo Documentation Modules/Legacy/DataIO/StreamMatrixFromDisk.cc

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/DataStreaming/StreamMatrix.h>


namespace SCIRun {

class StreamMatrixFromDisk : public Module {
  public:
    StreamMatrixFromDisk(GuiContext*);
    virtual ~StreamMatrixFromDisk() {}
    virtual void execute();

  private:
    GuiString row_or_col_;
    GuiInt    slider_min_;
    GuiInt    slider_max_;
    GuiInt    range_min_;
    GuiInt    range_max_;
    GuiString playmode_;
    GuiInt    current_;
    GuiString execmode_;
    GuiInt    delay_;
    GuiInt    inc_amount_;
    GuiInt    send_amount_;
    GuiString guifilename_;
    GuiInt    autoplay_;

    int       inc_;
    bool      loop_;
    bool      use_row_;
    bool      didrun_;

    SCIRunAlgo::StreamMatrixAlgo datafile_;

    void send_selection(int which, int amount);
    int increment(int which, int lower, int upper);
};


DECLARE_MAKER(StreamMatrixFromDisk)
StreamMatrixFromDisk::StreamMatrixFromDisk(GuiContext* ctx)
  : Module("StreamMatrixFromDisk", ctx, Source, "DataIO", "SCIRun"),
    row_or_col_(get_ctx()->subVar("row_or_col"),"column"),
    slider_min_(get_ctx()->subVar("slider_min"),0),
    slider_max_(get_ctx()->subVar("slider_max"),100),
    range_min_(get_ctx()->subVar("range_min"),0),
    range_max_(get_ctx()->subVar("range_max"),100),
    playmode_(get_ctx()->subVar("playmode"),"once"),
    current_(get_ctx()->subVar("current"),0),
    execmode_(get_ctx()->subVar("execmode"),"init"),
    delay_(get_ctx()->subVar("delay"),0.0),
    inc_amount_(get_ctx()->subVar("inc-amount"),1),
    send_amount_(get_ctx()->subVar("send-amount"),1),
    guifilename_(get_ctx()->subVar("filename"),""),
    autoplay_(get_ctx()->subVar("autoplay"),0),
    inc_(1),
    loop_(false),
    use_row_(false),
    didrun_(false)
{
  datafile_.set_progress_reporter(this);
}


void
StreamMatrixFromDisk::execute()
{
  // Deal with String input port
  StringHandle FileName;
  MatrixHandle Indices;
  MatrixHandle Weights;
  std::string filename;

  if (get_input_handle("Filename",FileName,false))
  {
    guifilename_.set(FileName->get());
    get_ctx()->reset();
  }
  filename = guifilename_.get();
  if (filename == "") return;

  FileName = new String(filename);

  // Get Indices or Weights
  get_input_handle("Indices",Indices,false);
  get_input_handle("Weights",Weights,false);

  update_state(Executing);

  if (!(datafile_.open(filename)))
  {
    error("Could not open header file: " + filename);
    return;
  }

  // Determine the mode we are running
  bool use_row = (row_or_col_.get() == "row");

  // Update the sliders
  if (use_row)
  {
    slider_min_.set(0);
    slider_max_.set(datafile_.get_numrows()-1);
  }
  else
  {
    slider_min_.set(0);
    slider_max_.set(datafile_.get_numcols()-1);
  }
  TCLInterface::execute(get_id() + " update_range");
  reset_vars();

  if (autoplay_.get())
  {
    if (use_row)
    {
      range_min_.set(0);
      range_max_.set(datafile_.get_numrows()-1);
    }
    else
    {
      range_min_.set(0);
      range_max_.set(datafile_.get_numcols()-1);
    }
    execmode_.set("play");
    get_ctx()->reset();
    didrun_ = true;
  }

  bool senddata = true;

  // Check whether we are running from input or from GUI
  if ((Indices.get_rep() == 0)&&(Weights.get_rep() == 0))
  {
    get_ctx()->reset();
    int start = range_min_.get();
    int end = range_max_.get();

    int lower = start;
    int upper = end;
    if (lower > upper) {int tmp = lower; lower = upper; upper = tmp; }

    // Update the increment.
    if (playmode_.get() == "once" || playmode_.get() == "loop")
    {
      if (start>end) inc_ = -1; else inc_ = 1;
    }

    // If the current value is invalid, reset it to the start.
    if (current_.get() < lower || current_.get() > upper)
    {
      current_.set(start);
      inc_ = (start>end)?-1:1;
    }

    // Catch execmode and reset it in case we bail out early.
    std::string execmode = execmode_.get();

    int current = current_.get();
    int amount = send_amount_.get();

    // If updating, we're done for now.
    if ((didrun_ == false)||(use_row!=use_row_))
    {
      use_row_ = use_row;
      senddata = true;
      didrun_ = true;
    }
    else if (execmode == "step")
    {
      current = increment(current, lower, upper);
      senddata = true;
    }
    else if (execmode == "stepb")
    {
      inc_ *= -1;
      current = increment(current, lower, upper);
      inc_ *= -1;
      senddata = true;
    }
    else if (execmode == "play")
    {
      if( !loop_ )
      {
        if (playmode_.get() == "once" && current >= end) current = start;
      }

      senddata = true;
      // User may have changed the execmode to stop so recheck.
      execmode_.reset();
      if ( loop_ = (execmode_.get() == "play") )
      {
        const int delay = delay_.get();

        if( delay > 0)
        {
          Time::waitFor(delay/1000.0);
        }

        int next = increment(current, lower, upper);

        // Incrementing may cause a stop in the execmode so recheck.
        execmode_.reset();
        if( loop_ = (execmode_.get() == "play") )
        {
          current = next;

          want_to_execute();
        }
      }
    }
    else
    {
      if( execmode == "rewind" ) current = start;
      else if( execmode == "fforward" )	current = end;

      senddata = true;

      if (playmode_.get() == "inc_w_exec")
      {
        current = increment(current, lower, upper);
      }
    }

    if (use_row_)
    {
      amount = Max(1, Min(datafile_.get_numrows()-current,amount));
    }
    else
    {
      amount = Max(1, Min(datafile_.get_numcols()-current,amount));
    }

    // Put the input from the GUI in the matrix Indices
    Indices = new DenseMatrix(1, amount);
    double* dataptr = Indices->get_data_pointer();
    for (int p=0; p <amount;p++) dataptr[p] = static_cast<double>(current+p);

    current_.set(current);
    current_.reset();
  }

  MatrixHandle Output;
  MatrixHandle ScaledIndices;

  if (use_row)
  {
    if (Indices.get_rep())
    {
      if (!(datafile_.getrowmatrix(Output,Indices))) return;

      ScaledIndices = Indices;
      ScaledIndices.detach();
      double* data = ScaledIndices->get_data_pointer();
      double spacing = datafile_.get_rowspacing();

      for (size_type p=0;p<ScaledIndices->get_data_size();p++)
      {
        data[p] = spacing * data[p];
      }
    }
    else if (Weights.get_rep())
    {
      if (!(datafile_.getrowmatrix_weights(Output,Weights))) return;
    }
  }
  else
  {
    if (Indices.get_rep())
    {
      if (!(datafile_.getcolmatrix(Output,Indices))) return;

      ScaledIndices = Indices;
      ScaledIndices.detach();
      double* data = ScaledIndices->get_data_pointer();
      double spacing = datafile_.get_colspacing();

      for (size_type p=0;p<ScaledIndices->get_data_size();p++)
      {
        data[p] = spacing * data[p];
      }
    }
    else if (Weights.get_rep())
    {
      if (!(datafile_.getcolmatrix_weights(Output,Weights))) return;
    }
  }

  send_output_handle("DataVector",Output,true);
  send_output_handle("Index",Indices,true);
  send_output_handle("Scaled Index",ScaledIndices,true);
  send_output_handle("Filename",FileName,true);
}


int
StreamMatrixFromDisk::increment(int current, int lower, int upper)
{
  // Do nothing if no range.
  if (upper == lower)
  {
    if (playmode_.get() == "once")
      execmode_.set( "stop" );
    return upper;
  }
  int inc_amount = Max(1, Min(upper, inc_amount_.get()));

  current += inc_ * inc_amount;

  if (current > upper)
  {
    if (playmode_.get() == "bounce1")
    {
      inc_ *= -1;
      return increment(upper, lower, upper);
    }
    else if (playmode_.get() == "bounce2")
    {
      inc_ *= -1;
      return upper;
    }
    else
    {
      if (playmode_.get() == "once") execmode_.set( "stop" );
      return lower;
    }
  }
  if (current < lower)
  {
    if (playmode_.get() == "bounce1")
    {
      inc_ *= -1;
      return increment(lower, lower, upper);
    }
    else if (playmode_.get() == "bounce2")
    {
      inc_ *= -1;
      return lower;
    }
    else
    {
      if (playmode_.get() == "once")execmode_.set( "stop" );
      return upper;
    }
  }
  return current;
}

} // End namespace SCIRun
