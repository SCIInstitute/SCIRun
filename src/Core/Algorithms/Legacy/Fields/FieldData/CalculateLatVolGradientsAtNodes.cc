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


#include <Core/Algorithms/Fields/FieldData/CalculateLatVolGradientsAtNodes.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
CalculateLatVolGradientsAtNodesAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("CalculateLatVolGradientsAtNodes");
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  if ((!(fi.is_latvolmesh()))&&(!(fi.is_imagemesh())))
  {
    error("This algorithm is only formulated for latvol meshes");
    algo_end(); return (false);
  }

  if (fi.is_nodata())
  {
    error("Input field does not have data associated with it");
    algo_end(); return (false);
  }

  if (!(fi.is_lineardata()))
  {
    error("The data at the input field is not located at the nodes");
    algo_end(); return (false);
  }

  if (!(fi.is_scalar()))
  {
    error("The data needs to be of scalar type to calculate gradients");
    algo_end(); return (false);
  }

  if (fi.is_latvolmesh())
  {
    fi.make_vector();
    output = CreateField(fi,input->mesh());

    if (output.get_rep() == 0)
    {
      error("Could not allocate output field");
      algo_end(); return (false);
    }

    VField* ifield = input->vfield();
    VField* ofield = output->vfield();
    VMesh*  imesh  = input->vmesh();
    VMesh*  omesh  = output->vmesh();

    ofield->resize_values();

    const VMesh::size_type ni = imesh->get_ni();
    const VMesh::size_type nj = imesh->get_nj();
    const VMesh::size_type nk = imesh->get_nk();

    VMesh::index_type x0, x1, y0, y1, z0, z1;
    VMesh::Node::array_type nodes(6);
    VMesh::Node::index_type idx;

    double values[6];

    const Transform &transform = imesh->get_transform();

    for (VMesh::index_type k=0; k<nk; k++)
    {
      for (VMesh::index_type j=0; j<nj; j++)
      {
        for (VMesh::index_type i=0; i<ni; i++)
        {
          double xscale = 0.5;
          double yscale = 0.5;
          double zscale = 0.5;

          if (i > 0) { x0 = i-1; } else { x0 = i; xscale=1.0; }
          if (i < (ni-1)) { x1 = i+1; } else { x1 = i; xscale=1.0; }

          if (j > 0) { y0 = j-1; } else { y0 = j; yscale=1.0; }
          if (j < (nj-1)) { y1 = j+1; } else { y1 = j; yscale=1.0; }

          if (k > 0) { z0 = k-1; } else { z0 = k; zscale=1.0; }
          if (k < (nk-1)) { z1 = k+1; } else { z1 = k; zscale=1.0; }

          imesh->to_index(nodes[0],x0,j,k);
          imesh->to_index(nodes[1],x1,j,k);
          imesh->to_index(nodes[2],i,y0,k);
          imesh->to_index(nodes[3],i,y1,k);
          imesh->to_index(nodes[4],i,j,z0);
          imesh->to_index(nodes[5],i,j,z1);

          ifield->get_values(values,nodes);

          const Vector g((values[1] - values[0])*xscale,
          (values[3] - values[2])*yscale, (values[5] - values[4])*zscale);

          const Vector gradient = transform.project_normal(g);

          omesh->to_index(idx,i,j,k);
          ofield->set_value(gradient,idx);
        }
      }
    }
  }
  else if (fi.is_imagemesh())
  {
    fi.make_vector();
    output = CreateField(fi,input->mesh());

    if (output.get_rep() == 0)
    {
      error("Could not allocate output field");
      algo_end(); return (false);
    }

    VField* ifield = input->vfield();
    VField* ofield = output->vfield();
    VMesh*  imesh  = input->vmesh();
    VMesh*  omesh  = output->vmesh();

    ofield->resize_values();

    const VMesh::size_type ni = imesh->get_ni();
    const VMesh::size_type nj = imesh->get_nj();

    VMesh::index_type x0, x1, y0, y1;
    VMesh::Node::array_type nodes(4);
    VMesh::Node::index_type idx;

    double values[4];

    const Transform &transform = imesh->get_transform();

    for (VMesh::index_type j=0; j<nj; j++)
    {
      for (VMesh::index_type i=0; i<ni; i++)
      {
        double xscale = 0.5;
        double yscale = 0.5;

        if (i > 0) { x0 = i-1; } else { x0 = i; xscale=1.0; }
        if (i < (ni-1)) { x1 = i+1; } else { x1 = i; xscale=1.0; }

        if (j > 0) { y0 = j-1; } else { y0 = j; yscale=1.0; }
        if (j < (nj-1)) { y1 = j+1; } else { y1 = j; yscale=1.0; }


        imesh->to_index(nodes[0],x0,j);
        imesh->to_index(nodes[1],x1,j);
        imesh->to_index(nodes[2],i,y0);
        imesh->to_index(nodes[3],i,y1);

        ifield->get_values(values,nodes);

        const Vector g((values[1] - values[0])*xscale, (values[3] - values[2])*yscale, 0.0);
        const Vector gradient = transform.project_normal(g);

        omesh->to_index(idx,i,j);
        ofield->set_value(gradient,idx);
      }
    }
  }
  else
  {
    error("Unknown field type encountered");
    algo_end(); return (false);
  }

  algo_end(); return (true);
}

} // end namespace SCIRun
