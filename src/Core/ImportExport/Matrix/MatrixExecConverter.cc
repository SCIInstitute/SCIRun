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

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/ImportExport/ExecConverter.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Util/StringUtil.h>
#include <Core/Util/sci_system.h>
#include <Core/Util/Environment.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include <sci_deprecated.h>

#ifdef _WIN32
#include <process.h>
#include <io.h>
#endif


using namespace SCIRun;


// ColumnMatrix

///// NOTE: These procedures are not static because I need to
/////       reference them in FieldIEPlugin.cc to force the Mac OSX to 
/////       instantiate static libraries.
/*

MatrixHandle
TextColumnMatrix_reader(ProgressReporter *pr, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "TextToColumnMatrix %f %t";
  MatrixHandle result;
  Exec_reader(pr, result, filename, command);
  return result;
}

bool
TextColumnMatrix_writer(ProgressReporter *pr,
			MatrixHandle matrix, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "ColumnMatrixToText %t %f";
  return Exec_writer(pr, matrix, filename, command);
}

#ifndef __APPLE__
static MatrixIEPlugin
TextColumnMatrix_plugin("TextColumnMatrix",
			"", "",
			TextColumnMatrix_reader,
			TextColumnMatrix_writer);
#endif


// DenseMatrix

static MatrixHandle
TextDenseMatrix_reader(ProgressReporter *pr, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "TextToDenseMatrix %f %t";
  MatrixHandle result;
  Exec_reader(pr, result, filename, command);
  return result;
}

static bool
TextDenseMatrix_writer(ProgressReporter *pr,
		       MatrixHandle matrix, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "DenseMatrixToText %t %f";
  return Exec_writer(pr, matrix, filename, command);
}

static MatrixIEPlugin
TextDenseMatrix_plugin("TextDenseMatrix",
		       "", "",
		       TextDenseMatrix_reader,
		       TextDenseMatrix_writer);


// SparseRowMatrix

static MatrixHandle
TextSparseRowMatrix_reader(ProgressReporter *pr, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "TextToSparseRowMatrix %f %t";
  MatrixHandle result;
  Exec_reader(pr, result, filename, command);
  return result;
}

static bool
TextSparseRowMatrix_writer(ProgressReporter *pr,
			   MatrixHandle matrix, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_OBJDIR"));
  const string command =
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "SparseRowMatrixToText %t %f";
  return Exec_writer(pr, matrix, filename, command);
}

static MatrixIEPlugin
TextSparseRowMatrix_plugin("TextSparseRowMatrix",
			   "", "",
			   TextSparseRowMatrix_reader,
			   TextSparseRowMatrix_writer);

//  Conversion of a NeuroFEM/CAUCHY/CURRY ca_perm.knw file in tensor form to SCIRun DenseMatrix format

static MatrixHandle
NeuroFEMknw_reader(ProgressReporter *pr, const char *filename)
{
  ASSERT(sci_getenv("SCIRUN_SRCDIR"));
  const string command =
    string(sci_getenv("SCIRUN_SRCDIR")) + "/convert/" +
    "KnwToDenseMatrix.pl %f %t1 && " +
    string(sci_getenv("SCIRUN_OBJDIR")) + "/convert/" +
    "TextToDenseMatrix %t1 %t";
  MatrixHandle result;
  Exec_reader(pr, result, filename, command);
  return result;
}

static MatrixIEPlugin
NeuroFEMknw_plugin("NeuroFEMTensor",
		       "", "",
		       NeuroFEMknw_reader,
		       NULL);

*/
