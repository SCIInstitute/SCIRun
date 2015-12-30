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

#include <Interface/Modules/Factory/ModuleDialogFactory.h>
#include <Interface/Modules/Base/ModuleDialogBasic.h>
#include <Interface/Modules/Testing/SendScalarDialog.h>
#include <Interface/Modules/Testing/ReceiveScalarDialog.h>
#include <Interface/Modules/DataIO/ReadMatrixClassicDialog.h>
#include <Interface/Modules/DataIO/ReadBundleDialog.h>
#include <Interface/Modules/DataIO/WriteMatrixDialog.h>
#include <Interface/Modules/DataIO/ReadFieldDialog.h>
#include <Interface/Modules/DataIO/ReadNrrdDialog.h>
#include <Interface/Modules/DataIO/WriteFieldDialog.h>
#include <Interface/Modules/Math/EvaluateLinearAlgebraUnaryDialog.h>
#include <Interface/Modules/Math/EvaluateLinearAlgebraBinaryDialog.h>
#include <Interface/Modules/Math/EvaluateLinearAlgebraGeneralDialog.h>
#include <Interface/Modules/Math/ReportMatrixInfoDialog.h>
#include <Interface/Modules/Math/CreateMatrixDialog.h>
#include <Interface/Modules/Math/AppendMatrixDialog.h>
#include <Interface/Modules/Math/SolveLinearSystemDialog.h>
#include <Interface/Modules/Math/ReportColumnMatrixMisfitDialog.h>
#include <Interface/Modules/Math/SelectSubMatrixDialog.h>
#include <Interface/Modules/Math/ConvertMatrixTypeDialog.h>
#include <Interface/Modules/Math/GetMatrixSliceDialog.h>
#include <Interface/Modules/Math/BuildNoiseColumnMatrixDialog.h>
#include <Interface/Modules/Math/CollectMatricesDialog.h>
#include <Interface/Modules/String/CreateStringDialog.h>
#include <Interface/Modules/String/NetworkNotesDialog.h>
#include <Interface/Modules/String/PrintDatatypeDialog.h>
#include <Interface/Modules/Fields/CreateLatVolDialog.h>
#include <Interface/Modules/Fields/EditMeshBoundingBoxDialog.h>
#include <Interface/Modules/Fields/GetDomainBoundaryDialog.h>
#include <Interface/Modules/Fields/ReportFieldInfoDialog.h>
#include <Interface/Modules/Fields/JoinFieldsDialog.h>
#include <Interface/Modules/Fields/SplitFieldByDomainDialog.h>
#include <Interface/Modules/Fields/SplitFieldByConnectedRegionDialog.h>
#include <Interface/Modules/Fields/SetFieldDataDialog.h>
#include <Interface/Modules/Fields/InterfaceWithCleaverDialog.h>
#include <Interface/Modules/Fields/MapFieldDataFromElemToNodeDialog.h>
#include <Interface/Modules/Fields/MapFieldDataFromNodeToElemDialog.h>
#include <Interface/Modules/Fields/GetSliceFromStructuredFieldByIndicesDialog.h>
#include <Interface/Modules/Fields/GenerateSinglePointProbeFromFieldDialog.h>
#include <Interface/Modules/Fields/GeneratePointSamplesFromFieldDialog.h>
#include <Interface/Modules/Fields/GeneratePointSamplesFromFieldOrWidgetDialog.h>
#include <Interface/Modules/Fields/CreateFieldDataDialog.h>
#include <Interface/Modules/Fields/CalculateFieldDataDialog.h>
#include <Interface/Modules/Fields/ResampleRegularMeshDialog.h>
#include <Interface/Modules/Fields/FairMeshDialog.h>
#if WITH_TETGEN
#include <Interface/Modules/Fields/InterfaceWithTetGenDialog.h>
#endif
#include <Interface/Modules/Fields/ProjectPointsOntoMeshDialog.h>
#include <Interface/Modules/Fields/CalculateDistanceToFieldDialog.h>
#include <Interface/Modules/Fields/CalculateDistanceToFieldBoundaryDialog.h>
#include <Interface/Modules/Fields/MapFieldDataOntoElemsDialog.h>
#include <Interface/Modules/Fields/MapFieldDataOntoNodesDialog.h>
#include <Interface/Modules/Fields/MapFieldDataFromSourceToDestinationDialog.h>
#include <Interface/Modules/Fields/ClipFieldByFunctionDialog.h>
#include <Interface/Modules/Fields/BuildMappingMatrixDialog.h>
#include <Interface/Modules/Fields/RefineMeshDialog.h>
#include <Interface/Modules/Fields/SetFieldDataToConstantValueDialog.h>
#include <Interface/Modules/Fields/ConvertFieldBasisDialog.h>
#include <Interface/Modules/Fields/SwapFieldDataWithMatrixEntriesDialog.h>
#include <Interface/Modules/Fields/ConvertIndicesToFieldDataDialog.h>
#include <Interface/Modules/Fields/RegisterWithCorrespondencesDialog.h>
#include <Interface/Modules/Forward/BuildBEMatrixDialog.h>
#include <Interface/Modules/Inverse/SolveInverseProblemWithTikhonovDialog.h>
#include <Interface/Modules/FiniteElements/ApplyFEMCurrentSourceDialog.h>
#include <Interface/Modules/Visualization/MatrixAsVectorFieldDialog.h>
#include <Interface/Modules/Visualization/ShowStringDialog.h>
#include <Interface/Modules/Visualization/ShowFieldDialog.h>
#include <Interface/Modules/Visualization/ShowFieldGlyphsDialog.h>
#include <Interface/Modules/Visualization/CreateStandardColorMapDialog.h>
#include <Interface/Modules/Visualization/ShowColorMapDialog.h>
#include <Interface/Modules/Visualization/RescaleColorMapDialog.h>
#include <Interface/Modules/Matlab/ImportDatatypesFromMatlabDialog.h>
#include <Interface/Modules/Render/ViewScene.h>
#include <Interface/Modules/Bundle/InsertFieldsIntoBundleDialog.h>
#include <Interface/Modules/Bundle/GetFieldsFromBundleDialog.h>
#include <Interface/Modules/Bundle/ReportBundleInfoDialog.h>
#include <Interface/Modules/Teem/ReportNrrdInfoDialog.h>
#include <Interface/Modules/Teem/ConvertNrrdToFieldDialog.h>
#include <Interface/Modules/Teem/ConvertNrrdToMatrixDialog.h>
#include <Interface/Modules/Fields/ExtractSimpleIsosurfaceDialog.h>
#include <Interface/Modules/Fields/ClipVolumeByIsovalueDialog.h>
#include <boost/assign.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

