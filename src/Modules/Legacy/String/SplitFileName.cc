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


/// @file  SplitFileName.cc
///
/// @author
///    jeroen

#include <Modules/Legacy/String/SplitFileName.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

/// @class SplitFileName
/// @brief This module splits a filename in: pathname, filename (base), and extension.

MODULE_INFO_DEF(SplitFileName, String, SCIRun)

SplitFileName::SplitFileName() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(Full_Filename);
  INITIALIZE_PORT(Pathname);
  INITIALIZE_PORT(Base);
  INITIALIZE_PORT(Extension);
  INITIALIZE_PORT(Filename);
}

void
SplitFileName::execute()
{
  auto filenameH = getRequiredInput(Full_Filename);

  const char sep = boost::filesystem::path::preferred_separator;

  auto filename = filenameH->value();

  if (needToExecute())
  {
    if (!filename.empty()) 
      if (filename.back() == sep)
        filename = filename.substr(0, filename.size() - 1);

    boost::filesystem::path p(filename);
    sendOutput(Pathname, std::make_shared<String>(p.parent_path().make_preferred().string() + sep));
    sendOutput(Base, std::make_shared<String>(p.stem().string()));
    sendOutput(Extension, std::make_shared<String>(p.extension().string()));
    sendOutput(Filename, std::make_shared<String>(p.filename().string()));
  }
}
