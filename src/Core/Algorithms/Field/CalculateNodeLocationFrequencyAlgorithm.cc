/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Core/Algorithms/Field/CalculateNodeLocationFrequencyAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <boost/unordered_map.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

namespace
{
  struct PointHash
  {
    size_t operator()(const Point& p) const
    {
      std::hash<double> hash;
      return hash(p.x()) ^ hash(p.y()) ^ hash(p.z());
    }
  };
}

FieldHandle CalculateNodeLocationFrequencyAlgo::runImpl(FieldHandle input) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateNodeLocationFrequencyAlgo");

  if (!input)
    THROW_ALGORITHM_INPUT_ERROR("No input field");

  FieldInformation fi(input);

  if (!fi.is_pointcloudmesh())
    THROW_ALGORITHM_INPUT_ERROR("Only point cloud input is supported at this time.");

  fi.make_unsigned_int();
  fi.make_lineardata();

  auto output = CreateField(fi, input->mesh());

  if (!output)
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output field");

  auto facade(input->mesh()->getFacade());
  boost::unordered_map<Point, unsigned int, PointHash> pointFreq;
  for (const auto& node : facade->nodes())
  {
    pointFreq[node.point()]++;
  }

  auto outputFacade(output->mesh()->getFacade());
  auto ofield = output->vfield();
  for (const auto& node : outputFacade->nodes())
  {
    ofield->set_value(pointFreq[node.point()], node.index());
  }

  return output;
}

AlgorithmOutput CalculateNodeLocationFrequencyAlgo::run_generic(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);

  AlgorithmOutput output;
  output[Variables::OutputField] = runImpl(inputField);
  return output;
}
