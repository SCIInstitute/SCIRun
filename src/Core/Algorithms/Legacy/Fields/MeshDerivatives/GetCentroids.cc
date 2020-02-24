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


#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetCentroids.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <boost/unordered_map.hpp>
#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Centroids);

GetCentroids::GetCentroids()
{
  addOption(Parameters::Centroids,"Element","Node|Edge|Face|Cell|Element|DElement");
}

AlgorithmOutput GetCentroids::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  FieldHandle output;
  if (!inputField)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input field");
  }

  FieldInformation fo(inputField);
  fo.make_pointcloudmesh();
  fo.make_nodata();

  std::string centroids;
  centroids=getOption(Parameters::Centroids);

  VMesh* imesh = inputField->vmesh();
  MeshHandle mesh = CreateMesh(fo);
  VMesh* omesh = mesh->vmesh();

  AlgorithmOutput outputField;

  if (centroids=="Element")
  {
    imesh->synchronize(Mesh::ELEMS_E);
    VField::size_type num_elems = imesh->num_elems();
    omesh->reserve_nodes(num_elems);
    for (VMesh::Elem::index_type idx=0; idx < num_elems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  else if (centroids=="Node")
  {
    imesh->synchronize(Mesh::NODES_E);
    VMesh::size_type num_nodes = imesh->num_nodes();
    omesh->reserve_nodes(num_nodes);
    for (VMesh::Node::index_type idx=0; idx < num_nodes; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  else if (centroids=="Edge")
  {
    imesh->synchronize(Mesh::EDGES_E);

    VMesh::size_type num_edges = imesh->num_edges();
    omesh->reserve_nodes(num_edges);
    for (VMesh::Edge::index_type idx=0; idx < num_edges; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  else if (centroids=="Face")
  {
    imesh->synchronize(Mesh::FACES_E);
    VMesh::Face::size_type num_faces;
    imesh->size(num_faces);
    omesh->reserve_nodes(num_faces);
    for (VMesh::Face::index_type idx=0; idx < num_faces; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  else if (centroids=="Cell")
  {
    imesh->synchronize(Mesh::CELLS_E);
    VMesh::size_type num_cells = imesh->num_cells();
    omesh->reserve_nodes(num_cells);
    for (VMesh::Cell::index_type idx=0; idx < num_cells; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }
    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  else if (centroids=="DElement")
  {
    imesh->synchronize(Mesh::DELEMS_E);

    VMesh::size_type num_delems = imesh->num_delems();
    omesh->reserve_nodes(num_delems);

    for (VMesh::DElem::index_type idx=0; idx < num_delems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();

    if(omesh->num_nodes()==0)
    {
      warning("No. of nodes equal to zero! Empty matrix will get generated");
    }
  }
  outputField[Variables::OutputField] = output;
  return outputField;
}
