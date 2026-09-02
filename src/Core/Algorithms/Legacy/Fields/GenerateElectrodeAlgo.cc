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


#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Color.h>


using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, ElectrodeLength);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeThickness);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeWidth);
ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeProjection);
ALGORITHM_PARAMETER_DEF(Fields, MoveAll);
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);

ALGORITHM_PARAMETER_DEF(Fields, ProbeColor);
ALGORITHM_PARAMETER_DEF(Fields, ProbeLabel);
ALGORITHM_PARAMETER_DEF(Fields, ProbeSize);


const AlgorithmOutputName GenerateElectrodeAlgo::ControlPoints("ControlPoints");
const AlgorithmOutputName GenerateElectrodeAlgo::ElectrodeMesh("ElectrodeMesh");

GenerateElectrodeAlgo::GenerateElectrodeAlgo()
{
  addParameter(Parameters::ElectrodeLength, 0.1);
  addParameter(Parameters::ElectrodeThickness, 0.003);
  addParameter(Parameters::ElectrodeWidth, 0.02);
  addOption(Parameters::ElectrodeType,"wire","wire|planar");
  addOption(Parameters::ElectrodeProjection,"midway","positive|midway|negative");
  addParameter(Parameters::NumberOfControlPoints,5);
  addParameter(Parameters::ElectrodeResolution,10);
  addParameter(Parameters::UseFieldNodes,true);
  addParameter(Parameters::MoveAll,false);
  addParameter(Parameters::ProbeColor, "Color(1.0, 1.0, 1.0)");
  addParameter(Parameters::ProbeLabel, std::string());
  addParameter(Parameters::ProbeSize, 1.0);
  
}

//namespace detail
//{
//class GenerateElectrodeAlgoF {
//  public:
//    typedef std::pair<double, VMesh::Elem::index_type> weight_type;
//    typedef std::vector<weight_type> table_type;
//
//    bool build_table(VMesh *mesh, VField* vfield,
//                     std::vector<weight_type> &table,
//                     std::string& method);
//
//    static bool
//    weight_less(const weight_type &a, const weight_type &b)
//    {
//      return (a.first < b.first);
//    }
//};






AlgorithmOutput GenerateElectrodeAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
    

  FieldHandle outputField;
  FieldHandle outputPoints;
  if (!runImpl(inputField, outputField, outputPoints))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[ControlPoints] = outputPoints;
  output[ElectrodeMesh] = outputField;
  return output;
}

 
