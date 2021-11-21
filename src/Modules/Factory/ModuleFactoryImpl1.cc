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


#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Basic/ReceiveComplexScalar.h>
#include <Modules/Basic/SendComplexScalar.h>
#include <Modules/Basic/DynamicPortTester.h>
#include <Modules/Legacy/Bundle/GetFieldsFromBundle.h>
#include <Modules/Basic/PrintDatatype.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Modules/Math/ReportMatrixInfo.h>
#include <Modules/Math/AppendMatrix.h>
#include <Modules/Math/CreateMatrix.h>
#include <Modules/Math/GetMatrixSlice.h>
#include <Modules/Math/SolveLinearSystem.h>
#include <Modules/Fields/ReportFieldInfo.h>
#include <Modules/Fields/EditMeshBoundingBox.h>
#include <Modules/Legacy/Fields/GetFieldBoundary.h>
#include <Modules/Legacy/Fields/CalculateSignedDistanceToField.h>
#include <Modules/Legacy/Fields/CalculateGradients.h>
#include <Modules/Legacy/Fields/ConvertQuadSurfToTriSurf.h>
#include <Modules/Legacy/Fields/AlignMeshBoundingBoxes.h>
#include <Modules/Legacy/Fields/GetFieldNodes.h>
#include <Modules/Legacy/Fields/SetFieldNodes.h>
#include <Modules/Legacy/Fields/GetSliceFromStructuredFieldByIndices.h>
#include <Modules/Legacy/Fields/JoinFields.h>
#include <Modules/Legacy/Fields/CalculateFieldData5.h>
#include <Modules/Legacy/Fields/TransformMeshWithTransform.h>
#include <Modules/Legacy/Fields/ResampleRegularMesh.h>
#include <Modules/Legacy/Fields/InterfaceWithTetGen.h>
#include <Modules/Legacy/Fields/BuildMappingMatrix.h>
#include <Modules/Legacy/Fields/ProjectPointsOntoMesh.h>
#include <Modules/Legacy/Fields/CalculateDistanceToField.h>
#include <Modules/Legacy/Fields/CalculateDistanceToFieldBoundary.h>
#include <Modules/Legacy/Fields/MapFieldDataOntoElems.h>
#include <Modules/Legacy/Fields/MapFieldDataOntoNodes.h>
#include <Modules/Legacy/Fields/ClipFieldByFunction3.h>
#include <Modules/Legacy/Fields/MapFieldDataFromSourceToDestination.h>
#include <Modules/Legacy/Fields/RefineMesh.h>
#include <Modules/Legacy/Fields/SetFieldDataToConstantValue.h>
#include <Modules/Legacy/Fields/FlipSurfaceNormals.h>
#include <Modules/Legacy/Fields/SwapFieldDataWithMatrixEntries.h>
#include <Modules/Legacy/Fields/BuildMatrixOfSurfaceNormals.h>
#include <Modules/Legacy/Fields/ConvertIndicesToFieldData.h>
#include <Modules/Legacy/Fields/RegisterWithCorrespondences.h>
#include <Modules/Legacy/Fields/GeneratePointSamplesFromField.h>
#include <Modules/Legacy/Fields/GeneratePointSamplesFromFieldOrWidget.h>
#include <Modules/Legacy/Fields/GenerateSinglePointProbeFromField.h>
#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>
#include <Modules/Legacy/Math/BuildNoiseColumnMatrix.h>
#include <Modules/Legacy/Math/ComputeSVD.h>
#include <Modules/Legacy/Math/AddLinkedNodesToLinearSystem.h>
#include <Modules/Legacy/Math/ReportColumnMatrixMisfit.h>
#include <Modules/Legacy/Math/CollectMatrices.h>
#include <Modules/Legacy/Matlab/DataIO/ImportDatatypesFromMatlab.h>
#include <Modules/Legacy/Forward/BuildBEMatrix.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>
#include <Modules/DataIO/ReadMatrixClassic.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/DataIO/ReadField.h>
#include <Modules/DataIO/WriteField.h>
#include <Modules/String/CreateString.h>
#include <Modules/String/NetworkNotes.h>
#include <Modules/Visualization/ShowField.h>
#include <Modules/Visualization/ShowFieldGlyphs.h>
#include <Modules/Visualization/CreateStandardColorMap.h>
#include <Modules/Visualization/ShowColorMapModule.h>
#include <Modules/Visualization/CreateTestingArrow.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Legacy/FiniteElements/BuildFEMatrix.h>
#include <Modules/Basic/AsyncPortTestModule.h>
#include <Modules/Basic/NeedToExecuteTester.h>
#include <Modules/Legacy/Converters/ConvertMatrixToString.h>
#include <Modules/Basic/PortFeedbackTestModules.h>
#include <Modules/Basic/LoggingTester.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Modules::Inverse;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::Matlab::DataIO;
using namespace SCIRun::Modules::StringProcessing;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Modules::Render;
using namespace SCIRun::Modules::Converters;

