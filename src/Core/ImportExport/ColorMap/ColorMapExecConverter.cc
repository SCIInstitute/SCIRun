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


#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
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

/*
// CurveColorMap

///// NOTE: The following 2 procedures are not static because I need to
/////       reference them in FieldIEPlugin.cc to force the Mac OSX to 
/////       instantiate static libraries.

ColorMapHandle
TextColorMap_reader(ProgressReporter *pr, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "TextToColorMap %f %t";
  ColorMapHandle result;
  Exec_reader(pr, result, filename, command);
  return result;
}

bool
TextColorMap_writer(ProgressReporter *pr,
		    ColorMapHandle colormap, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "ColorMapToText %t %f";
  return Exec_writer(pr, colormap, filename, command);
}

#ifndef __APPLE__
// On the Mac, this is done in FieldIEPlugin.cc, in the
// macImportExportForceLoad() function to force the loading of this
// (and other) plugins.
static ColorMapIEPlugin
TextColorMap_plugin("TextColorMap",
		    "", "",
		    TextColorMap_reader,
		    TextColorMap_writer);
#endif

*/
