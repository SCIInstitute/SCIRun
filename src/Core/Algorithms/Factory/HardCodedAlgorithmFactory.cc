/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromElemToNode.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateVectorMagnitudesAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/AlignMeshBoundingBoxes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/SetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ResampleMesh/ResampleRegularMesh.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Legacy/Fields/SmoothMesh/FairMesh.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/ScaleFieldMeshAndData.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/ProjectPointsOntoMesh.h>
#include <Core/Algorithms/Math/AddKnownsToLinearSystem.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Algorithms/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildFEMatrix.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshBySelection.h>
#include <boost/functional/factory.hpp>
#include <boost/assign.hpp>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Math;
using namespace boost::assign;

/// @todo: add unit test

HardCodedAlgorithmFactory::HardCodedAlgorithmFactory()
{
  addToMakerMap();
  addToMakerMap2();
}

#define ADD_MODULE_ALGORITHM(module, algorithm) (#module, boost::factory<algorithm*>())

void HardCodedAlgorithmFactory::addToMakerMap()
{
  if (factoryMap_.empty())
  {
    insert(factoryMap_)
      ADD_MODULE_ALGORITHM(GetFieldBoundary, GetFieldBoundaryAlgo)
      ADD_MODULE_ALGORITHM(SolveLinearSystem, SolveLinearSystemAlgo)
      ADD_MODULE_ALGORITHM(CalculateSignedDistanceToField, CalculateSignedDistanceFieldAlgo)
      ADD_MODULE_ALGORITHM(CalculateGradients, CalculateGradientsAlgo)
      ADD_MODULE_ALGORITHM(ConvertQuadSurfToTriSurf, ConvertMeshToTriSurfMeshAlgo)
      ADD_MODULE_ALGORITHM(AlignMeshBoundingBoxes, AlignMeshBoundingBoxesAlgo)
      ADD_MODULE_ALGORITHM(GetFieldNodes, GetMeshNodesAlgo)     /// @todo: interesting case of module/algo name mismatch. Could be a problem if I want to make this factory more generic
      ADD_MODULE_ALGORITHM(ElectrodeCoilSetup, ElectrodeCoilSetupAlgorithm)
      ADD_MODULE_ALGORITHM(SetConductivitiesToTetMesh, SetConductivitiesToTetMeshAlgorithm)
      ADD_MODULE_ALGORITHM(SetupRHSforTDCSandTMS, SetupRHSforTDCSandTMSAlgorithm)
      ADD_MODULE_ALGORITHM(GenerateROIStatistics, GenerateROIStatisticsAlgorithm)
      ADD_MODULE_ALGORITHM(SetFieldNodes, SetMeshNodesAlgo)
      ADD_MODULE_ALGORITHM(ReportFieldInfo, ReportFieldInfoAlgorithm)
      ADD_MODULE_ALGORITHM(ReportMatrixInfo, ReportMatrixInfoAlgorithm)
      ADD_MODULE_ALGORITHM(AppendMatrix, AppendMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(ReadMatrix, ReadMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(WriteMatrix, WriteMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(EvaluateLinearAlgebraUnary, EvaluateLinearAlgebraUnaryAlgorithm)
      ADD_MODULE_ALGORITHM(EvaluateLinearAlgebraBinary, EvaluateLinearAlgebraBinaryAlgorithm)
      ADD_MODULE_ALGORITHM(ConvertMeshToIrregularMesh, ConvertMeshToIrregularMeshAlgo)
      ADD_MODULE_ALGORITHM(ReadMesh, TextToTriSurfFieldAlgorithm)
      ADD_MODULE_ALGORITHM(AddKnownsToLinearSystem, AddKnownsToLinearSystemAlgo)
      ADD_MODULE_ALGORITHM(CalculateVectorMagnitudes, CalculateVectorMagnitudesAlgo)
      ADD_MODULE_ALGORITHM(BuildFEMatrix, BuildFEMatrixAlgo)
      ADD_MODULE_ALGORITHM(GetDomainBoundary, GetDomainBoundaryAlgo)
      ADD_MODULE_ALGORITHM(InterfaceWithCleaver, InterfaceWithCleaverAlgorithm)
      ADD_MODULE_ALGORITHM(GetFieldData, GetFieldDataAlgo)
      ADD_MODULE_ALGORITHM(SetFieldData, SetFieldDataAlgo)
      ADD_MODULE_ALGORITHM(JoinFields, JoinFieldsAlgo)
      ADD_MODULE_ALGORITHM(SplitFieldByDomain, SplitFieldByDomainAlgo)
      ADD_MODULE_ALGORITHM(ApplyMappingMatrix, ApplyMappingMatrixAlgo)
      ADD_MODULE_ALGORITHM(SelectSubMatrix, SelectSubMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(ConvertMatrixType, ConvertMatrixTypeAlgorithm)
      ADD_MODULE_ALGORITHM(MapFieldDataFromNodeToElem, MapFieldDataFromNodeToElemAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataFromElemToNode, MapFieldDataFromElemToNodeAlgo)
      ADD_MODULE_ALGORITHM(ResampleRegularMesh, ResampleRegularMeshAlgo)
      ADD_MODULE_ALGORITHM(FairMesh, FairMeshAlgo)
      ADD_MODULE_ALGORITHM(ScaleFieldMeshAndData, ScaleFieldMeshAndDataAlgo)
      ADD_MODULE_ALGORITHM(ConvertFieldBasis, ConvertFieldBasisTypeAlgo)
      ADD_MODULE_ALGORITHM(ProjectPointsOntoMesh, ProjectPointsOntoMeshAlgo)
      ADD_MODULE_ALGORITHM(CalculateDistanceToField, CalculateDistanceFieldAlgo)
      ADD_MODULE_ALGORITHM(CalculateDistanceToFieldBoundary, CalculateDistanceFieldAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataOntoNodes, MapFieldDataOntoNodesAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataOntoElems, MapFieldDataOntoElemsAlgo)
      ADD_MODULE_ALGORITHM(ClipFieldByFunction, ClipMeshBySelectionAlgo)
    ;
  }
}

AlgorithmHandle HardCodedAlgorithmFactory::create(const std::string& moduleName, const AlgorithmCollaborator* algoCollaborator) const
{
  AlgorithmHandle h;

  auto func = factoryMap_.find(moduleName);
  if (func != factoryMap_.end())
    h.reset((func->second)());

  if (h && algoCollaborator)
  {
    h->setLogger(algoCollaborator->getLogger());
    h->setUpdaterFunc(algoCollaborator->getUpdaterFunc());
  }

  return h;
}
