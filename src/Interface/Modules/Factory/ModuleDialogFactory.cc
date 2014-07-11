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

#include <Interface/Modules/Factory/ModuleDialogFactory.h>
#include <Interface/Modules/Base/ModuleDialogBasic.h>
#include <Interface/Modules/Testing/SendScalarDialog.h>
#include <Interface/Modules/Testing/ReceiveScalarDialog.h>
#include <Interface/Modules/DataIO/ReadMatrixDialog.h>
#include <Interface/Modules/DataIO/WriteMatrixDialog.h>
#include <Interface/Modules/DataIO/ReadMeshDialog.h>
#include <Interface/Modules/DataIO/ReadFieldDialog.h>
#include <Interface/Modules/DataIO/WriteFieldDialog.h>
#include <Interface/Modules/Math/EvaluateLinearAlgebraUnaryDialog.h>
#include <Interface/Modules/Math/EvaluateLinearAlgebraBinaryDialog.h>
#include <Interface/Modules/Math/ReportMatrixInfoDialog.h>
#include <Interface/Modules/Math/CreateMatrixDialog.h>
#include <Interface/Modules/Math/AppendMatrixDialog.h>
#include <Interface/Modules/Math/SolveLinearSystemDialog.h>
#include <Interface/Modules/Math/SelectSubMatrixDialog.h>
#include <Interface/Modules/Math/ConvertMatrixTypeDialog.h>
#include <Interface/Modules/String/CreateStringDialog.h>
#include <Interface/Modules/String/PrintDatatypeDialog.h>
#include <Interface/Modules/Fields/CreateLatVolDialog.h>
#include <Interface/Modules/Fields/GetDomainBoundaryDialog.h>
#include <Interface/Modules/Fields/ReportFieldInfoDialog.h>
#include <Interface/Modules/Fields/JoinFieldsDialog.h>
#include <Interface/Modules/Fields/SplitFieldByDomainDialog.h>
#include <Interface/Modules/Fields/SetFieldDataDialog.h>
#include <Interface/Modules/Fields/InterfaceWithCleaverDialog.h>
#include <Interface/Modules/Fields/MapFieldDataFromElemToNodeDialog.h>
#include <Interface/Modules/Fields/MapFieldDataFromNodeToElemDialog.h>
#include <Interface/Modules/Fields/CreateFieldDataDialog.h>
#include <Interface/Modules/Fields/CalculateFieldDataDialog.h>
#include <Interface/Modules/Fields/ResampleRegularMeshDialog.h>
#include <Interface/Modules/Fields/FairMeshDialog.h>
#include <Interface/Modules/Fields/ProjectPointsOntoMeshDialog.h>
#include <Interface/Modules/Fields/CalculateDistanceToFieldDialog.h>
#include <Interface/Modules/Fields/CalculateDistanceToFieldBoundaryDialog.h>
#include <Interface/Modules/Fields/MapFieldDataOntoElemsDialog.h>
#include <Interface/Modules/Fields/MapFieldDataOntoNodesDialog.h>
#include <Interface/Modules/Visualization/MatrixAsVectorFieldDialog.h>
#include <Interface/Modules/Visualization/ShowStringDialog.h>
#include <Interface/Modules/Visualization/ShowFieldDialog.h>
#include <Interface/Modules/Visualization/CreateBasicColorMapDialog.h>
#include <Interface/Modules/Render/ViewScene.h>
#include <Interface/Modules/Bundle/InsertFieldsIntoBundleDialog.h>
#include <Interface/Modules/Bundle/GetFieldsFromBundleDialog.h>

#include <boost/assign.hpp>
#include <boost/functional/factory.hpp>
#include <boost/foreach.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

ModuleDialogFactory::ModuleDialogFactory(QWidget* parentToUse) : parentToUse_(parentToUse)
{
  addDialogsToMakerMap1();
  addDialogsToMakerMap2();
}