void ModuleDescriptionLookup::addEssentialModules()
{
  addModuleDesc<ReadMatrix>("ReadMatrix", "DataIO", "SCIRun", "Redo of ReadMatrix", "...");
  addModuleDesc<WriteMatrix>("WriteMatrix", "DataIO", "SCIRun", "Functional, outputs text files or binary .mat only.", "...");
  addModuleDesc<ReadField>("ReadField", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
  addModuleDesc<WriteField>("WriteField", "DataIO", "SCIRun", "Functional, outputs binary .fld only.", "...");
  addModuleDesc<PrintDatatype>("PrintDatatype", "String", "SCIRun", "...", "...");
  addModuleDesc<ReportMatrixInfo>("ReportMatrixInfo", "Math", "SCIRun", "Functional, needs GUI work.", "...");
  addModuleDesc<ReportFieldInfo>("ReportFieldInfo", "MiscField", "SCIRun", "Same as v4", "...");
  addModuleDesc<AppendMatrix>("AppendMatrix", "Math", "SCIRun", "Fully functional.", "...");
  addModuleDesc<EvaluateLinearAlgebraUnary>("EvaluateLinearAlgebraUnary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
  addModuleDesc<EvaluateLinearAlgebraBinary>("EvaluateLinearAlgebraBinary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
  addModuleDesc<CreateMatrix>("CreateMatrix", "Math", "SCIRun", "Functional, needs GUI work.", "...");
  addModuleDesc<SolveLinearSystem>("SolveLinearSystem", "Math", "SCIRun", "Four multi-threaded algorithms available.", "...");
  addModuleDesc<CreateString>("CreateString", "String", "SCIRun", "Functional, needs GUI work.", "...");
	addModuleDesc<NetworkNotes>("NetworkNotes", "String", "SCIRun", "Functional, needs GUI work.", "...");
  addModuleDesc<ShowField>("Some basic options available, still work in progress.", "...");
  addModuleDesc<ShowFieldGlyphs>("Rewrite", "...");
  addModuleDesc<ViewScene>("Can display meshes and fields, pan/rotate/zoom.", "...");

  addModuleDesc<GetFieldBoundary>("GetFieldBoundary", "NewField", "SCIRun", "First real ported module", "...");
  addModuleDesc<CalculateSignedDistanceToField>("CalculateSignedDistanceToField", "ChangeFieldData", "SCIRun", "Second real ported module", "...");
  addModuleDesc<CalculateGradients>("CalculateGradients", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<ConvertQuadSurfToTriSurf>("ConvertQuadSurfToTriSurf", "ChangeMesh", "SCIRun", "Real ported module", "...");
	addModuleDesc<AlignMeshBoundingBoxes>("AlignMeshBoundingBoxes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<GetFieldNodes>("GetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<SetFieldNodes>("SetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<SolveMinNormLeastSqSystem>("SolveMinNormLeastSqSystem", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<CreateStandardColorMap>("In progress: four color maps available", "...");
  addModuleDesc<JoinFields>("Real ported module: Many bugs and UI logic issues", "...");
  addModuleDesc<GetSliceFromStructuredFieldByIndices>("Real ported module", "...");
  addModuleDesc<CalculateFieldData>("Real ported module", "...");
	addModuleDesc<SwapFieldDataWithMatrixEntries>("SwapFieldDataWithMatrixEntires","...");
  addModuleDesc<BuildFEMatrix>("BuildFEMatrix", "FiniteElements", "SCIRun", "In progress: main path through code works", "Generates stiffness matrix ");
  addModuleDesc<BuildBEMatrix>("Real ported module", "...");
  addModuleDesc<ResampleRegularMesh>("Real ported module", "...");
  addModuleDesc<ProjectPointsOntoMesh>("Real ported module", "...");
  addModuleDesc<InterfaceWithTetGen>("Real ported module", "...");
  addModuleDesc<CalculateDistanceToField>("Real ported module", "...");
  addModuleDesc<CalculateDistanceToFieldBoundary>("Real ported module", "...");
  addModuleDesc<MapFieldDataOntoNodes>("Real ported module", "...");
  addModuleDesc<MapFieldDataOntoElements>("Real ported module", "...");
  addModuleDesc<ClipFieldByFunction>("In progress", "...");
  addModuleDesc<MapFieldDataFromSourceToDestination>("Real ported module", "...");
  addModuleDesc<GetMatrixSlice>("New module based on GetRowOrColumnFromMatrix", "...");
	addModuleDesc<RefineMesh>("RefineMesh","ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<SetFieldDataToConstantValue>("Real ported module", "...");
	addModuleDesc<BuildMatrixOfSurfaceNormals>("Real ported module", "...");
  addModuleDesc<AddLinkedNodesToLinearSystem>("Real ported module", "...");
  addModuleDesc<ReportColumnMatrixMisfit>("Real ported module: GUI incomplete, untested", "...");
  addModuleDesc<BuildMappingMatrix>("Real ported module", "...");
  addModuleDesc<FlipSurfaceNormals>("FlipSurfaceNormals","ChangeMesh","SCIRun","...","...");
  addModuleDesc<BuildNoiseColumnMatrix>("BuildNoiseColumnMatrix","Math","SCIRun","...","...");
  addModuleDesc<ComputeSVD>("ComputeSVD","Math","SCIRun","...","...");

  addModuleDesc<EditMeshBoundingBox>("Rewrite", "...");
  addModuleDesc<ConvertIndicesToFieldData>("Real ported module", "...");
  addModuleDesc<SolveInverseProblemWithTikhonov>("...", "...");
  addModuleDesc<TransformMeshWithTransform>("...", "...");
  addModuleDesc<ShowColorMap>("ShowColorMap", "Visualization", "SCIRun", "Real ported module", "...");
  addModuleDesc<ConvertMatrixToString>("ConvertMatrixToString","Converters","SCIRun","...","...");
  addModuleDesc<RegisterWithCorrespondences>("RegisterWithCorrespondences","ChangeFieldData","SCIRun","...","In progress--Computation is incorrect");

  addModuleDesc<GeneratePointSamplesFromField>("Real ported module", "in progress");
  addModuleDesc<GeneratePointSamplesFromFieldOrWidget>("Real ported module", "in progress");
  addModuleDesc<GenerateSinglePointProbeFromField>("Real ported module", "in progress");
  addModuleDesc<CollectMatrices>("Real ported module", "...");
}

void ModuleDescriptionLookup::addTestingModules()
{
  addModuleDesc<SendComplexScalar>("SendComplexMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<ReceiveComplexScalar>("ReceiveComplexMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<DynamicPortTester>("DynamicPortTester", "Testing", "SCIRun", "...", "...");
  addModuleDesc<AsyncPortTestModule>("...", "ViewScene clone");
  addModuleDesc<NeedToExecuteTester>("...", "...");
  addModuleDesc<PortFeedbackReceiver>("...", "...");
  addModuleDesc<PortFeedbackSender>("...", "...");
  addModuleDesc<LoggingTester>("...", "...");
  addModuleDesc<CreateTestingArrow>("...", "...");
}
