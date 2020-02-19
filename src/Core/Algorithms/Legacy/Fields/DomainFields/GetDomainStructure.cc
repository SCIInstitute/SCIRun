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


#include <Core/Algorithms/Fields/DomainFields/GetDomainStructure.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
GetDomainStructureAlgo::
run( FieldHandle input, FieldHandle& sepsurfaces,
             FieldHandle& sepedges, FieldHandle& seppoints)
{
  algo_start("GetDomainStructure");

  if (input.get_rep() == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  FieldInformation fi(input), fo(input);
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements.");
    algo_end(); return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("This function only works for data located at the elements.");
    algo_end(); return (false);
  }

  if (fi.is_prismvolmesh())
  {
    error("Prism meshes are not supported by this function.");
    algo_end(); return (false);
  }

  if (!(fi.is_volume()||fi.is_surface()))
  {
    error("This function is only defined for volumes or surfaces.");
    algo_end(); return (false);
  }

  std::string indextype = "int";
  if (sizeof(index_type) == 8) indextype = "long_long";

  if (fi.is_volume())
  {
    VField *field = input->vfield();
    VMesh  *mesh  = input->vmesh();
    mesh->synchronize(Mesh::DELEMS_E);

    VMesh::size_type num_elems = mesh->num_elems();
    VMesh::size_type num_delems = mesh->num_delems();
    VMesh::size_type num_nodes = mesh->num_nodes();
    std::vector<index_type> nodemap(num_nodes,-1);

    if (fi.is_hex_element())
    {
      FieldInformation f("QuadSurfMesh",1,indextype);
      sepsurfaces = CreateField(f);
    }
    else
    {
      FieldInformation f("TriSurfMesh",1,indextype);
      sepsurfaces = CreateField(f);
    }

    VMesh::Elem::array_type elems;
    VMesh::Node::array_type nodes;
    std::vector<int> values;
    std::vector<index_type> nodeindices;

    VMesh* omesh = sepsurfaces->vmesh();
    VField* ofield = sepsurfaces->vfield();

    for (VMesh::DElem::index_type idx=0; idx<num_delems; idx++)
    {
      mesh->get_elems(elems,idx);
      field->get_values(values,elems);
      if ((elems.size() == 1)||(elems.size() == 2 && values[0] != values[1]))
      {
        mesh->get_nodes(nodes,idx);
        for (size_t j=0; j<nodes.size(); j++)
        {
          if (nodemap[nodes[j]] < 0)
          {
            Point p; mesh->get_center(p,nodes[j]);
            nodemap[nodes[j]] = omesh->add_node(p);
            nodeindices.push_back(nodes[j]);
          }
          nodes[j] = nodemap[nodes[j]];
        }
        omesh->add_elem(nodes);
      }
    }

    ofield->resize_values();
    ofield->set_values(nodeindices);

    field = sepsurfaces->vfield();
    mesh  = sepsurfaces->vmesh();
    mesh->synchronize(Mesh::DELEMS_E);

    num_elems = mesh->num_elems();
    num_delems = mesh->num_delems();
    num_nodes = mesh->num_nodes();
    nodemap.clear();
    nodemap.resize(num_nodes,-1);
    nodeindices.clear();

    FieldInformation f2("CurveMesh",1,indextype);
    sepedges = CreateField(f2);

    omesh = sepedges->vmesh();
    ofield = sepedges->vfield();

    for (VMesh::DElem::index_type idx=0; idx<num_delems; idx++)
    {
      mesh->get_elems(elems,idx);
      if (elems.size() != 2)
      {
        mesh->get_nodes(nodes,idx);
        for (size_t j=0; j<nodes.size(); j++)
        {
          if (nodemap[nodes[j]] < 0)
          {
            Point p; mesh->get_center(p,nodes[j]);
            nodemap[nodes[j]] = omesh->add_node(p);
            index_type index;
            field->get_value(index,nodes[j]);
            nodeindices.push_back(index);
          }
          nodes[j] = nodemap[nodes[j]];
        }
        omesh->add_elem(nodes);
      }
    }

    ofield->resize_values();
    ofield->set_values(nodeindices);

    field = sepedges->vfield();
    mesh  = sepedges->vmesh();
    mesh->synchronize(Mesh::DELEMS_E|Mesh::NODE_NEIGHBORS_E);

    num_elems = mesh->num_elems();
    num_delems = mesh->num_delems();
    num_nodes = mesh->num_nodes();
    nodemap.clear();
    nodemap.resize(num_nodes,-1);
    nodeindices.clear();

    FieldInformation f3("PointCloudMesh",1,indextype);
    seppoints = CreateField(f3);

    omesh = seppoints->vmesh();
    ofield = seppoints->vfield();

    for (VMesh::DElem::index_type idx=0; idx<num_delems; idx++)
    {
      mesh->get_elems(elems,idx);
      if (elems.size() != 2)
      {
        mesh->get_nodes(nodes,idx);
        for (size_t j=0; j<nodes.size(); j++)
        {
          if (nodemap[nodes[j]] < 0)
          {
            Point p; mesh->get_center(p,nodes[j]);
            nodemap[nodes[j]] = omesh->add_node(p);
            index_type index;
            field->get_value(index,nodes[j]);
            nodeindices.push_back(index);
          }
          nodes[j] = nodemap[nodes[j]];
        }
        omesh->add_elem(nodes);
      }
    }
    ofield->set_values(nodeindices);
  }

  if (fi.is_surface())
  {
    error ("Method has not been implemented yet for surfaces");
    algo_end(); return (false);
  }


  algo_end(); return(true);
}

} // namespace SCIRunAlgo
