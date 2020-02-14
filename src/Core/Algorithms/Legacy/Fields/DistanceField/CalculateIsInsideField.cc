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


#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateIsInsideField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ALGORITHM_PARAMETER_DEF(Fields, SamplingScheme);
ALGORITHM_PARAMETER_DEF(Fields, InsideFieldValue);
ALGORITHM_PARAMETER_DEF(Fields, OutsideFieldValue);
ALGORITHM_PARAMETER_DEF(Fields, FieldOutputType);
ALGORITHM_PARAMETER_DEF(Fields, CalcInsideMethod);

CalculateIsInsideFieldAlgo::CalculateIsInsideFieldAlgo()
{
  // How many samples inside the elements to test for being inside the
  // object field
  addOption(Parameters::SamplingScheme, "regular2", "regular1|regular2|regular3|regular4|regular5");
  // Value for inside elements
  addParameter(Parameters::InsideFieldValue, 1.0);
  // Value for outside elements
  addParameter(Parameters::OutsideFieldValue, 0.0);
  // Output type of the algorithm
  addOption(Parameters::FieldOutputType, "same as input", "same as input|char|unsigned char|short|unsigned short|int|unsigned int|float|double");
  // How many nodes need to be inside to call it inside
  addOption(Parameters::CalcInsideMethod, "all", "one|most|all");
}

bool
CalculateIsInsideFieldAlgo::runImpl(FieldHandle input, FieldHandle objfield, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "Calculate IsInsideField");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!objfield)
  {
    error("No object field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
  FieldInformation fobj(objfield);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  std::string output_type = getOption(Parameters::FieldOutputType);

  if (output_type != "same as input")
  {
    fo.set_data_type(output_type);
  }

  if (fo.is_vector()) fo.make_double();
  if (fo.is_tensor()) fo.make_double();
  fo.make_constantdata();

  bool clean_field = false;

  if (!output)
  {
    output = CreateField(fo,input->mesh());

    if (!output)
    {
      error("Could not create output field");
      return(false);
    }
    clean_field = true;
  }

  // For the moment we calculate everything in doubles

  VMesh*  omesh   = output->vmesh();
  VMesh*  objmesh = objfield->vmesh();
  VField* ofield  = output->vfield();

  double outside_value = get(Parameters::OutsideFieldValue).toDouble();
  double inside_value = get(Parameters::InsideFieldValue).toDouble();

  ofield->set_all_values(outside_value);

  objmesh->synchronize(Mesh::ELEM_LOCATE_E);

  VMesh::size_type num_elems = omesh->num_elems();
  VMesh::Node::array_type nodes;
  VMesh::Elem::index_type cidx;

  std::vector<Point> points;
  std::vector<Point> points2;

  std::vector<VMesh::coords_type> coords;
  std::vector<double> weights;

  std::string sampling_scheme = getOption(Parameters::SamplingScheme);
  if (sampling_scheme == "regular1") omesh->get_regular_scheme(coords,weights,1);
  else if (sampling_scheme == "regular2") omesh->get_regular_scheme(coords,weights,2);
  else if (sampling_scheme == "regular3") omesh->get_regular_scheme(coords,weights,3);
  else if (sampling_scheme == "regular4") omesh->get_regular_scheme(coords,weights,4);
  else if (sampling_scheme == "regular5") omesh->get_regular_scheme(coords,weights,5);

  std::string method = getOption(Parameters::CalcInsideMethod);

  if (method == "one")
  {
    for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
    {
      omesh->get_nodes(nodes,idx);
      omesh->get_centers(points,nodes);
      omesh->minterpolate(points2,coords,idx);

      bool is_inside = false;

      for (size_t r=0; r< points2.size(); r++)
      {
        if (objmesh->locate(cidx,points2[r]))
        {
          is_inside = true; break;
        }
      }

      if (!is_inside)
      {
        for (size_t r=0; r< points.size(); r++)
        {
          if (objmesh->locate(cidx,points[r]))
          {
            is_inside = true; break;
          }
        }
      }

      if (is_inside) ofield->set_value(inside_value,idx);
    }
  }
  else if (method == "all")
  {
    for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
    {
      omesh->get_nodes(nodes,idx);
      omesh->get_centers(points,nodes);
      omesh->minterpolate(points2,coords,idx);

      bool is_inside = true;

      for (size_t r=0; r< points2.size(); r++)
      {
        if (!(objmesh->locate(cidx,points2[r])))
        {
          is_inside = false; break;
        }
      }

      if (is_inside)
      {
        for (size_t r=0; r< points.size(); r++)
        {
          if (!(objmesh->locate(cidx,points[r])))
          {
            is_inside = false; break;
          }
        }
      }

      if (is_inside) ofield->set_value(inside_value,idx);
    }
  }
  else
  {
    for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
    {
      omesh->get_nodes(nodes,idx);
      omesh->get_centers(points,nodes);
      omesh->minterpolate(points2,coords,idx);

      int outside = 0;
      int inside = 0;
      for (size_t r=0; r< points2.size(); r++)
      {
        if (objmesh->locate(cidx,points2[r])) inside++; else outside++;
      }

      for (size_t r=0; r< points.size(); r++)
      {
        if (objmesh->locate(cidx,points[r])) inside++; else outside++;
      }

      if (inside >= outside) ofield->set_value(inside_value,idx);
    }
  }

  return (true);
}

AlgorithmOutput CalculateIsInsideFieldAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto objectField = input.get<Field>(Variables::ObjectField);

  FieldHandle outputField;

  if (!runImpl(inputField, objectField, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
