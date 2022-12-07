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


#ifndef CORE_IEPLUGIN_NRRDFIELD_H__
#define CORE_IEPLUGIN_NRRDFIELD_H__

#include <Core/Logging/LoggerFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/IEPlugin/share.h>

namespace SCIRun
{
  // This file contains readers for nrrds as fields. We currently have 5 variations
  // depending on whether data is defined on the nodes or the elements and what the
  // parity of the data is.
  //
  // Although these properties are defined in the nrrd file format, most nrrds are
  // annotated improperly and hence corrections have to made.

  // Default reader use the definitions in the nrrd
  SCISHARE FieldHandle NrrdToField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  // Override the location settings and force data on the nodes
  SCISHARE FieldHandle Nodal_NrrdToField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  // Override the location settings and force data on the elements
  SCISHARE FieldHandle Modal_NrrdToField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  // Override the location settings and force data on the nodes and invert space parity
  SCISHARE FieldHandle IPNodal_NrrdToField_reader(Core::Logging::LoggerHandle pr, const char *filename);
  // Override the location settings and force data on the elements and invert space parity
  SCISHARE FieldHandle IPModal_NrrdToField_reader(Core::Logging::LoggerHandle pr, const char *filename);

  // Default writer
  SCISHARE bool FieldToNrrd_writer(Core::Logging::LoggerHandle pr, FieldHandle fh, const char* filename);
}

#endif
