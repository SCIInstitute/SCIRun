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
/// @todo Documentation Modules/DataIO/WriteG3D.cc


#include <Modules/DataIO/WriteG3D.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Core::Datatypes;

const Dataflow::Networks::ModuleLookupInfo WriteG3D::staticInfo_("WriteG3D", "DataIO", "SCIRun");

WriteG3D::WriteG3D()
  : my_base(staticInfo_.module_name_, staticInfo_.category_name_, staticInfo_.package_name_, "Filename")
    //gui_increment_(get_ctx()->subVar("increment"), 0),
    //gui_current_(get_ctx()->subVar("current"), 0)
{
  INITIALIZE_PORT(FieldToWrite);
  filetype_ = "Binary";
  objectPortName_ = &FieldToWrite;

  FieldIEPluginManager mgr;
  auto types = makeGuiTypesListForExport(mgr);
  get_state()->setValue(Variables::FileTypeList, types);
}

bool WriteG3D::call_exporter(const std::string& filename)
{
  ///@todo: how will this work via python? need more code to set the filetype based on the extension...
  FieldIEPluginManager mgr;
  auto pl = mgr.get_plugin(get_state()->getValue(Variables::FileTypeName).toString());
  if (pl)
  {
    return pl->writeFile(handle_, filename, getLogger());
  }
  return false;
}

void WriteG3D::execute()
{
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //get the current file name
  const std::string oldfilename=filename_.get();
  
  //determine if we should increment an index in the file name
  if (gui_increment_.get()) 
  {

    //warn the user if they try to use 'Increment' incorrectly	
    const std::string::size_type loc2 = oldfilename.find("%d");
    if(loc2 == std::string::npos) 
    {
      remark("To use the increment function, there must be a '%d' in the file name.");
    }
    
    char buf[1024];
   
    int current=gui_current_.get();
    sprintf(buf, filename_.get().c_str(), current);
    
    filename_.set(buf);
    gui_current_.set(current+1);
  }
#endif

  my_base::execute();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   if (gui_increment_.get())
    filename_.set(oldfilename);
#endif
}

bool WriteG3D::useCustomExporter(const std::string& filename) const
{
  auto ft = get_state()->getValue(Variables::FileTypeName).toString();
  LOG_DEBUG("WriteG3D with filetype " << ft);
  auto ret = boost::filesystem::extension(filename) != ".fld";
  
  filetype_ = ft.find("SCIRun Field ASCII") != std::string::npos ? "ASCII" : "Binary";

  return ret;
}

std::string WriteG3D::defaultFileTypeName() const
{
  FieldIEPluginManager mgr;
  return defaultImportTypeForFile(&mgr);
}