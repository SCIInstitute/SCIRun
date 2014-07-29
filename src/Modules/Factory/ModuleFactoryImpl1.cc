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

#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Basic/ReceiveScalar.h>
#include <Modules/Basic/SendScalar.h>
#include <Modules/Basic/ReceiveTestMatrix.h>
#include <Modules/Basic/SendTestMatrix.h>
#include <Modules/Basic/DynamicPortTester.h>
#include <Modules/Legacy/Bundle/GetFieldsFromBundle.h>
#include <Modules/Legacy/Bundle/GetMatricesFromBundle.h>
#include <Modules/Legacy/Bundle/InsertFieldsIntoBundle.h>
#include <Modules/Legacy/Bundle/InsertMatricesIntoBundle.h>
#include <Modules/Legacy/Fields/SplitFieldByDomain.h>
#include <Modules/Basic/PrintDatatype.h>
#include <Modules/Math/EvaluateLinearAlgebraUnary.h>
#include <Modules/Math/EvaluateLinearAlgebraBinary.h>
#include <Modules/Math/ReportMatrixInfo.h>
#include <Modules/Math/AppendMatrix.h>
#include <Modules/Math/CreateMatrix.h>
#include <Modules/Math/SolveLinearSystem.h>
#include <Modules/Fields/ReportFieldInfo.h>
#include <Modules/Legacy/Fields/CreateLatVol.h>
#include <Modules/Legacy/Fields/GetFieldBoundary.h>
#include <Modules/Legacy/Fields/CalculateSignedDistanceToField.h>
#include <Modules/Legacy/Fields/CalculateGradients.h>
#include <Modules/Legacy/Fields/ConvertQuadSurfToTriSurf.h>
#include <Modules/Legacy/Fields/AlignMeshBoundingBoxes.h>
#include <Modules/Legacy/Fields/GetFieldNodes.h>
#include <Modules/Legacy/Fields/SetFieldNodes.h>
#include <Modules/Legacy/Fields/GetDomainBoundary.h>
#include <Modules/Legacy/Fields/JoinFields.h>
#include <Modules/Legacy/Fields/SplitFieldByDomain.h>
#include <Modules/Legacy/Fields/CreateFieldData.h>
#include <Modules/Legacy/Fields/CalculateFieldData5.h>
#include <Modules/Legacy/Fields/SplitFieldByDomain.h>
#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Modules/Legacy/Fields/SetFieldData.h>
#include <Modules/Legacy/Fields/ResampleRegularMesh.h>
#include <Modules/Legacy/Fields/FairMesh.h>
#include <Modules/Legacy/Fields/ConvertFieldBasis.h>
#include <Modules/Legacy/Fields/ScaleFieldMeshAndData.h>
#include <Modules/Legacy/Fields/ProjectPointsOntoMesh.h>
#include <Modules/Legacy/Fields/CalculateDistanceToField.h>
#include <Modules/Legacy/Fields/CalculateDistanceToFieldBoundary.h>
#include <Modules/Legacy/Fields/MapFieldDataOntoElems.h>
#include <Modules/Legacy/Fields/MapFieldDataOntoNodes.h>
#include <Modules/Legacy/Fields/ClipFieldByFunction3.h>
#include <Modules/Legacy/Math/SolveMinNormLeastSqSystem.h>
#include <Modules/DataIO/ReadMatrix.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/DataIO/ReadField.h>
#include <Modules/DataIO/WriteField.h>
#include <Modules/String/CreateString.h>
#include <Modules/Visualization/ShowString.h>
#include <Modules/Visualization/ShowField.h>
#include <Modules/Visualization/CreateBasicColorMap.h>
#include <Modules/FiniteElements/TDCSSimulator.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Legacy/FiniteElements/BuildFEMatrix.h>
#include <Modules/Basic/AsyncPortTestModule.h>
#include <Modules/Basic/NeedToExecuteTester.h>
//#include <Modules/Fields/@ModuleName@.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::StringProcessing;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Modules::Render;

