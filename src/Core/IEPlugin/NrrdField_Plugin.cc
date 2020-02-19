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


#include <Core/Algorithms/Legacy/DataIO/DataIOAlgo.h>
#include <Core/Algorithms/Legacy/Converter/ConverterAlgo.h>
#include <Core/IEPlugin/NrrdField_Plugin.h>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;

FieldHandle SCIRun::NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  std::string fn(filename);

  if (dalgo.readNrrd(fn,nrrd))
  {
    calgo.nrrdToField(nrrd,field);
  }

  return (field);
}

bool
SCIRun::FieldToNrrd_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  if (calgo.fieldToNrrd(fh,nrrd))
  {
    std::string fn(filename);
    return dalgo.writeNrrd(fn,nrrd);
  }

  return (false);
}

FieldHandle SCIRun::Nodal_NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  std::string fn(filename);

  if (dalgo.readNrrd(fn,nrrd))
  {
    calgo.nrrdToField(nrrd,field,"Node");
  }

  return (field);
}

FieldHandle SCIRun::Modal_NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  std::string fn(filename);

  if (dalgo.readNrrd(fn,nrrd))
  {
    calgo.nrrdToField(nrrd,field,"Element");
  }

  return (field);
}

FieldHandle SCIRun::IPNodal_NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  std::string fn(filename);

  if (dalgo.readNrrd(fn,nrrd))
  {
    calgo.nrrdToField(nrrd,field,"Node","Auto","Invert");
  }

  return (field);
}

FieldHandle SCIRun::IPModal_NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field;
  NrrdDataHandle nrrd;

  DataIOAlgo dalgo(pr);
  ConverterAlgo calgo(pr);

  std::string fn(filename);

  if (dalgo.readNrrd(fn,nrrd))
  {
    calgo.nrrdToField(nrrd,field,"Element","Auto","Invert");
  }

  return (field);
}
