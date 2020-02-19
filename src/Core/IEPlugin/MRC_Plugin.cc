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


#include <Core/Algorithms/DataIO/MRCReader.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>

namespace SCIRun {

NrrdDataHandle MRCToNrrd_reader(ProgressReporter *pr, const char *filename);

NrrdDataHandle MRCToNrrd_reader(ProgressReporter *pr, const char *filename)
{
  NrrdDataHandle nrrd = 0;
  SCIRunAlgo::MRCReader reader(pr);
  std::string fn(filename);
  if (! reader.read(fn, nrrd))
  {
    if (pr) pr->error("Convert MRC file to NRRD format failed.");
    return (nrrd);
  }
  return (nrrd);
}

static NrrdIEPlugin MRCToNrrd_plugin("MRCFile","{.mrc}", "", MRCToNrrd_reader, 0);

}
