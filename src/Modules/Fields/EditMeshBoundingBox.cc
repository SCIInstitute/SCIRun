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

#include "Core/Datatypes/MatrixFwd.h"
#include "Dataflow/Network/ModuleStateInterface.h"
#include "Graphics/Widgets/BoundingBoxWidget.h"
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/WidgetFactory.h>
#include <Modules/Fields/EditMeshBoundingBox.h>
#include <boost/format.hpp>

using namespace SCIRun;
using namespace Modules::Fields;
using namespace Core::Datatypes;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Graphics;
using namespace Datatypes;

MODULE_INFO_DEF(EditMeshBoundingBox, ChangeMesh, SCIRun)

EditMeshBoundingBox::EditMeshBoundingBox()
: GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Transformation_Widget);
  INITIALIZE_PORT(Transformation_Matrix);
  mFirstRun = true;
  mWidgetMoved = true;
  mWidgetAxesRotated = false;
}

void EditMeshBoundingBox::processWidgetFeedback(const ModuleFeedback& var)
{
  try
  {
    auto vsf = dynamic_cast<const ViewSceneFeedback&>(var);
    if (vsf.matchesWithModuleId(id()))
    {
      mWidgetMoved = true;
      adjustGeometryFromTransform(vsf.buttonClicked, vsf.movementType, vsf.transform);
      enqueueExecuteAgain(true);
    }
  }
  catch (std::bad_cast&)
  {
    //ignore
  }
}

void EditMeshBoundingBox::adjustGeometryFromTransform(MouseButton btn, WidgetMovement move, const Transform& feedbackTrans)
{
  if(btn == MouseButton::RIGHT && move == WidgetMovement::ROTATE)
  {
    mWidgetAxesRotated = true;
  }
  else
  {
    mFieldTrans = feedbackTrans * mFieldTrans;
    mTrans = feedbackTrans * mTrans;
    mTrans.orthogonalize();
  }
  if(move == WidgetMovement::ROTATE)
  {
    mWidgetAxesOrthonormal = feedbackTrans * mWidgetAxesOrthonormal;
    mWidgetAxesOrthonormal.orthogonalize();
  }
  mWidgetAxes = feedbackTrans * mWidgetAxes;
  mWidgetAxes.orthogonalize();
}

void EditMeshBoundingBox::clearVals()
{
  auto state = get_state();
  const std::string cleared("---");
  state->setValue(InputCenterX, cleared);
  state->setValue(InputCenterY, cleared);
  state->setValue(InputCenterZ, cleared);
  state->setValue(InputSizeX, cleared);
  state->setValue(InputSizeY, cleared);
  state->setValue(InputSizeZ, cleared);
}

void EditMeshBoundingBox::setStateDefaults()
{
  clearVals();
  auto state = get_state();
  state->setValue(TransformMatrix, VariableList());
  state->setValue(FieldTransformMatrix, VariableList());
  state->setValue(DataSaved, false);
  state->setValue(Scale, 1.0);
  state->setValue(OldScale, 1.0);
  state->setValue(ScaleChanged, false);
  state->setValue(ResetToInput, true);
  state->setValue(SetOutputCenter, false);
  state->setValue(RestrictX, false);
  state->setValue(RestrictY, false);
  state->setValue(RestrictZ, false);
  state->setValue(RestrictR, false);
  state->setValue(RestrictD, false);
  state->setValue(RestrictI, false);
  state->setValue(OutputCenterX, 0.0);
  state->setValue(OutputCenterY, 0.0);
  state->setValue(OutputCenterZ, 0.0);
  state->setValue(OutputSizeX, 1.0);
  state->setValue(OutputSizeY, 1.0);
  state->setValue(OutputSizeZ, 1.0);
  state->setValue(Scale, 0.1);
  state->setValue(NoTranslation, true);
  state->setValue(XYZTranslation, false);
  state->setValue(RDITranslation, false);
  state->setValue(BoxRealScale, 0.0);
  state->setValue(BoxMode, 0);

  getOutputPort(Transformation_Widget)->connectConnectionFeedbackListener([this]
    (const ModuleFeedback& var) { processWidgetFeedback(var); });
}

void EditMeshBoundingBox::execute()
{
  auto field = getRequiredInput(InputField);
  if (needToExecute() || mWidgetMoved)
  {
    updateState(field);
    sendOutputPorts();
    saveToParameters();
  }
}

