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


#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Field.h>

#include <Core/Algorithms/Converter/MatricesToDipoleField.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool MatricesToDipoleFieldAlgo::MatricesToDipoleField(ProgressReporter* pr, MatrixHandle locations,MatrixHandle strengths,FieldHandle& Dipoles)
{
  if (locations.get_rep() == 0)
  {
    pr->error("MatricesToDipoleField: No Locations Matrix");
    return (false);
  }

  if (strengths.get_rep() == 0)
  {
    pr->error("MatricesToDipoleField: No strengths Matrix");
    return (false);
  }


  if ((locations->ncols()!=strengths->ncols())||(locations->nrows()!=strengths->nrows()))
  {
    pr->error("MatricesToDipoleField: Strength and Location Matrices should have same dimensions");
    return (false);
  }

  if ((locations->nrows()==3)&&(locations->ncols()!=3))
  {
    locations = locations->make_transpose();
    strengths = strengths->make_transpose();
  }

  DenseMatrix *loc = locations->dense();
  DenseMatrix *str = locations->dense();

  if ((loc==0)||(str==0))
  {
    pr->error("MatricesToDipoleField: Could not convert matrices in dense matrices");
    return (false);
  }

  double *locdata = loc->get_data_pointer();
  double *strdata = str->get_data_pointer();
  size_type m = loc->ncols();

  FieldInformation fi("PointCloudMesh",0,"Vector");

  Dipoles = CreateField(fi);
  VMesh* omesh = Dipoles->vmesh();
  VField* ofield = Dipoles->vfield();

  if (omesh == 0)
  {
    pr->error("MatricesToDipoleField: Could not allocate mesh");
    return (false);
  }

  index_type k = 0;
  for (index_type p = 0 ; p < m; p++)
  {
    omesh->add_point(Point(locdata[k],locdata[k+1],locdata[k+2]));
    k+=3;
  }

  if (Dipoles.get_rep() == 0)
  {
    pr->error("MatricesToDipoleField: Could not allocate field");
    return (false);
  }

  k = 0;
  VMesh::Node::iterator it, it_end;
  omesh->begin(it);
  omesh->end(it_end);
  while (it!=it_end)
  {
    ofield->set_value(Vector(strdata[k+0],strdata[k+1],strdata[k+2]),*it);
    k+=3;
    ++it;
  }

  return (true);
}

} // end namespace SCIRunAlgo
