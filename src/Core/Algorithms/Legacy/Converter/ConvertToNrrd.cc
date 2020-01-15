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


#include <Core/Algorithms/Legacy/Converter/ConvertToNrrd.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Algorithms/Legacy/Converter/ConverterAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Converters;

ALGORITHM_PARAMETER_DEF(Converters, BuildPoints);
ALGORITHM_PARAMETER_DEF(Converters, BuildConnections);
ALGORITHM_PARAMETER_DEF(Converters, BuildData);
ALGORITHM_PARAMETER_DEF(Converters, DataLabel);

ConvertToNrrdAlgo::ConvertToNrrdAlgo()
{
  addParameter(Parameters::BuildPoints, true);
  addParameter(Parameters::BuildConnections, true);
  addParameter(Parameters::BuildData, true);
  addParameter(Parameters::DataLabel, std::string("unknown"));
}

bool
ConvertToNrrdAlgo::runImpl(FieldHandle input, NrrdDataHandle& points,
             NrrdDataHandle& connections,NrrdDataHandle& data) const
{
  ScopedAlgorithmStatusReporter asr(this, "ConvertToNrrd");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  VField* field = input->vfield();
  VMesh* mesh = input->vmesh();

  std::vector<size_type> dataDims;
  std::vector<size_type> ndims;
  Vector spc;
  Point minP, maxP;
  bool with_spacing = true;

  if (mesh->is_regularmesh())
  {
    VMesh::dimension_type dims;
    mesh->get_dimensions(dims);
    for (size_t j=0;j<dims.size(); j++) ndims.push_back(dims[j]);

    // regular
    BBox bbox = mesh->get_bounding_box();
    minP = bbox.get_min();
    maxP = bbox.get_max();
    spc = maxP - minP;
    if (field->basis_order() == 0)
    {
      spc.x(spc.x() / ndims[0]);
      if (ndims.size() > 1)
      {
        spc.y(spc.y() / ndims[1]);
      }
      if (ndims.size() > 2)
      {
        spc.z(spc.z() / ndims[2]);
      }
    }
    else
    {
      spc.x(spc.x() / (ndims[0] - 1));
      if (ndims.size() > 1)
      {
        spc.y(spc.y() / (ndims[1] - 1));
      }
      if (ndims.size() > 2)
      {
        spc.z(spc.z() / (ndims[2] - 1));
      }
    }
    dataDims = ndims;
    if (field->basis_order() == 0)
    {
      for (size_t i = 0; i < dataDims.size(); i++)
      {
        dataDims[i]--;
      }
    }
  }
  else
  {
    // unstructured data so create a 1D nrrd (2D if vector or tensor)
    size_type nsz = 0;
    size_type sz = 0;
    VMesh::Node::size_type size;
    mesh->synchronize(Mesh::NODES_E);
    mesh->size(size);
    nsz = size;

    switch(field->basis_order())
    {
      case 1 :
        sz = nsz;
        break;
      case 0:
        sz = mesh->num_elems();
        break;
    }

    ndims.push_back(nsz);
    dataDims.push_back(sz);

    with_spacing = false;
  }

  if (get(Parameters::BuildPoints).toBool())
  {
    points.reset(new NrrdData());
    switch(ndims.size())
    {
      case 1:
      {
        size_t size[NRRD_DIM_MAX];
        size[0] = 3;
        size[1] = ndims[0];
        nrrdAlloc_nva(points->getNrrd(), nrrdTypeDouble, 2, size);
      }
      break;
      case 2:
      {
        size_t size[NRRD_DIM_MAX];
        size[0] = 3;
        size[1] = ndims[0];
        size[2] = ndims[1];
        nrrdAlloc_nva(points->getNrrd(), nrrdTypeDouble, 3, size);
      }
      break;
      case 3:
      {
        size_t size[NRRD_DIM_MAX];
        size[0] = 3;
        size[1] = ndims[0];
        size[2] = ndims[1];
        size[3] = ndims[2];
        nrrdAlloc_nva(points->getNrrd(), nrrdTypeDouble, 4, size);
      }
      break;
    }

    double *data = reinterpret_cast<double*>(points->getNrrd()->data);
    VMesh::size_type num_nodes = mesh->num_nodes();
    for (VMesh::Node::index_type idx = 0; idx <num_nodes; idx++)
    {
      Point p;
      mesh->get_point(p,idx);
      data[0] = p.x();
      data[1] = p.y();
      data[2] = p.z();
      data += 3;
    }
  }


  if (get(Parameters::BuildConnections).toBool())
  {
    connections.reset(new NrrdData());

    VMesh::size_type num_elems = mesh->num_elems();
    VMesh::Node::array_type array;
    mesh->get_nodes(array ,VMesh::Elem::index_type(0));

    VMesh::size_type dof = array.size();
    if (dof == 1)
    {
      size_t size[NRRD_DIM_MAX];
      size[0] = num_elems;
      nrrdAlloc_nva(connections->getNrrd(), nrrdTypeInt, 1, size);
    }
    else
    {
      size_t size[NRRD_DIM_MAX];
      size[0] = dof;
      size[1] = num_elems;
      nrrdAlloc_nva(connections->getNrrd(), nrrdTypeInt, 2, size);
    }

    int* data2 = reinterpret_cast<int*>(connections->getNrrd()->data);

    for (VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
    {
      mesh->get_nodes(array ,idx);

      for(size_t i=0; i<array.size() && i<static_cast<size_t>(dof); i++)
      {
        data2[i] = array[i];
      }
      data2 += array.size();
    }
  }


  if (get(Parameters::BuildData).toBool() && field->basis_order() > -1 && field->basis_order() < 2)
  {
    data.reset(new NrrdData());

    size_t elsize = 0;
    int    nrrdtype = 0;
    if (field->is_char()) { nrrdtype = nrrdTypeChar; elsize = 1; }
    if (field->is_unsigned_char()) { nrrdtype = nrrdTypeUChar; elsize = 1; }
    if (field->is_short()) { nrrdtype = nrrdTypeShort; elsize = 1; }
    if (field->is_unsigned_short()) { nrrdtype = nrrdTypeUShort; elsize = 1; }
    if (field->is_int()) { nrrdtype = nrrdTypeInt; elsize = 1; }
    if (field->is_unsigned_int()) { nrrdtype = nrrdTypeUInt; elsize = 1; }
    if (field->is_longlong()) { nrrdtype = nrrdTypeLLong; elsize = 1; }
    if (field->is_unsigned_longlong()) { nrrdtype = nrrdTypeULLong; elsize = 1; }
    if (field->is_float()) { nrrdtype = nrrdTypeFloat; elsize = 1; }
    if (field->is_double()) { nrrdtype = nrrdTypeDouble; elsize = 1; }
    if (field->is_vector()) { nrrdtype = nrrdTypeFloat; elsize = 3; }
    if (field->is_tensor()) { nrrdtype = nrrdTypeFloat; elsize = 7; }

    if (elsize == 1)
    {
      size_t size[NRRD_DIM_MAX];
      unsigned int centers[NRRD_DIM_MAX];
      for (size_t j=0;j<dataDims.size(); j++) size[j] = dataDims[j];
      nrrdAlloc_nva(data->getNrrd(), nrrdtype, dataDims.size(), size);

      if (field->basis_order() == 1)
      {
        for (size_t j=0;j<dataDims.size(); j++) centers[j] = nrrdCenterNode;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }
      else if (field->basis_order() == 0)
      {
        for (size_t j=0;j<dataDims.size(); j++) centers[j] = nrrdCenterCell;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }
      else
      {
        for (size_t j=0;j<dataDims.size(); j++) centers[j] = nrrdCenterUnknown;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }
      if (dataDims.size() > 0) data->getNrrd()->axis[0].label = airStrdup("x");
      if (dataDims.size() > 1) data->getNrrd()->axis[1].label = airStrdup("y");
      if (dataDims.size() > 2) data->getNrrd()->axis[2].label = airStrdup("z");

      if (with_spacing)
      {
        if (dataDims.size() > 0)
        {
          data->getNrrd()->axis[0].min=minP.x();
          data->getNrrd()->axis[0].max=maxP.x();
          data->getNrrd()->axis[0].spacing=spc.x();
        }
        if (dataDims.size() > 1)
        {
          data->getNrrd()->axis[1].min=minP.x();
          data->getNrrd()->axis[1].max=maxP.x();
          data->getNrrd()->axis[1].spacing=spc.x();
        }
        if (dataDims.size() > 2)
        {
          data->getNrrd()->axis[2].min=minP.x();
          data->getNrrd()->axis[2].max=maxP.x();
          data->getNrrd()->axis[2].spacing=spc.x();
        }
      }

      for (size_t j=0;j<dataDims.size(); j++) data->getNrrd()->axis[j].kind = nrrdKindDomain;

      VField::size_type num_values = field->num_values();
      if (field->is_char())
        field->get_values(reinterpret_cast<char*>(data->getNrrd()->data),num_values);
      if (field->is_unsigned_char())
        field->get_values(reinterpret_cast<unsigned char*>(data->getNrrd()->data),num_values);
      if (field->is_short())
        field->get_values(reinterpret_cast<short*>(data->getNrrd()->data),num_values);
      if (field->is_unsigned_short())
        field->get_values(reinterpret_cast<unsigned short*>(data->getNrrd()->data),num_values);
      if (field->is_int())
        field->get_values(reinterpret_cast<int*>(data->getNrrd()->data),num_values);
      if (field->is_unsigned_int())
        field->get_values(reinterpret_cast<unsigned int*>(data->getNrrd()->data),num_values);
      if (field->is_longlong())
        field->get_values(reinterpret_cast<long long*>(data->getNrrd()->data),num_values);
      if (field->is_unsigned_longlong())
        field->get_values(reinterpret_cast<unsigned long long*>(data->getNrrd()->data),num_values);
      if (field->is_float())
        field->get_values(reinterpret_cast<float*>(data->getNrrd()->data),num_values);
      if (field->is_double())
        field->get_values(reinterpret_cast<double*>(data->getNrrd()->data),num_values);
    }
    else
    {
      int kind = nrrdKind3Vector;
      int type = nrrdTypeFloat;
      std::string data_label = get(Parameters::DataLabel).toString();
      std::string label = data_label + ":Vector";
      if (field->is_tensor())
      {
        kind = nrrdKind3DMaskedSymMatrix;
        label = data_label + ":Tensor";
        type = nrrdTypeFloat;
      }

      size_t size[NRRD_DIM_MAX];
      unsigned int centers[NRRD_DIM_MAX];

      size[0] = elsize;
      for (size_t j=0;j<dataDims.size(); j++) size[j+1] = dataDims[j];
      nrrdAlloc_nva(data->getNrrd(), type, dataDims.size()+1, size);

      if (field->basis_order() == 1)
      {
        for (size_t j=0;j<dataDims.size()+1; j++) centers[j] = nrrdCenterNode;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }
      else if (field->basis_order() == 0)
      {
        for (size_t j=0;j<dataDims.size()+1; j++) centers[j] = nrrdCenterCell;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }
      else
      {
        for (size_t j=0;j<dataDims.size()+1; j++) centers[j] = nrrdCenterUnknown;
        nrrdAxisInfoSet_nva(data->getNrrd(), nrrdAxisInfoCenter, centers);
      }

      data->getNrrd()->axis[0].label = airStrdup(label.c_str());
      if (dataDims.size() > 0) data->getNrrd()->axis[1].label = airStrdup("x");
      if (dataDims.size() > 1) data->getNrrd()->axis[2].label = airStrdup("y");
      if (dataDims.size() > 2) data->getNrrd()->axis[3].label = airStrdup("z");

      if (with_spacing)
      {
        if (dataDims.size() > 0)
        {
          data->getNrrd()->axis[1].min=minP.x();
          data->getNrrd()->axis[1].max=maxP.x();
          data->getNrrd()->axis[1].spacing=spc.x();
        }
        if (dataDims.size() > 1)
        {
          data->getNrrd()->axis[2].min=minP.x();
          data->getNrrd()->axis[2].max=maxP.x();
          data->getNrrd()->axis[2].spacing=spc.x();
        }
        if (dataDims.size() > 2)
        {
          data->getNrrd()->axis[3].min=minP.x();
          data->getNrrd()->axis[3].max=maxP.x();
          data->getNrrd()->axis[3].spacing=spc.x();
        }
      }

      data->getNrrd()->axis[0].kind = kind;
      for (size_t j=0;j<dataDims.size(); j++) data->getNrrd()->axis[j+1].kind = nrrdKindDomain;

      VField::size_type num_values = field->num_values();

      if (field->is_vector())
      {
        Vector vec;
        float* data_ptr = reinterpret_cast<float*>(data->getNrrd()->data);
        for(VField::index_type idx=0; idx<num_values; idx++)
        {
          field->get_value(vec,idx);
          data_ptr[0] = static_cast<float>(vec.x());
          data_ptr[1] = static_cast<float>(vec.y());
          data_ptr[2] = static_cast<float>(vec.z());
          data_ptr += 3;
        }
      }
      else
      {
        Tensor tensor;
        float* data_ptr = reinterpret_cast<float*>(data->getNrrd()->data);
        for(VField::index_type idx=0; idx<num_values; idx++)
        {
          field->get_value(tensor,idx);
          data_ptr[0] = static_cast<float>(1.0);
          data_ptr[1] = static_cast<float>(tensor.val(0,0));
          data_ptr[2] = static_cast<float>(tensor.val(0,1));
          data_ptr[3] = static_cast<float>(tensor.val(0,2));
          data_ptr[4] = static_cast<float>(tensor.val(1,1));
          data_ptr[5] = static_cast<float>(tensor.val(1,2));
          data_ptr[6] = static_cast<float>(tensor.val(2,2));
          data_ptr += 7;
        }
      }
    }
  }

  return(true);
}

const AlgorithmOutputName ConvertToNrrdAlgo::Data("Data");
const AlgorithmOutputName ConvertToNrrdAlgo::Points("Points");
const AlgorithmOutputName ConvertToNrrdAlgo::Connections("Connections");

AlgorithmOutput ConvertToNrrdAlgo::run(const AlgorithmInput& input) const
{
	auto input_field = input.get<Field>(Variables::InputField);

	NrrdDataHandle pts, cxns, data;
	runImpl(input_field, pts, cxns, data);

	AlgorithmOutput output;
	output[Points] = pts;
  output[Connections] = cxns;
  output[Data] = data;
	return output;
}
