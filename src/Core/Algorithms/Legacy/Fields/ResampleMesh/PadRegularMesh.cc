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


#include <Core/Algorithms/Fields/ResampleMesh/PadRegularMesh.h>

/// For mapping matrices
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

///////////////////////////////////////////////////////
// Refine elements for a TetVol

bool
PadRegularMeshAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("PadRegularMesh");

  if (input.get_rep() == 0)
  {
    error("No input field was given.");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  if ((!(fi.is_latvolmesh()))&&(!(fi.is_imagemesh()))&&(!(fi.is_scanlinemesh())))
  {
    error("This algorithm only operates on regular grids");
    algo_end(); return (false);
  }

  VMesh::dimension_type dims, newdims;

  VMesh*  vmesh  = input->vmesh();

  Transform trans;
  vmesh->get_canonical_transform(trans);
  vmesh->get_dimensions(dims);
  vmesh->get_dimensions(newdims);

  int padsize = get_int("pad-size");

  if (padsize < 0)
  {
    error("Negative pads are currently not supported");
    algo_end(); return (false);
  }

  MeshHandle mesh;

  for(size_t k=0;k<newdims.size();k++) newdims[k] = dims[k] + 2*(padsize);

  if(dims.size() == 1)
  {
    trans.post_translate(-0.5*Vector(static_cast<double>(newdims[0]-dims[0])/static_cast<double>(dims[0]-1),0.0,0.0));
    trans.post_scale(Vector(static_cast<double>(newdims[0]-1)/static_cast<double>(dims[0]-1),0.0,0.0));
    mesh = CreateMesh(fi,newdims[0],Point(0.0,0.0,0.0),Point(1.0,0.0,0.0));
    if (!(mesh.get_rep()))
    {
      error("Could not allocate output mesh");
      algo_end(); return (false);
    }
    mesh->vmesh()->transform(trans);
  }
  else if(dims.size() == 2)
  {
    trans.post_translate(-0.5*Vector(static_cast<double>(newdims[0]-dims[0])/static_cast<double>(dims[0]-1),static_cast<double>(newdims[1]-dims[1])/static_cast<double>(dims[1]-1),0.0));
    trans.post_scale(Vector(static_cast<double>(newdims[0]-1)/static_cast<double>(dims[0]-1),static_cast<double>(newdims[1]-1)/static_cast<double>(dims[1]-1),0.0));
    mesh = CreateMesh(fi,newdims[0],newdims[1],Point(0.0,0.0,0.0),Point(1.0,1.0,0.0));
    if (!(mesh.get_rep()))
    {
      error("Could not allocate output mesh");
      algo_end(); return (false);
    }
    mesh->vmesh()->transform(trans);
  }
  else if(dims.size() == 3)
  {
    trans.post_translate(-0.5*Vector(static_cast<double>(newdims[0]-dims[0])/static_cast<double>(dims[0]-1),static_cast<double>(newdims[1]-dims[1])/static_cast<double>(dims[1]-1),static_cast<double>(newdims[2]-dims[2])/static_cast<double>(dims[2]-1)));
    trans.post_scale(Vector(static_cast<double>(newdims[0]-1)/static_cast<double>(dims[0]-1),static_cast<double>(newdims[1]-1)/static_cast<double>(dims[1]-1),static_cast<double>(newdims[2]-1)/static_cast<double>(dims[2]-1)));
    mesh = CreateMesh(fi,newdims[0],newdims[1],newdims[2],Point(0.0,0.0,0.0),Point(1.0,1.0,1.0));
    if (!(mesh.get_rep()))
    {
      error("Could not allocate output mesh");
      algo_end(); return (false);
    }
    mesh->vmesh()->transform(trans);
  }
  else
  {
    error("internal error: number of dimensions is not 1,2, or 3");
    algo_end(); return(false);
  }

  output = CreateField(fi,mesh);

  if (!(output.get_rep()))
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh*  omesh  = output->vmesh();
  VMesh*  imesh  = input->vmesh();

  if (ifield->basis_order() == 0)
  {
    VMesh::Elem::index_type src;
    VMesh::Elem::index_type dst;

    ofield->set_all_values(get_scalar("pad-value"));

    if (dims.size() == 3)
    {
      for (VField::index_type k=0;k<(dims[2]-1);k++)
      {
        for (VField::index_type j=0;j<(dims[1]-1);j++)
        {
          for (VField::index_type i=0;i<(dims[0]-1);i++)
          {
            imesh->to_index(src,i,j,k);
            omesh->to_index(dst,i+padsize,j+padsize,k+padsize);
            ofield->copy_value(ifield,src,dst);
          }
        }
      }
    }
    else if (dims.size() == 2)
    {
      for (VField::index_type j=0;j<(dims[1]-1);j++)
      {
        for (VField::index_type i=0;i<(dims[0]-1);i++)
        {
          imesh->to_index(src,i,j);
          omesh->to_index(dst,i+padsize,j+padsize);
          ofield->copy_value(ifield,src,dst);
        }
      }
    }
    else if (dims.size() == 1)
    {
      for (VField::index_type i=0;i<(dims[0]-1);i++)
      {
        ofield->copy_value(ifield,i,i+padsize);
      }
    }
  }
  else if (ifield->basis_order() > 0)
  {
    VMesh::Node::index_type src;
    VMesh::Node::index_type dst;

    ofield->set_all_values(get_scalar("pad-value"));

    if (dims.size() == 3)
    {
      for (VField::index_type k=0;k<dims[2];k++)
      {
        for (VField::index_type j=0;j<dims[1];j++)
        {
          for (VField::index_type i=0;i<dims[0];i++)
          {
            imesh->to_index(src,i,j,k);
            omesh->to_index(dst,i+padsize,j+padsize,k+padsize);
            ofield->copy_value(ifield,src,dst);
          }
        }
      }
    }
    else if (dims.size() == 2)
    {
      for (VField::index_type j=0;j<dims[1];j++)
      {
        for (VField::index_type i=0;i<dims[0];i++)
        {
          imesh->to_index(src,i,j);
          omesh->to_index(dst,i+padsize,j+padsize);
          ofield->copy_value(ifield,src,dst);
        }
      }
    }
    else if (dims.size() == 1)
    {
      for (VField::index_type i=0;i<dims[0];i++)
      {
        ofield->copy_value(ifield,i,i+padsize);
      }
    }
  }

std::cerr << "2\n";


  algo_end(); return (true);
}

} // namespace
