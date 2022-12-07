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
///@author
///   Steven G. Parker
///   Department of Computer Science
///   University of Utah
///@date  July 1994
///

#ifndef MODULES_DATAIO_GENERIC_READER_H
#define MODULES_DATAIO_GENERIC_READER_H

#include <boost/filesystem.hpp>
#include <Core/Datatypes/String.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Thread/Mutex.h>
#include <Core/Utils/Legacy/Environment.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {
  namespace Modules {
    namespace DataIO {

template <class HType, class PortTag>
class GenericReader : public SCIRun::Dataflow::Networks::Module,
  public Has1InputPort<StringPortTag>,
  public Has2OutputPorts<PortTag, StringPortTag>
{
public:
  GenericReader(const std::string &name, const std::string &category, const std::string &package, const std::string& stateFilename);

  void setStateDefaults() override final;
  void execute() override;
  INPUT_PORT(0, Filename, String);
  //OUTPUT_PORT(0, Object, PortType);
  OUTPUT_PORT(1, FileLoaded, String);

protected:
  std::string filename_;
  Dataflow::Networks::StaticPortName<typename HType::element_type, 0> objectPortName_;
  virtual std::string defaultFileTypeName() const = 0;
  //GuiFilename gui_filename_;
  //GuiString gui_from_env_;

  time_t old_filemodification_;

  virtual bool useCustomImporter(const std::string& filename) const = 0;
  virtual bool call_importer(const std::string& /*filename*/, HType& /*handle*/) { return false; }

  static Core::Thread::Mutex fileCheckMutex_;
  static bool file_exists(const std::string& filename);
};


template <class HType, class PortTag>
GenericReader<HType, PortTag>::GenericReader(const std::string &name,
				    const std::string &cat, const std::string &pack, const std::string& objectPortName)
  : SCIRun::Dataflow::Networks::Module(SCIRun::Dataflow::Networks::ModuleLookupInfo(name, cat, pack)),
    //gui_filename_(get_ctx()->subVar("filename"), ""),
    //gui_from_env_(get_ctx()->subVar("from-env"),""),
    objectPortName_(SCIRun::Dataflow::Networks::PortId(0, objectPortName)),
    old_filemodification_(0)
{
  INITIALIZE_PORT(Filename);
  INITIALIZE_PORT(FileLoaded);
}

template <class HType, class PortTag>
void GenericReader<HType, PortTag>::setStateDefaults()
{
  auto state = get_state();
  state->setValue(SCIRun::Core::Algorithms::Variables::Filename, std::string());
  state->setValue(SCIRun::Core::Algorithms::Variables::FileTypeName, defaultFileTypeName());
  state->setValue(SCIRun::Core::Algorithms::Variables::GuiFileTypeName, std::string());
  state->setValue(SCIRun::Core::Algorithms::Variables::ScriptEnvironmentVariable, std::string());
}

template <class HType, class PortTag>
Core::Thread::Mutex GenericReader<HType,PortTag>::fileCheckMutex_("GenericReader");

template <class HType, class PortTag>
bool
  GenericReader<HType, PortTag>::file_exists(const std::string & filename)
{
  //BOOST FILESYSTEM BUG: it is not thread-safe. TODO: need to meld this locking code into the ENSURE_FILE_EXISTS macro.
  Core::Thread::Guard guard(GenericReader<HType, PortTag>::fileCheckMutex_.get());
  return boost::filesystem::exists(filename);
}

template <class HType, class PortTag>
void
GenericReader<HType, PortTag>::execute()
{
  auto state = get_state();
  auto environmentVariable = state->getValue(Core::Algorithms::Variables::ScriptEnvironmentVariable).toString();
  if (!environmentVariable.empty())
  {
    if (sci_getenv(environmentVariable))
    {
      std::string envfilename(sci_getenv(environmentVariable));
      state->setValue(SCIRun::Core::Algorithms::Variables::Filename, envfilename);
    }
    else
    {
      warning("No filename found under environment variable " + environmentVariable + ", reverting to GUI input.");
    }
  }
  else
  {
    // If there is an optional input string set the filename to it in the GUI.
    /// @todo: this will be a common pattern for file loading. Perhaps it will be a base class method someday...
    auto fileOption = getOptionalInput(Filename);
    if (fileOption && *fileOption)
    {
      state->setValue(SCIRun::Core::Algorithms::Variables::Filename, (*fileOption)->value());
    }
  }
  filename_ = state->getValue(SCIRun::Core::Algorithms::Variables::Filename).toFilename().string();


  // Read the status of this file so we can compare modification timestamps

  if (filename_.empty())
  {
    MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "No file has been selected. Please choose a file.");
  }
  else if (!file_exists(filename_))
  {
    if (!useCustomImporter(filename_))
    {
      MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "File '" + filename_ + "' not found.");
    }
    else
    {
      warning("File '" + filename_ + "' not found.  Maybe the plugin can find it?");
    }
  }

  // If we haven't read yet, or if it's a new filename,
  //  or if the datestamp has changed -- then read...

  time_t new_filemodification = boost::filesystem::last_write_time(filename_);

  if (new_filemodification != old_filemodification_ ||
    needToExecute()
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    || filename_changed ||
      !oport_cached(0) ||
      !oport_cached("Filename")
#endif
      )
  {
    old_filemodification_ = new_filemodification;

    HType handle;

    remark("loading file " +filename_);

    if (useCustomImporter(filename_))
    {
      if (!call_importer(filename_, handle))
      {
        MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Import failed.");
      }
    }
    else
    {
      auto stream = auto_istream(filename_, getLogger());
      if (!stream)
      {
        MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Error reading file '" + filename_ + "'.");
      }

      // Read the file
      Pio(*stream, handle);

      if (!handle || stream->error())
      {
        MODULE_ERROR_WITH_TYPE(Dataflow::Networks::GeneralModuleError, "Error reading data from file '" + filename_ + "'.");
      }
    }

    Core::Datatypes::StringHandle shandle(new Core::Datatypes::String(filename_));
    sendOutput(FileLoaded, shandle);
    sendOutput(objectPortName_, handle);
  }

}

} }}

#endif
