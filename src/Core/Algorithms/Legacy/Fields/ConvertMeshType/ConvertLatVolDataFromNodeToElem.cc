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


#include <Core/Algorithms/Fields/ConvertMeshType/ConvertLatVolDataFromNodeToElem.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
ConvertLatVolDataFromNodeToElemAlgo::
run(FieldHandle input, FieldHandle& output)
{
  /// Mark start of algorithm and report that we will not report progress
  algo_start("ConvertLatVolDataFromNodeToElem");

  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);

  if (fi.is_constantdata())
  {
    warning("The data is already at the elements, just copying ");
    output = input;
    algo_end(); return (true);
  }

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }

  if (fi.is_latvolmesh())
  {
    VField* ifield = input->vfield();
    VMesh*  imesh  = input->vmesh();
    fo.make_constantdata();

    const size_type ni = imesh->get_ni();
    const size_type nj = imesh->get_nj();
    const size_type nk = imesh->get_nk();

    const double ioff = (1.0 - (ni / (ni-1.0))) * 0.5;
    const double joff = (1.0 - (nj / (nj-1.0))) * 0.5;
    const double koff = (1.0 - (nk / (nk-1.0))) * 0.5;

    const Point minp(ioff, joff, koff);
    const Point maxp(1.0-ioff, 1.0-joff, 1.0-koff);

    MeshHandle mesh = CreateMesh(fo,ni+1,nj+1,nk+1,minp,maxp);
    if (mesh.get_rep() == 0)
    {
      error("Could not allocate output mesh");
      algo_end(); return (false);
    }
    VMesh* omesh = mesh->vmesh();

    Transform trans;
    imesh->get_canonical_transform(trans);
    omesh->transform(trans);

    output = CreateField(fo,mesh);
    if (output.get_rep() == 0)
    {
      error("Could not allocate output field");
      algo_end(); return (false);
    }
    VField* ofield = output->vfield();

    ofield->copy_values(ifield);

    algo_end(); return (true);
  }
  else if (fi.is_imagemesh())
  {
    VField* ifield = input->vfield();
    VMesh*  imesh  = input->vmesh();
    fo.make_constantdata();

    const size_type ni = imesh->get_ni();
    const size_type nj = imesh->get_nj();

    const double ioff = (1.0 - (ni / (ni-1.0))) * 0.5;
    const double joff = (1.0 - (nj / (nj-1.0))) * 0.5;

    const Point minp(ioff, joff, 00);
    const Point maxp(1.0-ioff, 1.0-joff, 0.0);

    MeshHandle mesh = CreateMesh(fo,ni+1,nj+1,minp,maxp);
    if (mesh.get_rep() == 0)
    {
      error("Could not allocate output mesh");
      algo_end(); return (false);
    }
    VMesh* omesh = mesh->vmesh();

    Transform trans;
    imesh->get_canonical_transform(trans);
    omesh->transform(trans);

    output = CreateField(fo,mesh);
    if (output.get_rep() == 0)
    {
      error("Could not allocate output field");
      algo_end(); return (false);
    }
    VField* ofield = output->vfield();

    ofield->copy_values(ifield);

    algo_end(); return (true);
  }
  else
  {
    error("This algorithm is only available for regular spaced data");
    algo_end(); return (false);
  }

}


} // End namespace SCIRunAlgo
