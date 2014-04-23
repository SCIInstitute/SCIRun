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
#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromNodeToElem.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromElemToNode.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
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
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildFEMatrix.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Math;

//TODO: add unit test 

HardCodedAlgorithmFactory::HardCodedAlgorithmFactory() {}

AlgorithmHandle HardCodedAlgorithmFactory::create(const std::string& moduleName, const AlgorithmCollaborator* algoCollaborator) const
{
  AlgorithmHandle h;

  if (moduleName == "GetFieldBoundary")
    h.reset(new GetFieldBoundaryAlgo);
  else if (moduleName == "SolveLinearSystem")
    h.reset(new SolveLinearSystemAlgo);
  else if (moduleName == "CalculateSignedDistanceToField")
    h.reset(new CalculateSignedDistanceFieldAlgo);
  else if (moduleName == "CalculateGradients")
    h.reset(new CalculateGradientsAlgo);
  else if (moduleName == "ConvertQuadSurfToTriSurf")
    h.reset(new ConvertMeshToTriSurfMeshAlgo);
  else if (moduleName == "AlignMeshBoundingBoxes")
    h.reset(new AlignMeshBoundingBoxesAlgo);
  else if (moduleName == "GetFieldNodes") //TODO: interesting case of module/algo name mismatch. Could be a problem if I want to make this factory more generic
    h.reset(new GetMeshNodesAlgo);    
  else if (moduleName == "ElectrodeCoilSetup")
    h.reset(new ElectrodeCoilSetupAlgorithm);     
  else if (moduleName == "SetConductivitiesToTetMesh")
    h.reset(new SetConductivitiesToTetMeshAlgorithm); 
  else if (moduleName == "SetupRHSforTDCSandTMS")
    h.reset(new SetupRHSforTDCSandTMSAlgorithm);   
  else if (moduleName == "GenerateROIStatistics")
    h.reset(new GenerateROIStatisticsAlgorithm);        
  else if (moduleName == "SetFieldNodes")
    h.reset(new SetMeshNodesAlgo);
  else if (moduleName == "ReportFieldInfo")
    h.reset(new ReportFieldInfoAlgorithm);
  else if (moduleName == "ReportMatrixInfo")
    h.reset(new ReportMatrixInfoAlgorithm);
  else if (moduleName == "AppendMatrix")
    h.reset(new AppendMatrixAlgorithm);
  else if (moduleName == "ReadMatrix")
    h.reset(new ReadMatrixAlgorithm);
  else if (moduleName == "WriteMatrix")
    h.reset(new WriteMatrixAlgorithm);
  else if (moduleName == "EvaluateLinearAlgebraUnary")
    h.reset(new EvaluateLinearAlgebraUnaryAlgorithm);
  else if (moduleName == "EvaluateLinearAlgebraBinary")
    h.reset(new EvaluateLinearAlgebraBinaryAlgorithm);
  else if (moduleName == "ConvertMeshToIrregularMesh")
    h.reset(new ConvertMeshToIrregularMeshAlgo);
  else if (moduleName == "ReadMesh")
    h.reset(new TextToTriSurfFieldAlgorithm);
  else if (moduleName == "BuildTDCSMatrix")
    h.reset(new BuildTDCSMatrixAlgo);
  else if (moduleName == "AddKnownsToLinearSystem")
    h.reset(new AddKnownsToLinearSystemAlgo);  
  else if (moduleName == "CalculateVectorMagnitudes")
    h.reset(new CalculateVectorMagnitudesAlgo); 
  else if (moduleName == "BuildFEMatrix")
    h.reset(new BuildFEMatrixAlgo);
  else if (moduleName == "GetDomainBoundary")
    h.reset(new GetDomainBoundaryAlgo);
  else if (moduleName == "InterfaceWithCleaver")
    h.reset(new InterfaceWithCleaverAlgorithm);      
  else if (moduleName == "GetFieldData") //TODO: interesting case of module/algo name mismatch. Could be a problem if I want to make this factory more generic
    h.reset(new GetFieldDataAlgo);
  else if (moduleName == "SetFieldData") //TODO: interesting case of module/algo name mismatch. Could be a problem if I want to make this factory more generic
    h.reset(new SetFieldDataAlgo);    
  else if (moduleName == "JoinFields")
    h.reset(new JoinFieldsAlgo);
  else if (moduleName == "SplitFieldByDomain")
    h.reset(new SplitFieldByDomainAlgo);
  else if (moduleName == "ApplyMappingMatrix")
    h.reset(new ApplyMappingMatrixAlgo); 
  else if (moduleName == "SelectSubMatrix")
    h.reset(new SelectSubMatrixAlgorithm);   
  else if (moduleName == "ConvertMatrixType")
    h.reset(new ConvertMatrixTypeAlgorithm);  
  else if (moduleName == "MapFieldDataFromElemToNode")
    h.reset(new MapFieldDataFromElemToNodeAlgo);      
  else if (moduleName == "MapFieldDataFromNodeToElem")
    h.reset(new MapFieldDataFromNodeToElemAlgo);   
    
  if (h && algoCollaborator)
  {
    h->setLogger(algoCollaborator->getLogger());
    h->setUpdaterFunc(algoCollaborator->getUpdaterFunc());
  }

  return h;
}
