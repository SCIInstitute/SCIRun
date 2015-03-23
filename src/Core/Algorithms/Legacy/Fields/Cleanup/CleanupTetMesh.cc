/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Fields/Cleanup/CleanupTetMesh.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
CleanupTetMeshAlgo::
run(FieldHandle input, FieldHandle& output)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("CleanupTetMesh");
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.
  
  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error. 
    error("No input field");
    algo_end(); return (false);
  }

  // Step 1: determine the type of the input fields and determine what type the
  // output field should be.

  FieldInformation fi(input);
  FieldInformation fo(input);
  // Here we test whether the class is part of any of these newly defined 
  // non-linear classes. If so we return an error.
  if (fi.is_nonlinear())
  {
    error("This algorithm has not yet been defined for non-linear elements yet");
    algo_end(); return (false);
  }

  // This one
  if (!(fi.is_tetvolmesh())) 
  {
    // Notify the user that no action is done  
    error("This algorithm only works on a TetVolMesh");
    // Copy input to output (output is a reference to the input)
    algo_end(); return (false);
  }


  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

  output = CreateField(fo);  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }
  
  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  bool fix_orientation = get_bool("fix_orientation");
  bool remove_degenerate = get_bool("remove_degenerate");

  omesh->copy_nodes(imesh);
  
  VMesh::Node::array_type nodes;
  VMesh::size_type num_elems = imesh->num_elems();
  std::vector<Point> points;
  
  int basis_order = ifield->basis_order();

  std::vector<VMesh::index_type> order;
  if (basis_order == 0) order.reserve(num_elems);
  
  int cnt =0;
  
  for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++, cnt++)
  {
    if (cnt == 200)
    {
      cnt = 0;
      update_progress(idx,num_elems);
    }
    
    imesh->get_nodes(nodes,idx);
    
    if (nodes.size() < 4) { continue; }

    if (nodes[0] == nodes[1] || nodes[0] == nodes[2] || nodes[0] == nodes[3] ||
        nodes[1] == nodes[2] || nodes[1] == nodes[3] || nodes[2] == nodes[3] )
    { // degenerate
      if (remove_degenerate) continue;
      // Cannot fix orientation on degenerate elements
      VMesh::index_type eidx = omesh->add_elem(nodes);
      if (basis_order == 0) order.push_back(eidx);
    }
    else
    {
      if (fix_orientation)
      {
        imesh->get_centers(points,nodes);
        if(Dot(Cross(points[1]-points[0],points[2]-points[0]),points[3]-points[0]) < 0.0)
        {
          VMesh::Node::index_type nidx = nodes[0]; nodes[0] = nodes[1]; nodes[1] = nidx;
        }
      }
      VMesh::index_type eidx = omesh->add_elem(nodes);
      if (basis_order == 0) order.push_back(eidx);
    }  
  }
  
  ofield->resize_values();
  if (basis_order == 0)
  {
    VField::size_type size = order.size();
    for(VField::index_type idx=0; idx<size; idx++)
    ofield->copy_value(ifield,order[idx],idx);
  }
  else if (basis_order == 1)
  {
    ofield->copy_values(ifield);
  }
    
  /// Copy properties of the property manager
	output->copy_properties(input.get_rep());
   
  // Success:
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
