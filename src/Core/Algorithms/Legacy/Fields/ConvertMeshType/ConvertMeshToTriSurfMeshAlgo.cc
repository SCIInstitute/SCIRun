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


#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

bool ConvertMeshToTriSurfMeshAlgo::run(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ConvertMeshToTriSurfMesh");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  // Create information fields and fill them out with the data types of the input
  FieldInformation fi(input);
  FieldInformation fo(input);

  // Ignore non linear cases for now
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  // In case it is already a trisurf skip algorithm
  if (fi.is_tri_element())
  {
    output = input;
    remark("Input is already a TriSurfMesh; just copying input to output");
    return (true);
  }

  // Only quads we know how to process, return an error for any other type
  if (!fi.is_quad_element())
  {
    error("This function has been defined for quadrilateral elements only");
    return (false);
  }

  // Fill out the output type by altering the mesh type
  // Everything else stays the same
  fo.make_trisurfmesh();

  // Create the output field
  output = CreateField(fo);

  // If creation fails return an error to the user
  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  // Algorithm starts here

  // Get Virtual interface classes:
  VMesh*  imesh  = input->vmesh();
  VMesh*  omesh  = output->vmesh();
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();

  // Get the number of nodes and elements in the input mesh
  VMesh::size_type num_nodes = imesh->num_nodes();
  VMesh::size_type num_elems = imesh->num_elems();

  // Copy all the nodes
  for (VMesh::Node::index_type i=0; i<num_nodes; i++)
  {
    Point p;
    imesh->get_center(p,i);
    omesh->add_node(p,i);
  }

  // Record which element index to use for filling out data
  std::vector<VMesh::size_type> elemmap(num_elems);

  // If it is a structured mesh use an alternating scheme to get a better mesh
  if (fi.is_image()|| fi.is_structquadsurf())
  {
    // Reserve two arrays to split quadrilateral
    VMesh::Node::array_type tri1(3), tri2(3);
    VMesh::dimension_type dim;
    VMesh::Node::array_type nodes;

    // Get the structured dimensions, dimension_type has always three elements
    // But here only the first two will be filled out
    imesh->get_dimensions(dim);

    // Loop over each element and split the quadrilaterals in two triangles
    for (VMesh::Elem::index_type i=0; i<num_elems; i++)
    {
      // Get the nodes that make up a quadrilateral.
      // Even for degenerate elements for nodes will be returned
      imesh->get_nodes(nodes,i);

      // Figure out the red-black scheme
      auto jj = i/dim[1];
      auto ii = i%dim[1];

      if ((ii^jj)&1)
      {
        // Splitting option 1
        tri1[0] = nodes[0]; tri1[1] = nodes[1]; tri1[2] = nodes[2];
        tri2[0] = nodes[2]; tri2[1] = nodes[3]; tri2[2] = nodes[0];
      }
      else
      {
        // SPlitting option 2
        tri1[0] = nodes[1]; tri1[1] = nodes[2]; tri1[2] = nodes[3];
        tri2[0] = nodes[3]; tri2[1] = nodes[0]; tri2[2] = nodes[1];
      }
      elemmap[i] = 2;

      // Add the elements to the output mesh
      omesh->add_elem(tri1);
      omesh->add_elem(tri2);
    }
  }
  else
  {
    // Alternative scheme: unstructured so we just split each quadrilateral
    VMesh::Node::array_type tri1(3), tri2(3);
    VMesh::Node::array_type nodes;

    for (VMesh::Elem::index_type i=0; i<num_elems; i++)
    {
      imesh->get_nodes(nodes,i);

      tri1[0] = nodes[0]; tri1[1] = nodes[1]; tri1[2] = nodes[2];
      tri2[0] = nodes[2]; tri2[1] = nodes[3]; tri2[2] = nodes[0];


      // Check for degenerate elements and record how many elements we are adding
      if (tri1[0]==tri1[1] || tri1[1]==tri1[2] || tri1[0]==tri1[2])
      {
         if (!(tri2[0]==tri2[1] || tri2[1]==tri2[2] || tri2[0]==tri2[2]))
         {
            omesh->add_elem(tri2);
            elemmap[i] = 1;
         }
         else
         {
            elemmap[i] = 0;
         }
      }
      else if (tri2[0]==tri2[1] || tri2[1]==tri2[2] || tri2[0]==tri2[2])
      {
        omesh->add_elem(tri1);
        elemmap[i] = 1;
      }
      else
      {
        omesh->add_elem(tri1);
        omesh->add_elem(tri2);
        elemmap[i] = 2;
      }
    }
  }

  ofield->resize_fdata();

  if (ifield->basis_order() == 1)
  {
    // Copy all the data
    ofield->copy_values(ifield);
  }
  else if (ifield->basis_order() == 0)
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type i=0; i<num_elems; i++)
    {
      if (elemmap[i] == 1)
      {
        ofield->copy_value(ifield,i,k); k++;
      }
      else if (elemmap[i] == 2)
      {
        ofield->copy_value(ifield,i,k); k++;
        ofield->copy_value(ifield,i,k); k++;
      }
    }
  }

  CopyProperties(*input, *output);

  // Success:
  return (true);
}

AlgorithmInputName ConvertMeshToTriSurfMeshAlgo::QuadSurf("QuadSurf");
AlgorithmOutputName ConvertMeshToTriSurfMeshAlgo::TriSurf("TriSurf");

AlgorithmOutput ConvertMeshToTriSurfMeshAlgo::run(const AlgorithmInput& input) const
{
  auto quad = input.get<Field>(QuadSurf);

  FieldHandle tri;
  if (!run(quad, tri))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[TriSurf] = tri;
  return output;
}