void EditMeshBoundingBox::updateState(FieldHandle field)
{
  mOutputField = FieldHandle(field->deep_clone());
  auto state = get_state();
  mWidgetMoved = false;
  bool inputResetRequested = transient_value_cast<bool>(state->getTransientValue(ResetToInput));

  if(inputsChanged() || inputResetRequested)
  {
    mTrans = Transform();
    mWidgetAxes = Transform();
    mWidgetAxesOrthonormal = Transform();
  }
  if (mFirstRun || inputsChanged() || inputResetRequested || mWidgetAxesRotated)
  {
    auto ogPos = mTrans.get_translation_point();
    auto fh = FieldHandle(field->deep_clone());

    fh->vmesh()->transform(mWidgetAxes);
    computeWidgetBox(fh->vmesh()->get_bounding_box());

    mTrans = mWidgetAxesOrthonormal * Transform(Point(0,0,0), mEigvecs[0]*mEigvals[0], mEigvecs[1]*mEigvals[1], mEigvecs[2]*mEigvals[2]);
    for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
      mTrans.set_mat_val(iDim, 3, ogPos[iDim]);
    mWidgetAxesRotated = false;
  }
  if (mFirstRun && state->getValue(DataSaved).toBool())
  {
    mFirstRun = false;
    loadFromParameters();
    updateInputFieldAttributes();
  }
  else if (inputsChanged() || inputResetRequested)
  {
    state->setTransientValue(ResetToInput, false);
    resetToInputField();
  }

  // Sets the translation vector in the homogeneous matrices
  if(transient_value_cast<bool>(state->getTransientValue(SetOutputCenter)))
  {
    state->setTransientValue(SetOutputCenter, false);

    mTrans.set_mat_val(0, 3, state->getValue(OutputCenterX).toDouble());
    mTrans.set_mat_val(1, 3, state->getValue(OutputCenterY).toDouble());
    mTrans.set_mat_val(2, 3, state->getValue(OutputCenterZ).toDouble());

    // Vector multiplication with Transform only does rotation, not translation
    auto rotatedPositionVec = mFieldTrans * Vector(mPos);
    mFieldTrans.set_mat_val(0, 3, state->getValue(OutputCenterX).toDouble() - rotatedPositionVec[0]);
    mFieldTrans.set_mat_val(1, 3, state->getValue(OutputCenterY).toDouble() - rotatedPositionVec[1]);
    mFieldTrans.set_mat_val(2, 3, state->getValue(OutputCenterZ).toDouble() - rotatedPositionVec[2]);
  }
  else if (transient_value_cast<bool>(state->getTransientValue(ResetCenter)))
  {
    state->setTransientValue(ResetCenter, false);

    for(int iDim = 0; iDim < mDIMENSIONS; ++iDim)
    {
      mTrans.set_mat_val(iDim, 3, mPos[iDim]);
      mFieldTrans.set_mat_val(iDim, 3, 0);
    }
  }
}

void EditMeshBoundingBox::sendOutputPorts()
{
  sendOutput(Transformation_Matrix, MatrixHandle(new DenseMatrix(mTrans)));
  mOutputField->vmesh()->transform(mFieldTrans);
  sendOutput(OutputField, mOutputField);

  generateGeomsList();
  auto geo = createGeomComposite(*this, "bbox", mGeoms.begin(), mGeoms.end());
  sendOutput(Transformation_Widget, geo);
}

void EditMeshBoundingBox::resetToInputField()
{
  mFieldTrans = Transform(); // Identity matrix
  // mTrans = Transform(mPos, mEigvecs[0]*mEigvals[0], mEigvecs[1]*mEigvals[1], mEigvecs[2]*mEigvals[2]);
  updateInputFieldAttributes();
}

std::string EditMeshBoundingBox::convertForLabel(double coord)
{
  return str(boost::format("%8.4f") % coord);
}


void EditMeshBoundingBox::computeWidgetBox(const BBox& box)
{
  auto bbox(box);
  if (!bbox.valid())
  {
    warning("Input field is empty -- using unit cube.");
    bbox.extend(Point(0, 0, 0));
    bbox.extend(Point(1, 1, 1));
  }

  // build a widget identical to the BBox
  auto diag = bbox.diagonal();
  const auto SMALL = 1e-4;
  if (fabs(diag.x())<SMALL)
  {
    diag.x(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(SMALL, 0.0, 0.0));
    bbox.extend(bbox.get_max() + Vector(SMALL, 0.0, 0.0));
  }
  if (fabs(diag.y())<SMALL)
  {
    diag.y(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, SMALL, 0.0));
    bbox.extend(bbox.get_max() + Vector(0.0, SMALL, 0.0));
  }
  if (fabs(diag.z())<SMALL)
  {
    diag.z(2 * SMALL);
    bbox.extend(bbox.get_min() - Vector(0.0, 0.0, SMALL));
    bbox.extend(bbox.get_max() + Vector(0.0, 0.0, SMALL));
  }

  mEigvals.resize(mDIMENSIONS);
  mEigvecs.resize(mDIMENSIONS);
  mPos = bbox.center();
  mEigvals[0] = diag.x() * 0.5;
  mEigvals[1] = diag.y() * 0.5;
  mEigvals[2] = diag.z() * 0.5;
  mEigvecs[0] = Vector(1,0,0);
  mEigvecs[1] = Vector(0,1,0);
  mEigvecs[2] = Vector(0,0,1);
}

