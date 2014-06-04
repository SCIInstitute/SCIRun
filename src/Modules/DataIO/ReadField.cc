/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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
///@file  ReadField.cc 
///@brief Read a persistent field from a file
///
///@author
///   Steven G. Parker
///   Department of Computer Science
///   University of Utah
///@date   July 1994
///

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/DataIO/ReadField.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::DataIO;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
class ReadField : public GenericReader<FieldHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_filetype_;
    GuiString gui_filename_base_;
    GuiInt    gui_number_in_series_;
    GuiInt    gui_delay_;

    virtual bool call_importer(const std::string &filename, FieldHandle &fHandle);

  public:
    ReadField(GuiContext* ctx);
    virtual ~ReadField() {}
    virtual void execute();
};


DECLARE_MAKER(ReadField)
#endif

ReadFieldModule::ReadFieldModule()
  : my_base("ReadField", "DataIO", "SCIRun", "Field")    
    //gui_filetype_(get_ctx()->subVar("filetype")),
    //gui_filename_base_(get_ctx()->subVar("filename_base"), ""),
    //gui_number_in_series_(get_ctx()->subVar("number_in_series"), 0),
    //gui_delay_(get_ctx()->subVar("delay"), 0)
{
  INITIALIZE_PORT(Field);

  FieldIEPluginManager mgr;
  auto types = makeGuiTypesList(mgr);

  get_state()->setValue(Variables::FileTypeList, types);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
bool
ReadField::call_importer(const std::string &filename,
			 FieldHandle & fHandle)
{
  const std::string ftpre = gui_filetype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);
  
  FieldIEPluginManager mgr;
  FieldIEPlugin *pl = mgr.get_plugin(ft);
  if (pl)
  {
    fHandle = pl->filereader(this, filename.c_str());
    return fHandle.get_rep();
  }
  return false;
}
#endif

void
ReadFieldModule::execute()
{     
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (gui_types_.changed() || gui_filetype_.changed()) inputs_changed_ = true; 

  const std::string guiFiletype = get_state()->getValue(Variables::FileExtension).getString();

  useCustomImporter_ = guiFiletype != ".fld";
#endif
  my_base::execute();
}

bool ReadFieldModule::useCustomImporter(const std::string& filename) const 
{
  return false;
}