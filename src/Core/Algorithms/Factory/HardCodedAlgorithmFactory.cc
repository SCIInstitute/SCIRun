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

#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromElemToNode.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromSourceToDestination.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/BuildMatrixOfSurfaceNormalsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateVectorMagnitudesAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTetVolMesh.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/AlignMeshBoundingBoxes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/SetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/FlipSurfaceNormals.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
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
#include <Core/Algorithms/Legacy/Fields/TransformMesh/TransformMeshWithTransform.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SwapFieldDataWithMatrixEntriesAlgo.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertIndicesToFieldDataAlgo.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/BuildMappingMatrixAlgo.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>
#include <Core/Algorithms/Legacy/Fields/SampleField/GeneratePointSamplesFromField.h>
#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>
#include <Core/Algorithms/Math/AddKnownsToLinearSystem.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Algorithms/Math/SelectSubMatrix.h>
#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Math/BuildNoiseColumnMatrix.h>
#include <Core/Algorithms/Math/ComputeSVD.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildFEMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFEVolRHS.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SimulateForwardMagneticFieldAlgorithm.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshBySelection.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldDataToConstantValue.h>
#include <Core/Algorithms/Legacy/Converter/ConvertMatrixToString.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <boost/functional/factory.hpp>
#include <boost/assign.hpp>
#include <Core/Algorithms/Math/ComputePCA.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::Converters;
using namespace boost::assign;

/// @todo: add unit test

HardCodedAlgorithmFactory::HardCodedAlgorithmFactory()
{
  addToMakerMap();
  addToMakerMap2();
  addToMakerMapGenerated();
}

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
      ADD_MODULE_ALGORITHM(SetConductivitiesToMesh, SetConductivitiesToMeshAlgorithm)
      ADD_MODULE_ALGORITHM(SetupTDCS, SetupTDCSAlgorithm)
      ADD_MODULE_ALGORITHM(GenerateROIStatistics, GenerateROIStatisticsAlgorithm)
      ADD_MODULE_ALGORITHM(SetFieldNodes, SetMeshNodesAlgo)
      ADD_MODULE_ALGORITHM(ReportFieldInfo, ReportFieldInfoAlgorithm)
			ADD_MODULE_ALGORITHM(BuildMatrixOfSurfaceNormals, BuildMatrixOfSurfaceNormalsAlgo)
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
      ADD_MODULE_ALGORITHM(InterfaceWithCleaver, InterfaceWithCleaverAlgorithm)
      ADD_MODULE_ALGORITHM(GetFieldData, GetFieldDataAlgo)
      ADD_MODULE_ALGORITHM(SetFieldData, SetFieldDataAlgo)
      ADD_MODULE_ALGORITHM(GetMatrixSlice, GetMatrixSliceAlgo)
      ADD_MODULE_ALGORITHM(JoinFields, JoinFieldsAlgo)
      ADD_MODULE_ALGORITHM(SplitFieldByDomain, SplitFieldByDomainAlgo)
      ADD_MODULE_ALGORITHM(ApplyMappingMatrix, ApplyMappingMatrixAlgo)
      ADD_MODULE_ALGORITHM(SelectSubMatrix, SelectSubMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(ConvertMatrixType, ConvertMatrixTypeAlgorithm)
      ADD_MODULE_ALGORITHM(MapFieldDataFromNodeToElem, MapFieldDataFromNodeToElemAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataFromElemToNode, MapFieldDataFromElemToNodeAlgo)
      ADD_MODULE_ALGORITHM(ResampleRegularMesh, ResampleRegularMeshAlgo)
      ADD_MODULE_ALGORITHM(FairMesh, FairMeshAlgo)
      ADD_MODULE_ALGORITHM(GenerateStreamLines, GenerateStreamLinesAlgo)
      ADD_MODULE_ALGORITHM(TransformMeshWithTransform, TransformMeshWithTransformAlgo)
      ADD_MODULE_ALGORITHM(ConvertFieldBasis, ConvertFieldBasisTypeAlgo)
      ADD_MODULE_ALGORITHM(ProjectPointsOntoMesh, ProjectPointsOntoMeshAlgo)
      ADD_MODULE_ALGORITHM(CalculateDistanceToField, CalculateDistanceFieldAlgo)
      ADD_MODULE_ALGORITHM(CalculateDistanceToFieldBoundary, CalculateDistanceFieldAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataOntoNodes, MapFieldDataOntoNodesAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataOntoElements, MapFieldDataOntoElemsAlgo)
      ADD_MODULE_ALGORITHM(ClipFieldByFunction, ClipMeshBySelectionAlgo)
      ADD_MODULE_ALGORITHM(MapFieldDataFromSourceToDestination, MapFieldDataFromSourceToDestinationAlgo)
      ADD_MODULE_ALGORITHM(SimulateForwardMagneticField, SimulateForwardMagneticFieldAlgo)
      ADD_MODULE_ALGORITHM(BuildFEVolRHS, BuildFEVolRHSAlgo)
			ADD_MODULE_ALGORITHM(RefineMesh, RefineMeshAlgo)
      ADD_MODULE_ALGORITHM(SetFieldDataToConstantValue, SetFieldDataToConstantValueAlgo)
			ADD_MODULE_ALGORITHM(SwapFieldDataWithMatrixEntries, SwapFieldDataWithMatrixEntriesAlgo)
	    ADD_MODULE_ALGORITHM(FlipSurfaceNormals,FlipSurfaceNormalsAlgo)
      ADD_MODULE_ALGORITHM(GeneratePointSamplesFromFieldOrWidget,GeneratePointSamplesFromFieldAlgo)
	    ADD_MODULE_ALGORITHM(BuildNoiseColumnMatrix,BuildNoiseColumnMatrixAlgorithm)
      ADD_MODULE_ALGORITHM(BuildMappingMatrix, BuildMappingMatrixAlgo)
	    ADD_MODULE_ALGORITHM(ConvertIndicesToFieldData, ConvertIndicesToFieldDataAlgo)
	    ADD_MODULE_ALGORITHM(ComputeSVD, ComputeSVDAlgo)
		  ADD_MODULE_ALGORITHM(ConvertMatrixToString, ConvertMatrixToStringAlgo)
      ADD_MODULE_ALGORITHM(ConvertHexVolToTetVol, ConvertMeshToTetVolMeshAlgo)
      ADD_MODULE_ALGORITHM(ExtractSimpleIsosurface, ExtractSimpleIsosurfaceAlgo)
	    ADD_MODULE_ALGORITHM(RegisterWithCorrespondences, RegisterWithCorrespondencesAlgo)
      ADD_MODULE_ALGORITHM(ComputePCA, ComputePCAAlgo)
      ADD_MODULE_ALGORITHM(ExtractSimpleIsosurface, ExtractSimpleIsosurfaceAlgo)
	    ADD_MODULE_ALGORITHM(RegisterWithCorrespondences, RegisterWithCorrespondencesAlgo)
      ADD_MODULE_ALGORITHM(ClipVolumeByIsovalue, ClipMeshByIsovalueAlgo)
     ;
  }
}

AlgorithmHandle HardCodedAlgorithmFactory::create(const std::string& moduleName, const AlgorithmCollaborator* algoCollaborator) const
{
  AlgorithmHandle h;

  auto func = factoryMap_.find(moduleName);
  if (func != factoryMap_.end())
    h.reset((func->second)());

  //TODO: make a convenience function to copy these for "sub-algorithms"
  if (h && algoCollaborator)
  {
    h->setLogger(algoCollaborator->getLogger());
    h->setUpdaterFunc(algoCollaborator->getUpdaterFunc());
  }

  return h;
}
