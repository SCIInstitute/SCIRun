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


#include <Core/Algorithms/Converter/MatrixToField.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <vector>

namespace SCIRunAlgo {

using namespace SCIRun;

bool MatrixToFieldAlgo::MatrixToField(ProgressReporter* pr, MatrixHandle input, FieldHandle& output, const std::string& datalocation)
{
  MatrixHandle mat = input->dense();
  if (mat.get_rep() == 0)
  {
    pr->error("MatrixToField: Could not convert matrix into dense matrix");
    return (false);
  }

  size_type m = mat->ncols();
  size_type n = mat->nrows();
  double* dataptr = mat->get_data_pointer();

  if (datalocation == "Node")
  {
    FieldInformation fi("ImageMesh",1,"double");
    MeshHandle mesh = CreateMesh(fi,m,n,Point(0.0,0.0,0.0),Point(static_cast<double>(m),static_cast<double>(n),0.0));
    output = CreateField(fi,mesh);
    output->vfield()->set_values(dataptr,m*n);
  }
  else if (datalocation == "Element")
  {
    FieldInformation fi("ImageMesh",0,"double");
    MeshHandle mesh = CreateMesh(fi,m+1,n+1,Point(0.0,0.0,0.0),Point(static_cast<double>(m+1),static_cast<double>(n+1),0.0));
    output = CreateField(fi,mesh);
    output->vfield()->set_values(dataptr,m*n);
  }
  else
  {
    pr->error("MatrixToField: Data location information is not recognized");
    return (false);
  }

  return (true);
}

} // end namespace SCIRunAlgo