void ModuleDescriptionLookup::addEssentialModules()
{
  addModuleDesc<ReadMatrixModule>("ReadMatrix", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
  addModuleDesc<WriteMatrixModule>("WriteMatrix", "DataIO", "SCIRun", "Functional, outputs text files or binary .mat only.", "...");
  addModuleDesc<ReadFieldModule>("ReadField", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
  addModuleDesc<WriteFieldModule>("WriteField", "DataIO", "SCIRun", "Functional, outputs binary .fld only.", "...");
  addModuleDesc<PrintDatatypeModule>("PrintDatatype", "String", "SCIRun", "...", "...");
  addModuleDesc<ReportMatrixInfoModule>("ReportMatrixInfo", "Math", "SCIRun", "Functional, needs GUI work.", "...");
  addModuleDesc<ReportFieldInfoModule>("ReportFieldInfo", "MiscField", "SCIRun", "Same as v4", "...");
  addModuleDesc<AppendMatrixModule>("AppendMatrix", "Math", "SCIRun", "Fully functional.", "...");
  addModuleDesc<EvaluateLinearAlgebraUnaryModule>("EvaluateLinearAlgebraUnary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
  addModuleDesc<EvaluateLinearAlgebraBinaryModule>("EvaluateLinearAlgebraBinary", "Math", "SCIRun", "Partially functional, needs GUI work.", "...");
  addModuleDesc<CreateMatrixModule>("CreateMatrix", "Math", "SCIRun", "Functional, needs GUI work.", "...");
  addModuleDesc<SolveLinearSystemModule>("SolveLinearSystem", "Math", "SCIRun", "Four multi-threaded algorithms available.", "...");
  addModuleDesc<CreateStringModule>("CreateString", "String", "SCIRun", "Functional, needs GUI work.", "...");
  //addModuleDesc<ShowStringModule>("ShowString", "String", "SCIRun", "...", "...");
  addModuleDesc<ShowFieldModule>("Some basic options available, still work in progress.", "...");
  addModuleDesc<CreateLatVol>("CreateLatVol", "NewField", "SCIRun", "Official ported v4 module.", "...");
  addModuleDesc<ViewScene>("Can display meshes and fields, pan/rotate/zoom.", "...");

  addModuleDesc<GetFieldBoundary>("GetFieldBoundary", "NewField", "SCIRun", "First real ported module", "...");
  addModuleDesc<CalculateSignedDistanceToField>("CalculateSignedDistanceToField", "ChangeFieldData", "SCIRun", "Second real ported module", "...");
  addModuleDesc<CalculateGradients>("CalculateGradients", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<ConvertQuadSurfToTriSurf>("ConvertQuadSurfToTriSurf", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<AlignMeshBoundingBoxes>("AlignMeshBoundingBoxes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<GetFieldNodes>("GetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<SetFieldNodes>("SetFieldNodes", "ChangeMesh", "SCIRun", "Real ported module", "...");
  addModuleDesc<TDCSSimulatorModule>("tDCSSimulator", "FiniteElements", "SCIRun", "Dummy module for design purposes", "...");
  addModuleDesc<SolveMinNormLeastSqSystem>("SolveMinNormLeastSqSystem", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<CreateBasicColorMap>("CreateStandardColorMap", "Visualization", "SCIRun", "In progress", "...");
  addModuleDesc<GetDomainBoundary>("Real ported module: Many bugs and UI logic issues", "...");
  addModuleDesc<JoinFields>("Real ported module: Many bugs and UI logic issues", "...");
  addModuleDesc<CreateFieldData>("Real ported module", "...");
  addModuleDesc<CalculateFieldData>("Real ported module", "...");
  addModuleDesc<BuildFEMatrix>("BuildFEMatrix", "FiniteElements", "SCIRun", " in progress ", "Generates stiffness matrix ");
  addModuleDesc<ResampleRegularMesh>("Real ported module", "...");
  addModuleDesc<FairMesh>("Real ported module", "...");
  addModuleDesc<ConvertFieldBasis>("NOT WORKING YET--Real ported module", "...");
  addModuleDesc<ScaleFieldMeshAndData>("NOT WORKING YET--Real ported module", "...");
  addModuleDesc<ProjectPointsOntoMesh>("Real ported module", "...");
  addModuleDesc<CalculateDistanceToField>("Real ported module", "...");
  addModuleDesc<CalculateDistanceToFieldBoundary>("Real ported module", "...");
  addModuleDesc<MapFieldDataOntoNodes>("Real ported module", "...");
  addModuleDesc<MapFieldDataOntoElements>("Real ported module", "...");
  addModuleDesc<ClipFieldByFunction>("Real ported module", "...");
  
  // insert module desc here
}

void ModuleDescriptionLookup::addBundleModules()
{
  //addModuleDesc<GetMatricesFromBundle>("Real ported module: improved UI", "...");
  //addModuleDesc<InsertMatricesIntoBundle>("Real ported module: improved UI", "...");
  addModuleDesc<GetFieldsFromBundle>("Real ported module: improved UI", "...");
  addModuleDesc<InsertFieldsIntoBundle>("Real ported module: improved UI", "...");
  addModuleDesc<SplitFieldByDomain>("Real ported module", "...");
}

void ModuleDescriptionLookup::addTestingModules()
{
  addModuleDesc<SendScalarModule>("SendScalar", "Testing", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
  addModuleDesc<ReceiveScalarModule>("ReceiveScalar", "Testing", "SCIRun", "...", "...");
  addModuleDesc<SendTestMatrixModule>("SendTestMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<ReceiveTestMatrixModule>("ReceiveTestMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<DynamicPortTester>("DynamicPortTester", "Testing", "SCIRun", "...", "...");
  addModuleDesc<AsyncPortTestModule>("...", "ViewScene clone");
  addModuleDesc<NeedToExecuteTester>("...", "...");
}
