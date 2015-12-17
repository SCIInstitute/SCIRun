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

#ifndef CORE_IEPLUGIN_TRISURFFIELD_PLUGIN_H__
#define CORE_IEPLUGIN_TRISURFFIELD_PLUGIN_H__

#include <Core/Logging/LoggerFwd.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/IEPlugin/share.h>

namespace SCIRun
{
  SCISHARE FieldHandle TextToTriSurfField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  SCISHARE FieldHandle MToTriSurfField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  SCISHARE FieldHandle VtkToTriSurfField_reader(Core::Logging::LoggerHandle pr, const char *filename);

  SCISHARE bool TriSurfFieldToTextBaseIndexZero_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
  SCISHARE bool TriSurfFieldToTextBaseIndexOne_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
  SCISHARE bool TriSurfFieldToM_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
  SCISHARE bool TriSurfFieldToVtk_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
  SCISHARE bool TriSurfFieldToExotxt_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
  SCISHARE bool TriSurfFieldToExotxtBaseIndexOne_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char *filename);
}

#endif
