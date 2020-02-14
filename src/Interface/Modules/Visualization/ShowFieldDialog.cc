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


#include <Interface/Modules/Visualization/ShowFieldDialog.h>
#include <Modules/Visualization/ShowField.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Visualization;

ShowFieldDialog::ShowFieldDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  defaultMeshColor_(Qt::gray)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  WidgetStyleMixin::tabStyle(this->displayOptionsTabs_);
  addLineEditManager(fieldNameLineEdit_, Parameters::FieldName);
  addCheckBoxManager(showNodesCheckBox_, Parameters::ShowNodes);
  addCheckBoxManager(showEdgesCheckBox_, Parameters::ShowEdges);
  addCheckBoxManager(showFacesCheckBox_, Parameters::ShowFaces);
  addCheckBoxManager(enableTransparencyNodesCheckBox_, Parameters::NodeTransparency);
  addCheckBoxManager(enableTransparencyEdgesCheckBox_, Parameters::EdgeTransparency);
  addCheckBoxManager(enableTransparencyFacesCheckBox_, Parameters::FaceTransparency);
  addCheckBoxManager(invertNormalsCheckBox, Parameters::FaceInvertNormals);
  addCheckBoxManager(showTextCheckBox_, Parameters::ShowText);
  addCheckBoxManager(showDataValuesCheckBox_, Parameters::ShowDataValues);
  addCheckBoxManager(showNodeIndicesCheckBox_, Parameters::ShowNodeIndices);
  addCheckBoxManager(showEdgeIndicesCheckBox_, Parameters::ShowEdgeIndices);
  addCheckBoxManager(showFaceIndicesCheckBox_, Parameters::ShowFaceIndices);
  addCheckBoxManager(showCellIndicesCheckBox_, Parameters::ShowCellIndices);
  addCheckBoxManager(cullTextCheckBox_, Parameters::CullBackfacingText);
  addCheckBoxManager(textAlwaysVisibleCheckBox_, Parameters::TextAlwaysVisible);
  addCheckBoxManager(renderIndicesLocationsCheckBox_, Parameters::RenderAsLocation);
  addCheckBoxManager(useFaceNormalsCheckBox_, Parameters::UseFaceNormals);
  addDoubleSpinBoxManager(transparencyDoubleSpinBox_, Parameters::FaceTransparencyValue);
  addDoubleSpinBoxManager(nodeTransparencyDoubleSpinBox_, Parameters::NodeTransparencyValue);
  addDoubleSpinBoxManager(edgeTransparencyDoubleSpinBox_, Parameters::EdgeTransparencyValue);
  addDoubleSpinBoxManager(scaleSphereDoubleSpinBox_, Parameters::SphereScaleValue);
  addDoubleSpinBoxManager(cylinder_rad_spin, Parameters::CylinderRadius);
  addSpinBoxManager(cylinder_res_spin, Parameters::CylinderResolution);
  addSpinBoxManager(sphereResolutionSpinBox, Parameters::SphereResolution);
  addSpinBoxManager(textSizeSpinBox_, Parameters::TextSize);
  addSpinBoxManager(textPrecisionSpinBox_, Parameters::TextPrecision);
  addRadioButtonGroupManager({ edgesAsLinesButton_, edgesAsCylindersButton_ }, Parameters::EdgesAsCylinders);
  addRadioButtonGroupManager({ nodesAsPointsButton_, nodesAsSpheresButton_ }, Parameters::NodeAsSpheres);
  addRadioButtonGroupManager({ defaultNodeColoringButton_, colormapLookupNodeColoringButton_/*, conversionRGBNodeColoringButton_*/ }, Parameters::NodesColoring);
  addRadioButtonGroupManager({ defaultEdgeColoringButton_, colormapLookupEdgeColoringButton_/*, conversionRGBEdgeColoringButton_*/ }, Parameters::EdgesColoring);
  addRadioButtonGroupManager({ defaultFaceColoringButton_, colormapLookupFaceColoringButton_/*, conversionRGBFaceColoringButton_*/ }, Parameters::FacesColoring);
  addRadioButtonGroupManager({ textColoringRadioButton_, colormapLookupTextRadioButton_, conversionRGBTextRadioButton_ }, Parameters::TextColoring);

  connect(defaultMeshColorButton_, SIGNAL(clicked()), this, SLOT(assignDefaultMeshColor()));
  connect(textColorPushButton_, SIGNAL(clicked()), this, SLOT(assignDefaultTextColor()));

  connectButtonsToExecuteSignal({ showNodesCheckBox_, showEdgesCheckBox_, showFacesCheckBox_, enableTransparencyNodesCheckBox_,
    enableTransparencyEdgesCheckBox_, enableTransparencyFacesCheckBox_, invertNormalsCheckBox, edgesAsLinesButton_,
    edgesAsCylindersButton_, nodesAsPointsButton_, nodesAsSpheresButton_ ,
    defaultNodeColoringButton_, colormapLookupNodeColoringButton_, //conversionRGBNodeColoringButton_,
    defaultEdgeColoringButton_, colormapLookupEdgeColoringButton_, //conversionRGBEdgeColoringButton_,
    defaultFaceColoringButton_, colormapLookupFaceColoringButton_, /*conversionRGBFaceColoringButton_*/
    defaultMeshColorButton_, textColorPushButton_ });

  connectButtonToExecuteSignal(useFaceNormalsCheckBox_);

  createExecuteInteractivelyToggleAction();

  /////Set unused widgets to be not visible
  //Nodes Tab
  conversionRGBNodeColoringButton_->setVisible(false);

  //Edges Tab
  conversionRGBEdgeColoringButton_->setVisible(false);

  //Faces Tab
  conversionRGBFaceColoringButton_->setVisible(false);
  textureCheckBox_->setVisible(false);

  //Text Tab
  displayOptionsTabs_->removeTab(3);
}

