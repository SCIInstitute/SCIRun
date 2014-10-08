
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

#include <Core/Algorithms/Legacy/DataIO/DataIOAlgo.h>
#include <Core/Algorithms/Legacy/Converter/ConverterAlgo.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;

// This file contains readers for nrrds as fields. We currently have 5 variations
// depending on whether data is defined on the nodes or the elements and what the
// parity of the data is.
// 
// Although these properties are defined in the nrrd file format, most nrrds are
// annotated improperly and hence corrections have to made.

//// Default reader use the definitions in the nrrd
//FieldHandle  NrrdToField_reader(ProgressReporter *pr, const char *filename);
//// Override the location settings and force data on the nodes
//FieldHandle  Nodal_NrrdToField_reader(ProgressReporter *pr, const char *filename);
//// Override the location settings and force data on the elements
//FieldHandle  Modal_NrrdToField_reader(ProgressReporter *pr, const char *filename);
//// Override the location settings and force data on the nodes and invert space parity
//FieldHandle  IPNodal_NrrdToField_reader(ProgressReporter *pr, const char *filename);
//// Override the location settings and force data on the elements and invert space parity
//FieldHandle  IPModal_NrrdToField_reader(ProgressReporter *pr, const char *filename);
//
//// Default writer
//bool FieldToNrrd_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

FieldHandle SCIRun::NrrdToField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  std::string fn(filename);
  
  if (dalgo.ReadNrrd(fn,nrrd))
  {
    calgo.NrrdToField(nrrd,field);
  }
  
  return (field);
}

bool
SCIRun::FieldToNrrd_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{ 
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  if (calgo.FieldToNrrd(fh,nrrd))
  {
    std::string fn(filename);
    return(dalgo.WriteNrrd(fn,nrrd));
  }
  
  return (false);
}

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
FieldHandle Nodal_NrrdToField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  std::string fn(filename);
  
  if (dalgo.ReadNrrd(fn,nrrd))
  {
    calgo.NrrdToField(nrrd,field,"Node");
  }
  
  return (field);
}

FieldHandle Modal_NrrdToField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  std::string fn(filename);
  
  if (dalgo.ReadNrrd(fn,nrrd))
  {
    calgo.NrrdToField(nrrd,field,"Element");
  }
  
  return (field);
}

FieldHandle IPNodal_NrrdToField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  std::string fn(filename);
  
  if (dalgo.ReadNrrd(fn,nrrd))
  {
    calgo.NrrdToField(nrrd,field,"Node","Auto","Invert");
  }
  
  return (field);
}

FieldHandle IPModal_NrrdToField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle field = 0;
  NrrdDataHandle nrrd = 0;
  
  SCIRunAlgo::DataIOAlgo dalgo(pr);
  SCIRunAlgo::ConverterAlgo calgo(pr);

  std::string fn(filename);
  
  if (dalgo.ReadNrrd(fn,nrrd))
  {
    calgo.NrrdToField(nrrd,field,"Element","Auto","Invert");
  }
  
  return (field);
}
#endif
//
//static FieldIEPlugin  NrrdToField_plugin("NrrdFile","{.nhdr} {.nrrd}", "*.nrrd", NrrdToField_reader, FieldToNrrd_writer);
//static FieldIEPlugin  NodalNrrdToField_plugin("NrrdFile[DataOnNodes]","{.nhdr} {.nrrd}", "", Nodal_NrrdToField_reader, 0);
//static FieldIEPlugin  ModalNrrdToField_plugin("NrrdFile[DataOnElements]","{.nhdr} {.nrrd}", "", Modal_NrrdToField_reader, 0);
//static FieldIEPlugin  IPNodalNrrdToField_plugin("NrrdFile[DataOnNodes,InvertParity]","{.nhdr} {.nrrd}", "", IPNodal_NrrdToField_reader, 0);
//static FieldIEPlugin  IPModalNrrdToField_plugin("NrrdFile[DataOnElements,InvertParity]","{.nhdr} {.nrrd}", "", IPModal_NrrdToField_reader, 0);


}
