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


#ifndef MODULES_DATAIO_GENERIC_WRITER_H
#define MODULES_DATAIO_GENERIC_WRITER_H

///
///@author
///   Steven G. Parker
///   Department of Computer Science
///   University of Utah
///@date  July 1994
///

#include <boost/filesystem.hpp>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/String.h>

namespace SCIRun {
  namespace Modules {
    namespace DataIO {

template <class HType, class PortTag, class PortDescriber = Has2InputPorts<PortTag, StringPortTag>>
class GenericWriter : public Dataflow::Networks::Module,
  public PortDescriber,
  public HasNoOutputPorts
{
public:
  GenericWriter(const std::string &name, const std::string &category, const std::string &package, const std::string& stateFilename);

  virtual void setStateDefaults() override;
  virtual void execute() override;

  INPUT_PORT(1, Filename, String);

protected:
  HType       handle_;
  std::string filename_;
  mutable std::string filetype_;
  Core::Algorithms::AlgorithmParameterName stateFilename_;
  Dataflow::Networks::StaticPortName<typename HType::element_type, 0>* objectPortName_;

  virtual std::string defaultFileTypeName() const = 0;

  //GuiInt      confirm_;
  //GuiInt			confirm_once_;

  virtual bool useCustomExporter(const std::string& filename) const = 0;
  virtual bool call_exporter(const std::string &filename) { return false; }

  virtual bool overwrite() { return true; } /// @todo
};


template <class HType, class PortTag, class PortDescriber>
GenericWriter<HType, PortTag, PortDescriber>::GenericWriter(const std::string &name, const std::string &cat, const std::string &pack, const std::string& stateFilename)
  : SCIRun::Dataflow::Networks::Module(SCIRun::Dataflow::Networks::ModuleLookupInfo(name, cat, pack)),
    //confirm_(get_ctx()->subVar("confirm"), sci_getenv_p("SCIRUN_CONFIRM_OVERWRITE")),
		//confirm_once_(get_ctx()->subVar("confirm-once"),0),
    stateFilename_(stateFilename),
    objectPortName_(nullptr)
{
  INITIALIZE_PORT(Filename);
}

template <class HType, class PortTag, class PortDescriber>
void GenericWriter<HType, PortTag, PortDescriber>::setStateDefaults()
{
  get_state()->setValue(stateFilename_, std::string());
  get_state()->setValue(SCIRun::Core::Algorithms::Variables::FileTypeName, defaultFileTypeName());
  get_state()->setValue(SCIRun::Core::Algorithms::Variables::GuiFileTypeName, std::string());
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

template <class HType, class PortTag, class PortDescriber>
void GenericWriter<HType, PortTag, PortDescriber>::execute()
{
  // If there is an optional input string set the filename to it in the GUI.
  /// @todo: this will be a common pattern for file loading. Perhaps it will be a base class method someday...
  auto fileOption = getOptionalInput(Filename);
  if (fileOption && *fileOption)
  {
    get_state()->setValue(stateFilename_, (*fileOption)->value());
  }

  filename_ = get_state()->getValue(stateFilename_).toFilename().string();

  {
    boost::filesystem::path path(filename_);
    path = boost::filesystem::absolute(path);
    if (!boost::filesystem::exists(path.parent_path()))
    {
      MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Could not create path to filename");
    }
    filename_ = path.string();
  }

  if (!objectPortName_)
  {
    MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Logical error: object port name not specified.");
  }
  handle_ = getRequiredInput(*objectPortName_);

  if (filename_.empty())
  {
    MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "No filename specified.");
  }
  if (needToExecute())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    update_state(Executing);
#endif
    remark("saving file " + filename_);

    if (!overwrite()) return;

    if (useCustomExporter(filename_))
    {
      if (!call_exporter(filename_))
      {
        MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Export failed.");
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
        MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Could not open file for writing" + filename_);
      }
      else
      {
        Pio(*stream, handle_);
      }
    }
  }
}

}}}

#endif
