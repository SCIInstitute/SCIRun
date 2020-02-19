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


#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/MarchingCubes.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

#include <boost/unordered_map.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Isovalues);
ALGORITHM_PARAMETER_DEF(Fields, SingleIsoValue);
ALGORITHM_PARAMETER_DEF(Fields, IsovalueChoice);
ALGORITHM_PARAMETER_DEF(Fields, IsovalueListInclusiveExclusive);
ALGORITHM_PARAMETER_DEF(Fields, IsovalueQuantityFromField);
ALGORITHM_PARAMETER_DEF(Fields, ManualMinimumIsovalue);
ALGORITHM_PARAMETER_DEF(Fields, ManualMaximumIsovalue);
ALGORITHM_PARAMETER_DEF(Fields, ListOfIsovalues);
ALGORITHM_PARAMETER_DEF(Fields, QuantityOfIsovalues);
ALGORITHM_PARAMETER_DEF(Fields, IsovalueListString);

ExtractSimpleIsosurfaceAlgo::ExtractSimpleIsosurfaceAlgo()
{
  addParameter(Parameters::SingleIsoValue, 0.0);
  addParameter(Parameters::Isovalues, VariableList());
  addParameter(Parameters::IsovalueListInclusiveExclusive, 0);
  addParameter(Parameters::ListOfIsovalues, std::string());
  addParameter(Parameters::QuantityOfIsovalues, 1);
  addParameter(Parameters::IsovalueQuantityFromField, 1);
  addParameter(Parameters::ManualMaximumIsovalue, 0.0);
  addParameter(Parameters::ManualMinimumIsovalue, 0.0);
  addOption(Parameters::IsovalueChoice, "Single", "Single|List|Quantity");
}

bool ExtractSimpleIsosurfaceAlgo::run(FieldHandle input, const std::vector<double>& isovalues, FieldHandle& output) const
{
  if (isovalues.empty())
  {
    THROW_ALGORITHM_INPUT_ERROR("Error in ExtractIsosurface algorithm: No Isovalue available.");
  }

  MarchingCubesAlgo marching_;
  marching_.set(MarchingCubesAlgo::build_field, true);

  marching_.run(input, isovalues, output);

  return (true);
}

AlgorithmOutput ExtractSimpleIsosurfaceAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);
  FieldHandle output_field;
  auto iso_values = get(Parameters::Isovalues).toVector();
  std::vector<double> iso_value_vector;
  std::transform(iso_values.begin(), iso_values.end(), std::back_inserter(iso_value_vector), [](const Variable& v) { return v.toDouble(); });

  if (!run(field, iso_value_vector, output_field))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  DenseMatrixHandle output_matrix( new DenseMatrix(iso_value_vector.size(),1,0.0));
  for (size_t k=0;k<iso_value_vector.size();k++) {(*output_matrix)(k,0) = iso_value_vector[k];}

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;
  output[Variables::OutputMatrix] = output_matrix;

  return output;
}
