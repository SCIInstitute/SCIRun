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

#include <Dataflow/Network/Module.h>

#include <Core/Datatypes/String.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

using namespace SCIRun;

/// @class SplitFileName
/// @brief This module splits a filename in: pathname, filename (base), and extension. 

class SplitFileName : public Module {
public:
  SplitFileName(GuiContext*);

  virtual ~SplitFileName();

  virtual void execute();
};


DECLARE_MAKER(SplitFileName)
SplitFileName::SplitFileName(GuiContext* ctx)
  : Module("SplitFileName", ctx, Source, "String", "SCIRun")
{
}


SplitFileName::~SplitFileName()
{
}

void
SplitFileName::execute()
{
  StringHandle filenameH;
  if (!get_input_handle("Filename", filenameH)) return;

  std::string filename, fn, pn, ext, fnext;

  const char sep = '/';
  const char dot = '.';
  
  filename = filenameH->get();
  
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
  send_output_handle("Pathname", pnH);

  StringHandle fnH(new String(fn));
  send_output_handle("Filename Base", fnH);

  StringHandle extH(new String(ext));
  send_output_handle("Extension", extH);

  StringHandle fnextH(new String(fnext));
  send_output_handle("Filename", fnextH);
}

} // End namespace SCIRun


