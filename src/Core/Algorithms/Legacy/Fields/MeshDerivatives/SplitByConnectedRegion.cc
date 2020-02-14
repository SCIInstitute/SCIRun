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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/SplitByConnectedRegion.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

AlgorithmInputName SplitFieldByConnectedRegionAlgo::InputField("InputField");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField1("OutputField1");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField2("OutputField2");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField3("OutputField3");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField4("OutputField4");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField5("OutputField5");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField6("OutputField6");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField7("OutputField7");
AlgorithmOutputName SplitFieldByConnectedRegionAlgo::OutputField8("OutputField8");

AlgorithmParameterName SplitFieldByConnectedRegionAlgo::SortDomainBySize() { return AlgorithmParameterName("SortDomainBySize"); }
AlgorithmParameterName SplitFieldByConnectedRegionAlgo::SortAscending() { return AlgorithmParameterName("SortAscending"); }

/// TODO: These should be refactored to hold const std::vector<double>& rather than double*
class SortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    SortSizes(double* sizes) : sizes_(sizes) {}

    bool operator()(index_type i1, index_type i2)
    {
      return (sizes_[i1] > sizes_[i2]);
    }

  private:
    double*      sizes_;
};
/// TODO: These should be refactored to hold const std::vector<double>& rather than double*
class AscSortSizes : public std::binary_function<index_type,index_type,bool>
{
  public:
    AscSortSizes(double* sizes) : sizes_(sizes) {}

    bool operator()(index_type i1, index_type i2)
    {
      return (sizes_[i1] < sizes_[i2]);
    }

  private:
    double*      sizes_;
};

SplitFieldByConnectedRegionAlgo::SplitFieldByConnectedRegionAlgo()
{
  addParameter(SortDomainBySize(), false);
  addParameter(SortAscending(), false);
}

