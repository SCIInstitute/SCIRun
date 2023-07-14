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

#include <Modules/DataIO/WriteBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::DataIO;

/// @class WriteBundle
/// @brief This module writes a bundle to file (a SCIRun .bdl file).

MODULE_INFO_DEF(WriteBundle, DataIO, SCIRun)

WriteBundle::WriteBundle()
  : my_base(staticInfo_.module_name_, staticInfo_.category_name_, staticInfo_.package_name_, "Filename")
{
  // guiTypes_(get_ctx()->subVar("types", false)),
  // guiFileType_(get_ctx()->subVar("filetype"),"Binary")
  //
  // std::string exporttypes = "{";
  // exporttypes += "{{SCIRun Bundle File} {.bdl} } ";
  // exporttypes += "{{SCIRun Bundle Any} {.*} } ";
  // exporttypes += "}";
  //
  // guiTypes_.set(exporttypes);
  INITIALIZE_PORT(BundleToWrite)
  objectPortName_ = &BundleToWrite;
  filetype_ = "Binary";
}

void WriteBundle::execute()
{
  // const std::string ftpre = guiFileType_.get();
  // const std::string::size_type loc = ftpre.find(" (");
  // const std::string ft = ftpre.substr(0, loc);
  //
  // exporting_ = false;
  my_base::execute();
}

std::string WriteBundle::defaultFileTypeName() const
{
  return "*.bdl";
}