namespace
{
  void checkForVerySmall(double& size)
  {
    const auto VERY_SMALL = 1e-12;
    if (size < VERY_SMALL) size = 1.0;
  }
}

void EditMeshBoundingBox::generateGeomsList()
{
  static int widgetNum = 0;
  const auto bboxWidget = WidgetFactory::createBoundingBox(*this, "EMBB",
    get_state()->getValue(Scale).toDouble(), mTrans, mTrans.get_translation_point(), widgetNum++);

  mGeoms.clear();
  for (const auto& widget : bboxWidget->mWidgets)
    mGeoms.push_back(widget);
}

void EditMeshBoundingBox::loadFromParameters()
{
  auto state = get_state();
  mTrans = transformFromString(state->getValue(TransformMatrix).toString());
  mFieldTrans = transformFromString(state->getValue(FieldTransformMatrix).toString());
}

void EditMeshBoundingBox::updateInputFieldAttributes()
{
  auto state = get_state();
  state->setValue(InputCenterX, convertForLabel(mPos[0]));
  state->setValue(InputCenterY, convertForLabel(mPos[1]));
  state->setValue(InputCenterZ, convertForLabel(mPos[2]));
  state->setValue(InputSizeX, convertForLabel(mEigvals[0]));
  state->setValue(InputSizeY, convertForLabel(mEigvals[1]));
  state->setValue(InputSizeZ, convertForLabel(mEigvals[2]));
}

void EditMeshBoundingBox::saveToParameters()
{
  auto state = get_state();
  state->setValue(TransformMatrix, mTrans.get_string());
  state->setValue(FieldTransformMatrix, mFieldTrans.get_string());
  state->setValue(DataSaved, true);
}

const AlgorithmParameterName EditMeshBoundingBox::TransformMatrix("TransformMatrix");
const AlgorithmParameterName EditMeshBoundingBox::FieldTransformMatrix("FieldTransformMatrix");
const AlgorithmParameterName EditMeshBoundingBox::ResetToInput("ResetToInput");
const AlgorithmParameterName EditMeshBoundingBox::ResetCenter("ResetCenter");
const AlgorithmParameterName EditMeshBoundingBox::ResetSize("ResetSize");
const AlgorithmParameterName EditMeshBoundingBox::DataSaved("DataSaved");

const AlgorithmParameterName EditMeshBoundingBox::InputCenterX("InputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterY("InputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::InputCenterZ("InputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeX("InputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeY("InputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::InputSizeZ("InputSizeZ");
 //Output Field Atributes
const AlgorithmParameterName EditMeshBoundingBox::SetOutputCenter("SetOutputCenter");
const AlgorithmParameterName EditMeshBoundingBox::SetOutputSize("SetOutputSize");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterX("OutputCenterX");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterY("OutputCenterY");
const AlgorithmParameterName EditMeshBoundingBox::OutputCenterZ("OutputCenterZ");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeX("OutputSizeX");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeY("OutputSizeY");
const AlgorithmParameterName EditMeshBoundingBox::OutputSizeZ("OutputSizeZ");
//Widget Scale/Mode
const AlgorithmParameterName EditMeshBoundingBox::Scale("Scale");
const AlgorithmParameterName EditMeshBoundingBox::OldScale("OldScale");
const AlgorithmParameterName EditMeshBoundingBox::ScaleChanged("ScaleChanged");
const AlgorithmParameterName EditMeshBoundingBox::NoTranslation("NoTranslation");
const AlgorithmParameterName EditMeshBoundingBox::XYZTranslation("XYZTranslation");
const AlgorithmParameterName EditMeshBoundingBox::RDITranslation("RDITranslation");
const AlgorithmParameterName EditMeshBoundingBox::RestrictX("RestrictX");
const AlgorithmParameterName EditMeshBoundingBox::RestrictY("RestrictY");
const AlgorithmParameterName EditMeshBoundingBox::RestrictZ("RestrictZ");
const AlgorithmParameterName EditMeshBoundingBox::RestrictR("RestrictR");
const AlgorithmParameterName EditMeshBoundingBox::RestrictD("RestrictD");
const AlgorithmParameterName EditMeshBoundingBox::RestrictI("RestrictI");

const AlgorithmParameterName EditMeshBoundingBox::BoxMode("BoxMode");
const AlgorithmParameterName EditMeshBoundingBox::BoxRealScale("BoxRealScale");
