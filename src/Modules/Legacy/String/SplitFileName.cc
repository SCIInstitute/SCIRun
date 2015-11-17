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

///@file  SplitFileName.cc
///
///@author
///   jeroen

#include <Modules/Legacy/String/SplitFileName.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun;
using namespace SCIRun::Modules::StringManip;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

/// @class SplitFileName
/// @brief This module splits a filename in: pathname, filename (base), and extension. 

const ModuleLookupInfo SplitFileName::staticInfo_("SplitFileName", "String", "SCIRun");


SplitFileName::SplitFileName() : Module(staticInfo_,false)
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
  

  std::string filename, fn, pn, ext, fnext;

  const char sep = '/';
  const char dot = '.';
  
  filename = filenameH->value();
  
  if (needToExecute())
  {
    
      if (filename.size() > 0) if (filename[filename.size()-1] == sep) 
                                                              filename = filename.substr(0,filename.size()-1);
      
    int lastsep = -1;
    for (size_t p = 0; p < filename.size(); p++) if (filename[p] == sep) lastsep = (int)p;
    
    if (lastsep > -1)
    {
      pn = filename.substr(0,lastsep+1);
      fn = filename.substr(lastsep+1);
    }
    else
    {
      pn = "";
      fn = filename;
    }

    int lastdot = -1;
    for (size_t p = 0; p < fn.size(); p++) if (fn[p] == dot) lastdot = (int)p;
    
    if (lastdot > -1)
    {
      ext = fn.substr(lastdot);
      fn = fn.substr(0,lastdot);
    }
    else
    {
      ext = "";
    }

    fnext = fn+ext;
    
    StringHandle pnH(new String(pn));
    sendOutput(Pathname, pnH);

    StringHandle fnH(new String(fn));
    sendOutput(Base, fnH);

    StringHandle extH(new String(ext));
    sendOutput(Extension, extH);

    StringHandle fnextH(new String(fnext));
    sendOutput(Filename, fnextH);
  }
}



