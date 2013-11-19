/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_DATAIO_GENERIC_WRITER_H
#define MODULES_DATAIO_GENERIC_WRITER_H

/*
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   July 1994
 *
 */

#include <boost/filesystem.hpp>
#include <Dataflow/Network/Module.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Datatypes/String.h>

namespace SCIRun {
  namespace Modules {
    namespace DataIO {

template <class HType, class PortTag>
class GenericWriter : public SCIRun::Dataflow::Networks::Module,
  public Has2InputPorts<PortTag, StringPortTag>,
  public HasNoOutputPorts
{
public:
  GenericWriter(const std::string &name, const std::string &category, const std::string &package, const std::string& stateFilename);
  virtual ~GenericWriter() { }

  virtual void execute();

  INPUT_PORT(1, Filename, String);

protected:
  HType       handle_;
  std::string filename_, filetype_;
  Core::Algorithms::AlgorithmParameterName stateFilename_;
  StaticPortName<typename HType::element_type, 0> objectPortName_;

  //GuiFilename filename_;
  //GuiString   filetype_;
  //GuiInt      confirm_;
  //GuiInt			confirm_once_;
  bool        exporting_;

  virtual bool overwrite() { return true; } //TODO
  virtual bool call_exporter(const std::string &filename);
};


template <class HType, class PortTag>
GenericWriter<HType, PortTag>::GenericWriter(const std::string &name, const std::string &cat, const std::string &pack, const std::string& stateFilename)
  : SCIRun::Dataflow::Networks::Module(SCIRun::Dataflow::Networks::ModuleLookupInfo(name, cat, pack)),
    //filename_(get_ctx()->subVar("filename"), ""),
    //filetype_(get_ctx()->subVar("filetype"), "Binary"),
    //confirm_(get_ctx()->subVar("confirm"), sci_getenv_p("SCIRUN_CONFIRM_OVERWRITE")),
		//confirm_once_(get_ctx()->subVar("confirm-once"),0),
    stateFilename_(stateFilename),
    exporting_(false)
{
  INITIALIZE_PORT(Filename);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template <class HType, class PortTag>
bool
GenericWriter<HType, PortTag>::overwrite()
{
  std::string result;
  TCLInterface::eval(get_id() + " overwrite", result);
  if (result == std::string("0"))
  {
    warning("User chose to not save.");
    return false;
  }
  return true;
}
#endif

template <class HType, class PortTag>
bool
GenericWriter<HType, PortTag>::call_exporter(const std::string &/*filename*/)
{
  return false;
}


template <class HType, class PortTag>
void
GenericWriter<HType, PortTag>::execute()
{
  // If there is an optional input string set the filename to it in the GUI.
  //TODO: this will be a common pattern for file loading. Perhaps it will be a base class method someday...
  auto fileOption = getOptionalInput(Filename);
  if (!fileOption)
    filename_ = get_state()->getValue(stateFilename_).getString();
  else
  {
    filename_ = (*fileOption)->value();

    boost::filesystem::path path(filename_);
    path = boost::filesystem::absolute(path);
    if (!boost::filesystem::exists(path.parent_path()))
    {
      error("Could not create path to filename");
      return;
    }
    filename_ = path.string();
  }

  handle_ = getRequiredInput(objectPortName_);
  
  if (filename_.empty())
  {
    error("No filename specified.");
    return;
  }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  update_state(Executing);  
#endif
  remark("saving file " + filename_);

  if (!overwrite()) return;
 
  if (exporting_)
  {
    if (!call_exporter(filename_))
    {
      error("Export failed.");
      return;
    }
  }
  else
  {
    PiostreamPtr stream;
    if (filetype_ == "Binary")
    {
      stream = auto_ostream(filename_, "Binary", getLogger());
    }
    else
    {
      stream = auto_ostream(filename_, "Text", getLogger());
    }

    if (stream->error())
    {
      error("Could not open file for writing" + filename_);
    }
    else
    {
      Pio(*stream, handle_);
    } 
  }
}

}}}

#endif