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

#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/AlignMeshBoundingBoxes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/SetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystemAlgo.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Algorithms/DataIO/WriteMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms::BrainStimulator;

HardCodedAlgorithmFactory::HardCodedAlgorithmFactory() {}

AlgorithmHandle HardCodedAlgorithmFactory::create(const std::string& name, const AlgorithmCollaborator* algoCollaborator) const
{
  AlgorithmHandle h;

  if (name == "GetFieldBoundary")
    h.reset(new GetFieldBoundaryAlgo);
  else if (name == "SolveLinearSystem")
    h.reset(new SolveLinearSystemAlgo);
  else if (name == "CalculateSignedDistanceToField")
    h.reset(new CalculateSignedDistanceFieldAlgo);
  else if (name == "CalculateGradients")
    h.reset(new CalculateGradientsAlgo);
  else if (name == "ConvertQuadSurfToTriSurf")
    h.reset(new ConvertMeshToTriSurfMeshAlgo);
  else if (name == "AlignMeshBoundingBoxes")
    h.reset(new AlignMeshBoundingBoxesAlgo);
  else if (name == "GetFieldNodes") //TODO: interesting case of module/algo name mismatch. Could be a problem if I want to make this factory more generic
    h.reset(new GetMeshNodesAlgo);    
  else if (name == "ElectrodeCoilSetup")
    h.reset(new ElectrodeCoilSetupAlgorithm);     
  else if (name == "SetConductivitiesToTetMesh")
    h.reset(new SetConductivitiesToTetMeshAlgorithm); 
  else if (name == "SetupRHSforTDCSandTMS")
    h.reset(new SetupRHSforTDCSandTMSAlgorithm);   
  else if (name == "GenerateROIStatistics")
    h.reset(new GenerateROIStatisticsAlgorithm);        
  else if (name == "SetFieldNodes")
    h.reset(new SetMeshNodesAlgo);
  else if (name == "ReportFieldInfo")
    h.reset(new ReportFieldInfoAlgorithm);
  else if (name == "ReportMatrixInfo")
    h.reset(new ReportMatrixInfoAlgorithm);
  else if (name == "AppendMatrix")
    h.reset(new AppendMatrixAlgorithm);
  else if (name == "ReadMatrix")
    h.reset(new ReadMatrixAlgorithm);
  else if (name == "WriteMatrix")
    h.reset(new WriteMatrixAlgorithm);
  else if (name == "EvaluateLinearAlgebraUnary")
    h.reset(new EvaluateLinearAlgebraUnaryAlgorithm);
  else if (name == "EvaluateLinearAlgebraBinary")
    h.reset(new EvaluateLinearAlgebraBinaryAlgorithm);
  else if (name == "ConvertMeshToIrregularMesh")
    h.reset(new ConvertMeshToIrregularMeshAlgo);
  else if (name == "ReadMesh")
    h.reset(new TextToTriSurfFieldAlgorithm);
  else if (name == "BuildTDCSMatrix")
    h.reset(new BuildTDCSMatrixAlgo);
    
  if (h && algoCollaborator)
  {
    h->setLogger(algoCollaborator->getLogger());
    h->setUpdaterFunc(algoCollaborator->getUpdaterFunc());
  }

  return h;
}
