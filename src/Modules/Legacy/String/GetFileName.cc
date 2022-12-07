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
/// @file  GetFileName.cc
///
/// @author
///    jeroen

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/DenseMatrix.h>


namespace SCIRun {

using namespace SCIRun;

/// @class GetFileName
/// @brief This Module gets a filename and stores it in a string.

class GetFileName : public Module {
public:
  GetFileName(GuiContext*);

  virtual ~GetFileName();

  virtual void execute();
  virtual void tcl_command(GuiArgs& args, void* userdata);

protected:
  GuiFilename gui_filename_;
  GuiString   gui_filename_base_;
  GuiInt      gui_number_in_series_;
  GuiInt      gui_delay_;
  GuiInt      gui_pinned_;

  std::vector< StringHandle > stringHandles_;
};


DECLARE_MAKER(GetFileName)
GetFileName::GetFileName(GuiContext* ctx)
  : Module("GetFileName", ctx, Source, "String", "SCIRun"),
    gui_filename_(get_ctx()->subVar("filename"), ""),
    gui_filename_base_(get_ctx()->subVar("filename_base"), ""),
    gui_number_in_series_(get_ctx()->subVar("number_in_series"), 0),
    gui_delay_(get_ctx()->subVar("delay"), 0),
    gui_pinned_(get_ctx()->subVar("pinned"), 0)
{
}


GetFileName::~GetFileName()
{
}

void
GetFileName::execute()
{
  // If there are strings in the list send them down
  if( stringHandles_.size() )
  {
    StringHandle sHandle = stringHandles_[0];
    stringHandles_.erase( stringHandles_.begin() );

    send_output_handle("Full Filename", sHandle );

    // If there are still some strings in the list execute again.
    if( stringHandles_.size() )
    {
      want_to_execute();
    }
  }

  // Check to see if the input has changed.
  else if( gui_filename_.changed( true ) ||
	   !oport_cached("Full Filename") )
  {
    StringHandle handle(new String(gui_filename_.get()));
    send_output_handle("Full Filename", handle);
  }

  /// If no data or an input change recreate the field. I.e Only
  /// execute when neeed.
  if( gui_number_in_series_.changed( true ) ||
      !oport_cached("Number in Series") )
  {
    DenseMatrix *selected = new DenseMatrix(1,1);

    selected->put(0, 0, gui_number_in_series_.get() );

    // Send the data downstream.
    MatrixHandle handle(selected);

    send_output_handle("Number in Series", handle);
  }
}


// NEED TO FIX THIS MODULE TO USE CALLBACKS

void GetFileName::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2){
    args.error("GetFileName needs a minor command");
    return;
  }

  // Store all of the string names in a list and then let the execute
  // peel them off. This is done so names are not lost when using the
  // VCR controls.
  if (args[1] == "sendfilename")
  {
    // For somereason it is possible to add a name twice when the
    // delay is too short for the network to finish executing. As
    // such, do not add the name unless it has change.
    if( gui_filename_.changed( true ) )
    {
      stringHandles_.push_back( StringHandle(new String(gui_filename_.get())) );
      // Send an execute message - but only if there is one string.
      // The other strings will be handled once the first one has been
      // sent.
      if( stringHandles_.size() == 1 ) want_to_execute();
    }
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}


} // End namespace SCIRun