ModuleDialogFactory::ModuleDialogFactory(QWidget* parentToUse,
  ExecutionDisablingServiceFunction disablerAdd,
  ExecutionDisablingServiceFunction disablerRemove) :
  parentToUse_(parentToUse)
{
  addDialogsToMakerMapGenerated();
  addDialogsToMakerMap1();
  addDialogsToMakerMap2();

  ModuleDialogGeneric::setExecutionDisablingServiceFunctionAdd(disablerAdd);
  ModuleDialogGeneric::setExecutionDisablingServiceFunctionRemove(disablerRemove);
}

void ModuleDialogFactory::addDialogsToMakerMap1()
{
  insert(dialogMakerMap_)
    ADD_MODULE_DIALOG(SendScalar, SendScalarDialog)
    ADD_MODULE_DIALOG(ReceiveScalar, ReceiveScalarDialog)
    ADD_MODULE_DIALOG(ReadMatrix, ReadMatrixClassicDialog)
    ADD_MODULE_DIALOG(WriteMatrix, WriteMatrixDialog)
    ADD_MODULE_DIALOG(ReadField, ReadFieldDialog)
    ADD_MODULE_DIALOG(WriteField, WriteFieldDialog)
    ADD_MODULE_DIALOG(ReadBundle, ReadBundleDialog)
    ADD_MODULE_DIALOG(EvaluateLinearAlgebraUnary, EvaluateLinearAlgebraUnaryDialog)
    ADD_MODULE_DIALOG(EvaluateLinearAlgebraBinary, EvaluateLinearAlgebraBinaryDialog)
    ADD_MODULE_DIALOG(EvaluateLinearAlgebraGeneral, EvaluateLinearAlgebraGeneralDialog)
    ADD_MODULE_DIALOG(ShowString, ShowStringDialog)
    ADD_MODULE_DIALOG(ShowField, ShowFieldDialog)
    ADD_MODULE_DIALOG(ShowFieldGlyphs, ShowFieldGlyphsDialog)
    ADD_MODULE_DIALOG(AppendMatrix, AppendMatrixDialog)
    ADD_MODULE_DIALOG(CreateMatrix, CreateMatrixDialog)
    ADD_MODULE_DIALOG(CreateString, CreateStringDialog)
    ADD_MODULE_DIALOG(NetworkNotes, NetworkNotesDialog)
    ADD_MODULE_DIALOG(PrintDatatype, PrintDatatypeDialog)
    ADD_MODULE_DIALOG(ReportMatrixInfo, ReportMatrixInfoDialog)
    ADD_MODULE_DIALOG(ReportFieldInfo, ReportFieldInfoDialog)
    ADD_MODULE_DIALOG(ReportBundleInfo, ReportBundleInfoDialog)
    ADD_MODULE_DIALOG(MatrixAsVectorField, MatrixAsVectorFieldDialog)
    ADD_MODULE_DIALOG(ViewScene, ViewSceneDialog)
    ADD_MODULE_DIALOG(SolveLinearSystem, SolveLinearSystemDialog)
    ADD_MODULE_DIALOG(CreateStandardColorMap, CreateStandardColorMapDialog)
    ADD_MODULE_DIALOG(JoinFields, JoinFieldsDialog)
    ADD_MODULE_DIALOG(GetFieldsFromBundle, GetFieldsFromBundleDialog)
    ADD_MODULE_DIALOG(SplitFieldByDomain, SplitFieldByDomainDialog)
    ADD_MODULE_DIALOG(CreateFieldData, CreateFieldDataDialog)
    ADD_MODULE_DIALOG(CalculateFieldData, CalculateFieldDataDialog)
    ADD_MODULE_DIALOG(SetFieldData, SetFieldDataDialog)
    ADD_MODULE_DIALOG(InterfaceWithCleaver, InterfaceWithCleaverDialog)
    ADD_MODULE_DIALOG(SelectSubMatrix, SelectSubMatrixDialog)
    ADD_MODULE_DIALOG(GetMatrixSlice, GetMatrixSliceDialog)
    ADD_MODULE_DIALOG(MapFieldDataFromElemToNode, MapFieldDataFromElemToNodeDialog)
    ADD_MODULE_DIALOG(GetFieldsFromBundle, GetFieldsFromBundleDialog)
    ADD_MODULE_DIALOG(SplitFieldByDomain, SplitFieldByDomainDialog)
    ADD_MODULE_DIALOG(ConvertMatrixType, ConvertMatrixTypeDialog)
    ADD_MODULE_DIALOG(MapFieldDataFromNodeToElem, MapFieldDataFromNodeToElemDialog)
    ADD_MODULE_DIALOG(ResampleRegularMesh, ResampleRegularMeshDialog)
    ADD_MODULE_DIALOG(FairMesh, FairMeshDialog)
    ADD_MODULE_DIALOG(BuildBEMatrix, BuildBEMatrixDialog)
    ADD_MODULE_DIALOG(ApplyFEMCurrentSource, ApplyFEMCurrentSourceDialog)
    ADD_MODULE_DIALOG(ProjectPointsOntoMesh, ProjectPointsOntoMeshDialog)
    ADD_MODULE_DIALOG(CalculateDistanceToField, CalculateDistanceToFieldDialog)
    ADD_MODULE_DIALOG(CalculateDistanceToFieldBoundary, CalculateDistanceToFieldBoundaryDialog)
#if WITH_TETGEN
    ADD_MODULE_DIALOG(InterfaceWithTetGen, InterfaceWithTetGenDialog)
#endif
    ADD_MODULE_DIALOG(MapFieldDataOntoElements, MapFieldDataOntoElemsDialog)
    ADD_MODULE_DIALOG(MapFieldDataOntoNodes, MapFieldDataOntoNodesDialog)
    ADD_MODULE_DIALOG(MapFieldDataFromSourceToDestination, MapFieldDataFromSourceToDestinationDialog)
    ADD_MODULE_DIALOG(SplitFieldByConnectedRegion, SplitFieldByConnectedRegionDialog)
    ADD_MODULE_DIALOG(ClipFieldByFunction, ClipFieldByFunctionDialog)
    ADD_MODULE_DIALOG(ImportDatatypesFromMatlab, ImportDatatypesFromMatlabDialog)
    ADD_MODULE_DIALOG(RefineMesh, RefineMeshDialog)
    ADD_MODULE_DIALOG(ReportColumnMatrixMisfit, ReportColumnMatrixMisfitDialog)
    ADD_MODULE_DIALOG(SetFieldDataToConstantValue, SetFieldDataToConstantValueDialog)
    ADD_MODULE_DIALOG(ConvertFieldBasis, ConvertFieldBasisDialog)
    ADD_MODULE_DIALOG(BuildNoiseColumnMatrix, BuildNoiseColumnMatrixDialog)
    ADD_MODULE_DIALOG(SwapFieldDataWithMatrixEntries, SwapFieldDataWithMatrixEntriesDialog)
    ADD_MODULE_DIALOG(BuildMappingMatrix, BuildMappingMatrixDialog)
    ADD_MODULE_DIALOG(CollectMatrices, CollectMatricesDialog)
    ADD_MODULE_DIALOG(EditMeshBoundingBox, EditMeshBoundingBoxDialog)
    ADD_MODULE_DIALOG(GenerateSinglePointProbeFromField, GenerateSinglePointProbeFromFieldDialog)
    ADD_MODULE_DIALOG(GeneratePointSamplesFromField, GeneratePointSamplesFromFieldDialog)
    ADD_MODULE_DIALOG(GeneratePointSamplesFromFieldOrWidget, GeneratePointSamplesFromFieldOrWidgetDialog)
    ADD_MODULE_DIALOG(GetSliceFromStructuredFieldByIndices, GetSliceFromStructuredFieldByIndicesDialog)
    ADD_MODULE_DIALOG(ConvertIndicesToFieldData, ConvertIndicesToFieldDataDialog)
    ADD_MODULE_DIALOG(SolveInverseProblemWithTikhonov, SolveInverseProblemWithTikhonovDialog)
    ADD_MODULE_DIALOG(ShowColorMap, ShowColorMapDialog)
    ADD_MODULE_DIALOG(RescaleColorMap, RescaleColorMapDialog)
    ADD_MODULE_DIALOG(ExtractSimpleIsosurface, ExtractSimpleIsosurfaceDialog)
    ADD_MODULE_DIALOG(RegisterWithCorrespondences, RegisterWithCorrespondencesDialog)
    ADD_MODULE_DIALOG(ClipVolumeByIsovalue, ClipVolumeByIsovalueDialog)
  ;
}

ModuleDialogGeneric* ModuleDialogFactory::makeDialog(const std::string& moduleId, ModuleStateHandle state)
{
  for(const auto& makerPair : dialogMakerMap_)
  {
    //TODO: match full string name; need to strip module id's number
    auto findIndex = moduleId.find(makerPair.first);
    if (findIndex != std::string::npos && moduleId[makerPair.first.size()] == ':')
      return makerPair.second(moduleId, state, parentToUse_);
  }

  QMessageBox::critical(nullptr, "Module/Dialog Inconsistency", "The module with ID \"" +
    QString::fromStdString(moduleId) + "\" cannot find its dialog implementation. SCIRun is constructing a basic dialog so your network still is functional. Please update your network file by hand.");
  return new ModuleDialogBasic(moduleId, parentToUse_);
}
