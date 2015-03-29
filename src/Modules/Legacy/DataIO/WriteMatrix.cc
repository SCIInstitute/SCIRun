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

///@author
///   Steven G. Parker
///   Department of Computer Science
///   University of Utah
///@date  July 1994

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Modules/DataIO/GenericWriter.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>

namespace SCIRun {

template class GenericWriter<MatrixHandle>;

/// @class WriteMatrix
/// @brief This module writes a matrix to file 
///
/// (a SCIRun .mat file and various other file formats using a plugin system). 

class WriteMatrix : public GenericWriter<MatrixHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_exporttype_;
    GuiInt split_;

    virtual bool call_exporter(const std::string &filename);

  public:
    WriteMatrix(GuiContext* ctx);
    virtual ~WriteMatrix() {}

    virtual void execute();
};


DECLARE_MAKER(WriteMatrix)

WriteMatrix::WriteMatrix(GuiContext* ctx)
  : GenericWriter<MatrixHandle>("WriteMatrix", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_exporttype_(get_ctx()->subVar("exporttype"), ""),
    split_(get_ctx()->subVar("split"), 0)
{
  MatrixIEPluginManager mgr;
  std::vector<std::string> exporters;
  mgr.get_exporter_list(exporters);
  
  std::string exporttypes = "{";
  exporttypes += "{{SCIRun Matrix Binary} {.mat} } ";
  exporttypes += "{{SCIRun Matrix ASCII} {.mat} } ";

  for (unsigned int i = 0; i < exporters.size(); i++)
  {
    MatrixIEPlugin *pl = mgr.get_plugin(exporters[i]);
    if (pl->fileExtension_ != "")
    {
      exporttypes += "{{" + exporters[i] + "} {" + pl->fileExtension_ + "} } ";
    }
    else
    {
      exporttypes += "{{" + exporters[i] + "} {.*} } ";
    }
  }

  exporttypes += "}";

  gui_types_.set(exporttypes);
}

bool
WriteMatrix::call_exporter(const std::string &filename)
{
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);
  
  MatrixIEPluginManager mgr;
  MatrixIEPlugin *pl = mgr.get_plugin(ft);
  if (pl)
  {
    const bool result = pl->fileWriter_(this, handle_, filename.c_str());
    return result;
  }
  return false;
}


void
WriteMatrix::execute()
{
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  StringHandle filename;
  if (get_input_handle("Filename",filename,false)) 
  {
		// This piece of code makes sure that the path to the output
		// file exists and that we use an absolute filename
		FullFileName ffn(filename->get());
		if (!(ffn.create_file_path()))
		{
			error("Could not create path to filename");
			return;
		}
		filename = new String(ffn.get_abs_filename());

    filename_.set(filename->get());
    get_ctx()->reset();
  }

  exporting_ = !(ft == "" ||
		 ft == "SCIRun Matrix Binary" ||
		 ft == "SCIRun Matrix ASCII" ||
                 ft == "Binary");

  // Determine if we're ASCII or Binary
  std::string ab = "Binary";
  if (ft == "SCIRun Matrix ASCII") ab = "ASCII";
  filetype_.set(ab);

  // Read data from the input port
  if (!get_input_handle("Input Data", handle_)) return;

  // If no name is provided, return.
  const std::string fn(filename_.get());
  if (fn == "")
  {
    warning("No filename specified.");
    return;
  }

  if (!overwrite()) return;

  if (exporting_)
  {
    if (!call_exporter(fn))
    {
      error("Export failed.");
      return;
    }
  }
  else
  {
    // Open up the output stream
    PiostreamPtr stream;
    std::string ft(filetype_.get());
    if (ft == "Binary")
    {
      stream = auto_ostream(fn, "Binary", this);
    }
    else
    {
      stream = auto_ostream(fn, "Text", this);
    }

    // Check whether the file should be split into header and data
    handle_->set_raw(split_.get());
  
    if (stream->error()) {
      error("Could not open file for writing" + fn);
    } else {
      // Write the file
      Pio(*stream, handle_);
    } 
  }
}

} // End namespace SCIRun