void ShowFieldDialog::push()
{
  if (!pulling_)
  {
    pushColor();
  }
}

void ShowFieldDialog::createStartupNote()
{
  auto showFieldId = windowTitle().split(':')[1];
  setStartupNote("ID: " + showFieldId);
}

void ShowFieldDialog::pullSpecial()
{
  defaultMeshColor_ = colorFromState(Parameters::DefaultMeshColor);
  defaultTextColor_ = colorFromState(Parameters::DefaultTextColor);

  QString styleSheet = "QLabel{ background: rgb(" + QString::number(defaultTextColor_.red()) + "," +
    QString::number(defaultTextColor_.green()) + "," + QString::number(defaultTextColor_.blue()) + "); }";
  textColorLabel_->setStyleSheet(styleSheet);

  nodesTab_->setEnabled(state_->getValue(Parameters::NodesAvailable).toBool());
  displayOptionsTabs_->setTabText(0, nodesTab_->isEnabled() ? "Nodes" : "NO NODES");
  edgesTab_->setEnabled(state_->getValue(Parameters::EdgesAvailable).toBool());
  displayOptionsTabs_->setTabText(1, edgesTab_->isEnabled() ? "Edges" : "NO EDGES");
  facesTab_->setEnabled(state_->getValue(Parameters::FacesAvailable).toBool());
  displayOptionsTabs_->setTabText(2, facesTab_->isEnabled() ? "Faces" : "NO FACES");
}

void ShowFieldDialog::assignDefaultMeshColor()
{
  auto newColor = QColorDialog::getColor(defaultMeshColor_, this, "Choose default mesh color");
  if (newColor.isValid())
  {
    defaultMeshColor_ = newColor;
    //TODO: set color of button to this color
    //defaultMeshColorButton_->set
    pushColor();
  }
}

void ShowFieldDialog::assignDefaultTextColor()
{
  auto newColor = QColorDialog::getColor(defaultTextColor_, this, "Choose default text color");
  if (newColor.isValid())
  {
    defaultTextColor_ = newColor;
    //TODO: set color of button to this color
    //defaultMeshColorButton_->set
    pushColor();
  }
}

void ShowFieldDialog::pushColor()
{
  colorToState(Parameters::DefaultTextColor, defaultTextColor_);
  colorToState(Parameters::DefaultMeshColor, defaultMeshColor_);

  QString styleSheet = "QLabel{ background: rgb(" + QString::number(defaultTextColor_.red()) + "," +
    QString::number(defaultTextColor_.green()) + "," + QString::number(defaultTextColor_.blue()) + "); }";
  textColorLabel_->setStyleSheet(styleSheet);
}