std::vector<FieldHandle> SplitFieldByConnectedRegionAlgo::run(FieldHandle input) const
{
 bool sortDomainBySize = get(SortDomainBySize()).toBool();
 bool sortAscending = get(SortAscending()).toBool();

 if (!input)
 {
      THROW_ALGORITHM_INPUT_ERROR("Input mesh is empty.");
 }

 std::vector<FieldHandle> output;

   /// Figure out what the input type and output type have to be
  FieldInformation fi(input);

  /// We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    THROW_ALGORITHM_INPUT_ERROR("This function has not yet been defined for non-linear elements.");
  }

  if (!(fi.is_unstructuredmesh()))
  {
    output.push_back(input);
    remark("Structured meshes consist always of one piece. Hence there is no algorithm to perform.");
  }

  if (fi.is_pointcloudmesh())
  {
    THROW_ALGORITHM_INPUT_ERROR("This algorithm has not yet been defined for point clouds.");
  }

  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

  VField::index_type k = 0;

  VMesh::size_type num_nodes = imesh->num_nodes();
  VMesh::size_type num_elems = imesh->num_elems();

  VField::size_type surfsize = static_cast<VField::size_type>(pow(num_elems, 2.0 / 3.0));
  std::vector<VMesh::Elem::index_type> buffer;
  buffer.reserve(surfsize);

  imesh->synchronize(Mesh::NODE_NEIGHBORS_E|Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E);

  std::vector<index_type> elemmap(num_elems, 0);
  std::vector<index_type> nodemap(num_nodes, 0);
  std::vector<index_type> renumber(num_nodes,0);
  std::vector<short> visited(num_elems, 0);

  VMesh::Node::array_type nnodes;
  VMesh::Elem::array_type neighbors;

  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    // if list of elements to process is empty ad the next one
    if (buffer.size() == 0)
    {
      if(visited[idx] == 0) { buffer.push_back(idx); k++; }
    }

    if (buffer.size() > 0)
    {
      for (size_t i=0; i< buffer.size(); i++)
      {
        VField::index_type j = buffer[i];
        if (visited[j] > 0) { continue; }
        visited[j] = 1;

        imesh->get_nodes(nnodes,buffer[i]);
        for (size_t q=0; q<nnodes.size(); q++)
        {
          imesh->get_elems(neighbors,nnodes[q]);
          for (size_t p=0; p<neighbors.size(); p++)
          {
            if(visited[neighbors[p]] == 0)
            {
              buffer.push_back(neighbors[p]);
              visited[neighbors[p]] = -1;
            }
          }
        }

        if (j >= static_cast<index_type>(elemmap.size())) elemmap.resize(j+1);

        elemmap[j] = k;
        for (size_t p=0;p<nnodes.size();p++)
        {
          if (static_cast<size_t>(nnodes[p]) >= nodemap.size())
            nodemap.resize(nnodes[p]+1);
          nodemap[nnodes[p]] = k;
        }
      }
      buffer.clear();
    }
  }

 output.resize(k);
  for (size_type p=0; p<k; p++)
  {
    VField* ofield;
    VMesh* omesh;
    MeshHandle mesh;
    FieldHandle field;

    VField::size_type nn = 0;
    VField::size_type ne = 0;

    for (VField::index_type q=0;q<num_nodes;q++) if (nodemap[q] == p+1) nn++;
    for (VField::index_type q=0;q<num_elems;q++) if (elemmap[q] == p+1) ne++;

    mesh = CreateMesh(fi);
    if (!mesh)
    {
      THROW_ALGORITHM_INPUT_ERROR("Could not create output field.");
    }
    omesh = mesh->vmesh();

    omesh->node_reserve(nn);
    omesh->elem_reserve(ne);

    field = CreateField(fi,mesh);
    if (field == nullptr)
    {
      THROW_ALGORITHM_INPUT_ERROR("Could not create output field");
    }

    ofield = field->vfield();
    output[p] = field;

    Point point;
    for (index_type q=0;q<num_nodes;q++)
    {
      if (nodemap[q] == p+1)
      {
        imesh->get_center(point,VMesh::Node::index_type(q));
        renumber[q] = omesh->add_point(point);
      }
    }

    VMesh::Node::array_type elemnodes;
    for (index_type q=0;q<num_elems;q++)
    {
      if (elemmap[q] == p+1)
      {
        imesh->get_nodes(elemnodes,VMesh::Elem::index_type(q));
        for (size_t r=0; r< elemnodes.size(); r++)
        {
          elemnodes[r] = VMesh::Node::index_type(renumber[elemnodes[r]]);
        }
        omesh->add_elem(elemnodes);
      }
    }

    ofield->resize_fdata();

    if (ifield->basis_order() == 1)
    {
      VField::index_type qq = 0;
      for (VField::index_type q=0;q<num_nodes;q++)
      {
        if (nodemap[q] == p+1)
        {
          ofield->copy_value(ifield,q,qq); qq++;
        }
      }
    }

    if (ifield->basis_order() == 0)
    {
      VField::index_type qq = 0;
      for (VField::index_type q=0;q<num_elems;q++)
      {
        if (elemmap[q] == p+1)
        {
          ofield->copy_value(ifield,q,qq); qq++;
        }
      }
    }

   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    ofield->copy_properties(ifield);
   #endif
  }

  if (sortDomainBySize)
  {
    std::vector<double> sizes(output.size());
    std::vector<index_type> order(output.size());
    std::vector<FieldHandle> temp(output.size());

    for (size_t j=0; j<output.size(); j++)
    {
      VMesh* mesh = output[j]->vmesh();
      VMesh::Elem::size_type num_elems = mesh->num_elems();
      double size = 0.0;
      for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
      {
        size += mesh->get_size(idx);
      }
      sizes[j] = size;
      order[j] = j;
      temp[j] = output[j];
    }

    if (!sizes.empty())
    {
      if (sortAscending)
      {
        std::sort(order.begin(), order.end(), AscSortSizes(&(sizes[0])));
      }
      else
      {
        std::sort(order.begin(), order.end(), SortSizes(&(sizes[0])));
      }
    }

    for (size_t j=0; j<output.size(); j++)
    {
      output[j] = temp[order[j]];
    }
  }

 return output;
}


AlgorithmOutput SplitFieldByConnectedRegionAlgo::run(const AlgorithmInput& input) const
{
 AlgorithmOutput output;

 auto mesh_ = input.get<Field>(Variables::InputField);

 if (!mesh_)
     THROW_ALGORITHM_INPUT_ERROR("Input mesh is empty.");

 std::vector<FieldHandle> output_fields=run(mesh_);

 if (output_fields.empty())
  {
    THROW_ALGORITHM_INPUT_ERROR(" No input fields given ");
  }

  if (output_fields.size()>8)
  {
    remark(" Not all output meshes could be sent to the eight field output ports. ");
  }

 if (output_fields.size() > 0)
    output[OutputField1]=output_fields[0];
 if (output_fields.size() > 1)
    output[OutputField2]=output_fields[1];
 if (output_fields.size() > 2)
    output[OutputField3]=output_fields[2];
 if (output_fields.size() > 3)
    output[OutputField4]=output_fields[3];
 if (output_fields.size() > 4)
    output[OutputField5]=output_fields[4];
 if (output_fields.size() > 5)
    output[OutputField6]=output_fields[5];
 if (output_fields.size() > 6)
    output[OutputField7]=output_fields[6];
 if (output_fields.size() > 7)
    output[OutputField8]=output_fields[7];

 /// TODO: enable dynamic output ports

 return output;
}