void ModuleDialogFactory::addDialogsToMakerMap1()
{
  insert(dialogMakerMap_)
    ADD_MODULE_DIALOG(SendScalar, SendScalarDialog)
    ADD_MODULE_DIALOG(ReceiveScalar, ReceiveScalarDialog)
    ADD_MODULE_DIALOG(ReadMatrix, ReadMatrixDialog)
    ADD_MODULE_DIALOG(WriteMatrix, WriteMatrixDialog)
    ADD_MODULE_DIALOG(ReadMesh, ReadMeshDialog)
    ADD_MODULE_DIALOG(ReadField, ReadFieldDialog)
    ADD_MODULE_DIALOG(WriteField, WriteFieldDialog)
    ADD_MODULE_DIALOG(EvaluateLinearAlgebraUnary, EvaluateLinearAlgebraUnaryDialog)
    ADD_MODULE_DIALOG(EvaluateLinearAlgebraBinary, EvaluateLinearAlgebraBinaryDialog)
    ADD_MODULE_DIALOG(ShowString, ShowStringDialog)
    ADD_MODULE_DIALOG(ShowField, ShowFieldDialog)
    ADD_MODULE_DIALOG(AppendMatrix, AppendMatrixDialog)
    ADD_MODULE_DIALOG(CreateMatrix, CreateMatrixDialog)
    ADD_MODULE_DIALOG(CreateString, CreateStringDialog)
    ADD_MODULE_DIALOG(PrintDatatype, PrintDatatypeDialog)
    ADD_MODULE_DIALOG(ReportMatrixInfo, ReportMatrixInfoDialog)
    ADD_MODULE_DIALOG(ReportFieldInfo, ReportFieldInfoDialog)
    ADD_MODULE_DIALOG(MatrixAsVectorField, MatrixAsVectorFieldDialog)
    ADD_MODULE_DIALOG(ViewScene, ViewSceneDialog)
    ADD_MODULE_DIALOG(SolveLinearSystem, SolveLinearSystemDialog)
    ADD_MODULE_DIALOG(CreateLatVol, CreateLatVolDialog)
    ADD_MODULE_DIALOG(CreateStandardColorMap, CreateBasicColorMapDialog)
    ADD_MODULE_DIALOG(GetDomainBoundary, GetDomainBoundaryDialog)
    ADD_MODULE_DIALOG(JoinFields, JoinFieldsDialog)
    ADD_MODULE_DIALOG(InsertFieldsIntoBundle, InsertFieldsIntoBundleDialog)
    ADD_MODULE_DIALOG(GetFieldsFromBundle, GetFieldsFromBundleDialog)
    ADD_MODULE_DIALOG(SplitFieldByDomain, SplitFieldByDomainDialog)
    ADD_MODULE_DIALOG(CreateFieldData, CreateFieldDataDialog)
    ADD_MODULE_DIALOG(CalculateFieldData, CalculateFieldDataDialog)
    ADD_MODULE_DIALOG(SetFieldData, SetFieldDataDialog)
    ADD_MODULE_DIALOG(InterfaceWithCleaver, InterfaceWithCleaverDialog)
    ADD_MODULE_DIALOG(SelectSubMatrix, SelectSubMatrixDialog)
    ADD_MODULE_DIALOG(MapFieldDataFromElemToNode, MapFieldDataFromElemToNodeDialog)
    ADD_MODULE_DIALOG(InsertFieldsIntoBundle, InsertFieldsIntoBundleDialog)
    ADD_MODULE_DIALOG(GetFieldsFromBundle, GetFieldsFromBundleDialog)
    ADD_MODULE_DIALOG(SplitFieldByDomain, SplitFieldByDomainDialog)
    ADD_MODULE_DIALOG(ConvertMatrixType, ConvertMatrixTypeDialog)
    ADD_MODULE_DIALOG(MapFieldDataFromNodeToElem, MapFieldDataFromNodeToElemDialog)
    ADD_MODULE_DIALOG(ResampleRegularMesh, ResampleRegularMeshDialog)
    ADD_MODULE_DIALOG(FairMesh, FairMeshDialog)
    ADD_MODULE_DIALOG(ProjectPointsOntoMesh, ProjectPointsOntoMeshDialog)
    ADD_MODULE_DIALOG(CalculateDistanceToField, CalculateDistanceToFieldDialog)
    ADD_MODULE_DIALOG(CalculateDistanceToFieldBoundary, CalculateDistanceToFieldBoundaryDialog)
    ADD_MODULE_DIALOG(MapFieldDataOntoElems, MapFieldDataOntoElemsDialog)
    ADD_MODULE_DIALOG(MapFieldDataOntoNodes, MapFieldDataOntoNodesDialog)
  ;
}

ModuleDialogGeneric* ModuleDialogFactory::makeDialog(const std::string& moduleId, ModuleStateHandle state)
{
  BOOST_FOREACH(const DialogMakerMap::value_type& makerPair, dialogMakerMap_)
  {
    if (moduleId.find(makerPair.first) != std::string::npos)
      return makerPair.second(moduleId, state, parentToUse_);
  }

  return new ModuleDialogBasic(moduleId, parentToUse_);
}
