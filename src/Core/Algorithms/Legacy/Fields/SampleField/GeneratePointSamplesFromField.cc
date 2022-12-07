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


#include <Core/Algorithms/Legacy/Fields/SampleField/GeneratePointSamplesFromField.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, NumSamples);
ALGORITHM_PARAMETER_DEF(Fields, DistributionType);
ALGORITHM_PARAMETER_DEF(Fields, IncrementRNGSeed);
ALGORITHM_PARAMETER_DEF(Fields, ClampToNodes);
ALGORITHM_PARAMETER_DEF(Fields, RNGSeed);

GeneratePointSamplesFromFieldAlgo::GeneratePointSamplesFromFieldAlgo()
{
  addParameter(Parameters::NumSamples, 10);
  addParameter(Parameters::RNGSeed, 1);
  addOption(Parameters::DistributionType,"uniuni","impscat|impuni|uniuni|uniscat");
  addParameter(Parameters::ClampToNodes,true);
}

namespace detail
{
class GeneratePointSamplesFromFieldAlgoF {
  public:
    typedef std::pair<double, VMesh::Elem::index_type> weight_type;
    typedef std::vector<weight_type> table_type;

    bool build_table(VMesh *mesh, VField* vfield,
                     std::vector<weight_type> &table,
                     std::string& method);

    static bool
    weight_less(const weight_type &a, const weight_type &b)
    {
      return (a.first < b.first);
    }
};

bool
GeneratePointSamplesFromFieldAlgoF::build_table(VMesh *vmesh,
                                                VField* vfield,
                                                std::vector<weight_type> &table,
                                                std::string& method)
{
  VMesh::size_type num_elems = vmesh->num_elems();

  long double sum = 0.0;
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    double elemsize = 0.0;
    if (method == "impuni")
    { // Size of element * data at element.
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length() * vmesh->get_size(idx);
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d * vmesh->get_size(idx);
        }
      }
    }
    else if (method == "impscat")
    { // data at element
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length();
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d;
        }
      }
    }
    else if (method == "uniuni")
    { // size of element only
      elemsize = vmesh->get_size(idx);
    }
    else if (method == "uniscat")
    {
      elemsize = 1.0;
    }

    if (elemsize > 0.0)
    {
      sum += elemsize;
      table.push_back(weight_type(sum, idx));
    }
  }
  if (table.size() > 0)
  {
    return (true);
  }

  return (false);
}
}

bool
GeneratePointSamplesFromFieldAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "GeneratePointSamplesFromField");

  VField::size_type num_seeds = get(Parameters::NumSamples).toInt();
  int               rng_seeds = get(Parameters::RNGSeed).toInt();
  std::string method          = getOption(Parameters::DistributionType);
  bool              clamp     = get(Parameters::ClampToNodes).toBool();

  if (!input)
  {
    error("No input field was given");
    return (false);
  }

  VMesh*  mesh  = input->vmesh();
  VField* field = input->vfield();

  using namespace detail;
  GeneratePointSamplesFromFieldAlgoF table_algo;
  GeneratePointSamplesFromFieldAlgoF::table_type table;

  if (method == "uniuni" || method == "uniscat")
  {
    if (!table_algo.build_table(mesh, field, table, method))
    {
      error("Unable to build unweighted weight table for this mesh. Mesh is likely to be empty.");
      return (false);
    }
  }
  else if (field->is_scalar() || field->is_vector())
  {
    mesh->synchronize(Mesh::LOCATE_E);
    if (!table_algo.build_table(mesh, field, table, method))
    {
      error("Invalid weights in mesh, probably all zero. Try using an unweighted option.");
      return (false);
    }
  }
  else
  {
    error("Mesh contains non-weight data. Try using an unweighted option.");
    return (false);
  }

  FieldRNG rng(rng_seeds);

  long double max = table[table.size()-1].first;

  FieldInformation fi("PointCloudMesh",0,"double");
  output = CreateField(fi);

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  for (VField::index_type i=0; i < num_seeds; i++)
  {
    Point p;
    GeneratePointSamplesFromFieldAlgoF::table_type::iterator loc;

    do
    {
      loc = std::lower_bound(table.begin(), table.end(),
			 GeneratePointSamplesFromFieldAlgoF::weight_type(
       rng() * max, VMesh::Elem::index_type(0)),
       GeneratePointSamplesFromFieldAlgoF::weight_less);
    }
    while (loc == table.end());

    if (clamp)
    {
      // Find a random node in that cell.
      VMesh::Node::array_type ra;
      mesh->get_nodes(ra, (*loc).second);
      auto index = static_cast<size_t>(rng()*ra.size());
      mesh->get_center(p, ra[index]);
    }
    else
    {
      // Find random point in that cell.
      mesh->get_random_point(p, (*loc).second, rng);
    }
    omesh->add_point(p);
  }

  ofield->resize_values();

  return (true);
}

AlgorithmOutput GeneratePointSamplesFromFieldAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(inputField, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Samples] = outputField;
  return output;
}

const AlgorithmOutputName GeneratePointSamplesFromFieldAlgo::Samples("Samples");
