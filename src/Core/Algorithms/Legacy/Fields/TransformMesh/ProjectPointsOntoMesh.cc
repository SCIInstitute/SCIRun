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

#include <Core/Algorithms/Fields/TransformMesh/ProjectPointsOntoMesh.h>
#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/Field.h>

ProjectPointsOntoMeshAlgo::ProjectPointsOntoMeshAlgo()
{
  // Are we projecting points on thre nodes or on the elements
  add_option("method","nodes","elements|nodes");
}

namespace SCIRunAlgo {

bool ProjectPointsOntoMeshAlgo::run(FieldHandle input, FieldHandle object, 
                                    FieldHandle& output)
{
  algo_start("ProjectPointsOntoMesh");
  std::string method = get_option("method");

  if (input.get_rep() == 0)
  {
    error("No input field.");
    algo_end(); return (false);
  }

  if (object.get_rep() == 0)
  {
    error("No mesh to project points onto.");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input), fo(input), fobj(object);

  // Create the proper output type
  if (fi.is_crv_element()) fo.make_curvemesh();
  if (fi.is_quad_element()) fo.make_quadsurfmesh();
  if (fi.is_hex_element()) fo.make_hexvolmesh();
  
  if ((!(fobj.is_surface()||fobj.is_volume()||fobj.is_curve()))&&(method=="elements"))
  {
    error("The object field needs to be a curve, a surface or a volume.");
    algo_end(); return (false);    
  }
  
  MeshHandle outputmesh = CreateMesh(fo);
  if (outputmesh.get_rep() == 0)
  {
    error("Could not create output mesh.");
    algo_end(); return (false);    
  }
  
  output = CreateField(fo,outputmesh);
  if (output.get_rep() == 0)
  {
    error("Could not create output field.");
    algo_end(); return (false);    
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh* objmesh = object->vmesh();
  
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  omesh->reserve_nodes(imesh->num_nodes());
  
  if ((method == "elements") && (fobj.is_volume()))
  {
    objmesh->synchronize(Mesh::ELEM_LOCATE_E);
    FieldHandle surffield;

    GetFieldBoundaryAlgo algo;
    algo.set_progress_reporter(get_progress_reporter());
    if(!(algo.run(object,surffield)))
    {
      error("Could not compute boundary field.");
      algo_end(); return (false);    
    }
    
    VMesh* smesh = surffield->vmesh();
    
    VMesh::size_type numnodes = imesh->num_nodes();    
    VMesh::index_type count = 0;    
    for (VMesh::Node::index_type i=0; i< numnodes; i++)
    {
      Point p;
      imesh->get_center(p,i);
      VMesh::Elem::index_type eidx;
      if (objmesh->locate(eidx,p))
      {
        // it is inside the volume
        omesh->add_point(p);
      }
      else
      {
        Point r;
        smesh->find_closest_elem(r,eidx,p);
        omesh->add_point(r);
      }
      count++; if (count == 100) { update_progress(i,numnodes); count=0; }
    }
  }
  else if ((method == "elements"  && (fobj.is_surface()||fobj.is_curve())))
  {
    objmesh->synchronize(Mesh::ELEM_LOCATE_E);

    VMesh::size_type numnodes = imesh->num_nodes();

    VMesh::index_type count = 0;    
    for (VMesh::Node::index_type i=0; i< numnodes; i++)
    {
      Point p, r;
      imesh->get_center(p,i);
      VMesh::Elem::index_type eidx;
      objmesh->find_closest_elem(r,eidx,p);
      omesh->add_point(r);
      count++; if (count == 100) { update_progress(i,numnodes); count=0; }
    }
  }
  else if (method == "nodes")
  {
    objmesh->synchronize(Mesh::NODE_LOCATE_E);

    VMesh::size_type numnodes = imesh->num_nodes();

    VMesh::index_type count = 0;    
    for (VMesh::Node::index_type i=0; i< numnodes; i++)
    {
      Point p, r;
      imesh->get_center(p,i);
      VMesh::Node::index_type nidx;
      objmesh->locate(nidx,p);
      objmesh->get_center(r,nidx);
      omesh->add_point(r);
      count++; if (count == 100) { update_progress(i,numnodes); count=0; }
    }
  }
  else
  {
    error("Invalid method for projection.");
    algo_end(); return (false);
  }
  
  VMesh::size_type numelems = imesh->num_elems();
  omesh->reserve_elems(numelems);
  for (VMesh::Elem::index_type i=0; i<numelems; i++)
  {
    VMesh::Node::array_type nodes;
    imesh->get_nodes(nodes,i);
    omesh->add_elem(nodes);
  }
  
  // Copy values if any are stored
  if (ifield->basis_order() > -1)
  {
    ofield->resize_values();
    ofield->copy_values(ifield);
  }
  
  algo_end(); return (true); 
}

} // end namespace
