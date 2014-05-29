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


/*
 *  ExecConverter.cc
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   December 2004
 *
 */

// Use a standalone converter to convert a scirun object into a
// temporary file, then read in that file.

#ifndef SCI_project_ExecConverter_h
#define SCI_project_ExecConverter_h 1

#include <Core/Persistent/Pstreams.h>

#include <Core/ImportExport/share.h>

namespace SCIRun {

// The implementation of these two functions is in
// Field/ExecConverter.cc for now.
void
Exec_setup_command(const char *cfilename, const std::string &precommand,
		   std::string &command, std::string &tmpfilename);

SCISHARE bool
Exec_execute_command(ProgressReporter *pr,
		     const std::string &icommand, const std::string &tmpfilename);


template <class HType>
static bool
Exec_reader(ProgressReporter *pr, HType &handle,
	    const char *cfilename, const std::string &precommand)
{
  handle = 0;

  std::string command, tmpfilename;
  Exec_setup_command(cfilename, precommand, command, tmpfilename);

  if (Exec_execute_command(pr, command, tmpfilename))
  {
    PiostreamPtr stream = auto_istream(tmpfilename, pr);
    if (!stream)
    {
      pr->error("ExecConverter - Error reading converted file '" +
		tmpfilename + "'.");
      return false;
    }
    
    // Read the file
    Pio(*stream, handle);

    pr->remark(std::string("ExecConverter - Successfully converted ")
	       + cfilename + ".");

    unlink(tmpfilename.c_str());
    
    return true;
  }

  unlink(tmpfilename.c_str());
  return false;
}



template <class HType>
static bool
Exec_writer(ProgressReporter *pr,
	    HType handle,
	    const char *cfilename, const std::string &precommand)
{
  std::string command, tmpfilename;
  bool result = true;

  Exec_setup_command(cfilename, precommand, command, tmpfilename);

  Piostream *stream = new BinaryPiostream(tmpfilename, Piostream::Write);
  if (stream->error())
  {
    delete stream;
    pr->error("ExecConverter - Could not open temporary file '" + tmpfilename +
	      "' for writing.");
    result = false;
  }
  else
  {
    Pio(*stream, handle);
    delete stream;
    result = Exec_execute_command(pr, command, tmpfilename);
  }
  unlink(tmpfilename.c_str());

  return result;
}


} // End namespace SCIRun

#endif
