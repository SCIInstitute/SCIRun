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

   Author:              Moritz Dannhauer
   Last Modification:   October 25 2017 (ported from SciRun4)
*/


#include <Core/Algorithms/Legacy/Fields/Cleanup/CleanupTetMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

CleanupTetMeshAlgo::CleanupTetMeshAlgo()
{
  addParameter(Parameters::FixOrientationCheckBox, true);
  addParameter(Parameters::RemoveDegenerateCheckBox, true);
}

AlgorithmInputName CleanupTetMeshAlgo::InputTetMesh("InputTetMesh");
AlgorithmOutputName CleanupTetMeshAlgo::OutputTetMesh("OutputTetMesh");

ALGORITHM_PARAMETER_DEF(Fields, FixOrientationCheckBox);
ALGORITHM_PARAMETER_DEF(Fields, RemoveDegenerateCheckBox);

bool CleanupTetMeshAlgo::run(FieldHandle input, FieldHandle& output) const
{
  FieldInformation fi(input);
  FieldInformation fo(input);

  if(fi.is_nonlinear())
  {
   error("This algorithm has not yet been defined for non-linear elements yet");
   return true;
  }

  if (!(fi.is_tetvolmesh()))
  {
    error("This algorithm only works on a TetVolMesh");
    return true;
  }

  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

  output = CreateField(fo);

  if (!output)
  {
    error("Could not allocate output field");
    return true;
  }


  if (!output)
  {
    error("Could not allocate output field");
    return true;
  }

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  bool fix_orientation = get(Parameters::FixOrientationCheckBox).toBool();
  bool remove_degenerate = get(Parameters::RemoveDegenerateCheckBox).toBool();

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
      this->update_progress(idx/num_elems);
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

 return true;
}

AlgorithmOutput CleanupTetMeshAlgo::run(const AlgorithmInput& input) const
{

  auto input_tet_mesh = input.get<Field>(InputTetMesh);
  FieldHandle output_mesh;
  AlgorithmOutput output;

  if(!input_tet_mesh)
  {
   error("No input field");
   return output;
  }

  if(!run(input_tet_mesh, output_mesh))
  {
    error("Algorithm failed!");
  }

  output[OutputTetMesh] = output_mesh;
  return output;

}
