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

#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/ExecConverter.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Util/StringUtil.h>
#include <Core/Util/sci_system.h>
#include <Core/Util/Environment.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#ifdef _WIN32
#include <process.h>
#include <io.h>
#endif


using namespace SCIRun;

namespace SCIRun {

void
Exec_setup_command(const char *cfilename, const std::string &precommand,
		   std::string &command, std::string &tmpfilename)
{
  // Filename as string
  const std::string filename(cfilename);
  
  // Base filename.
  std::string::size_type loc = filename.find_last_of("/");
  const std::string basefilename =
    (loc==std::string::npos)?filename:filename.substr(loc+1);

  // Base filename with first extension removed.
  loc = basefilename.find_last_of(".");
  const std::string basenoext = basefilename.substr(0, loc);

  // Filename with first extension removed.
  loc = filename.find_last_of(".");
  const std::string noext = filename.substr(0, loc);

  // Temporary filename.
  tmpfilename = std::string(sci_getenv("SCIRUN_TMP_DIR")) + "/" + basenoext + "-" +
    to_string((unsigned int)(getpid())) + ".sci";

  // Filename with first extension removed.
  loc = filename.find_last_of(".");
  //const std::string filenoext = filename.substr(0, loc);

  // Replace all of the variables in the reader command.
  command = precommand;
  while ((loc = command.find("%f")) != std::string::npos)
  {
    command.replace(loc, 2, filename);
  }
  while ((loc = command.find("%e")) != std::string::npos)
  {
    command.replace(loc, 2, noext);
  }
  while ((loc = command.find("%t")) != std::string::npos)
  {
    command.replace(loc, 2, tmpfilename);
  }
}


bool
Exec_execute_command(ProgressReporter *pr,
		     const std::string &icommand, const std::string &tmpfilename)
{
  pr->remark("ExecConverter - Executing: " + icommand + ".");

  FILE *pipe = 0;
  bool result = true;
  std::string command = icommand + " > " + tmpfilename + ".log 2>&1";
  const int status = sci_system(command.c_str());
  if (status != 0)
  {
    pr->error("ExecConverter syscal error " + to_string(status) + ": "
	      + "command was '" + command + "'.");
    result = false;
  }
  pipe = fopen((tmpfilename + ".log").c_str(), "r");

  char buffer[256];
  while (pipe && fgets(buffer, 256, pipe) != NULL)
  {
    pr->remark(buffer);
  }

  if (pipe)
  {
    fclose(pipe);
    unlink((tmpfilename + ".log").c_str());
  }

  return result;
}

} // namespace SCIRun


