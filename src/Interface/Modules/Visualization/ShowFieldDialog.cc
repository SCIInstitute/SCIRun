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
  addLineEditManager(fieldNameLineEdit_, ShowField::FieldName);
  addCheckBoxManager(showNodesCheckBox_, ShowField::ShowNodes);
  addCheckBoxManager(showEdgesCheckBox_, ShowField::ShowEdges);
  addCheckBoxManager(showFacesCheckBox_, ShowField::ShowFaces);
  addCheckBoxManager(enableTransparencyNodesCheckBox_, ShowField::NodeTransparency);
  addCheckBoxManager(enableTransparencyEdgesCheckBox_, ShowField::EdgeTransparency);
  addCheckBoxManager(enableTransparencyFacesCheckBox_, ShowField::FaceTransparency);
  addCheckBoxManager(invertNormalsCheckBox, ShowField::FaceInvertNormals);
  addCheckBoxManager(showTextCheckBox_, ShowField::ShowText);
  addCheckBoxManager(showDataValuesCheckBox_, ShowField::ShowDataValues);
  addCheckBoxManager(showNodeIndicesCheckBox_, ShowField::ShowNodeIndices);
  addCheckBoxManager(showEdgeIndicesCheckBox_, ShowField::ShowEdgeIndices);
  addCheckBoxManager(showFaceIndicesCheckBox_, ShowField::ShowFaceIndices);
  addCheckBoxManager(showCellIndicesCheckBox_, ShowField::ShowCellIndices);
  addCheckBoxManager(cullTextCheckBox_, ShowField::CullBackfacingText);
  addCheckBoxManager(textAlwaysVisibleCheckBox_, ShowField::TextAlwaysVisible);
  addCheckBoxManager(renderIndicesLocationsCheckBox_, ShowField::RenderAsLocation);
  addCheckBoxManager(useFaceNormalsCheckBox_, ShowField::UseFaceNormals);
  addDoubleSpinBoxManager(transparencyDoubleSpinBox_, ShowField::FaceTransparencyValue);
  addDoubleSpinBoxManager(nodeTransparencyDoubleSpinBox_, ShowField::NodeTransparencyValue);
  addDoubleSpinBoxManager(edgeTransparencyDoubleSpinBox_, ShowField::EdgeTransparencyValue);
  addDoubleSpinBoxManager(scaleSphereDoubleSpinBox_, ShowField::SphereScaleValue);
  addDoubleSpinBoxManager(cylinder_rad_spin, Parameters::CylinderRadius);
  addSpinBoxManager(cylinder_res_spin, ShowField::CylinderResolution);
  addSpinBoxManager(sphereResolutionSpinBox, ShowField::SphereResolution);
  addSpinBoxManager(textSizeSpinBox_, ShowField::TextSize);
  addSpinBoxManager(textPrecisionSpinBox_, ShowField::TextPrecision);
  addRadioButtonGroupManager({ edgesAsLinesButton_, edgesAsCylindersButton_ }, ShowField::EdgesAsCylinders);
  addRadioButtonGroupManager({ nodesAsPointsButton_, nodesAsSpheresButton_ }, ShowField::NodeAsSpheres);
  addRadioButtonGroupManager({ defaultNodeColoringButton_, colormapLookupNodeColoringButton_/*, conversionRGBNodeColoringButton_*/ }, ShowField::NodesColoring);
  addRadioButtonGroupManager({ defaultEdgeColoringButton_, colormapLookupEdgeColoringButton_/*, conversionRGBEdgeColoringButton_*/ }, ShowField::EdgesColoring);
  addRadioButtonGroupManager({ defaultFaceColoringButton_, colormapLookupFaceColoringButton_/*, conversionRGBFaceColoringButton_*/ }, ShowField::FacesColoring);
  addRadioButtonGroupManager({ textColoringRadioButton_, colormapLookupTextRadioButton_, conversionRGBTextRadioButton_ }, ShowField::TextColoring);

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
  defaultMeshColor_ = colorFromState(ShowField::DefaultMeshColor);
  defaultTextColor_ = colorFromState(ShowField::DefaultTextColor);

  QString styleSheet = "QLabel{ background: rgb(" + QString::number(defaultTextColor_.red()) + "," +
    QString::number(defaultTextColor_.green()) + "," + QString::number(defaultTextColor_.blue()) + "); }";
  textColorLabel_->setStyleSheet(styleSheet);

  nodesTab_->setEnabled(state_->getValue(ShowField::NodesAvailable).toBool());
  displayOptionsTabs_->setTabText(0, nodesTab_->isEnabled() ? "Nodes" : "NO NODES");
  edgesTab_->setEnabled(state_->getValue(ShowField::EdgesAvailable).toBool());
  displayOptionsTabs_->setTabText(1, edgesTab_->isEnabled() ? "Edges" : "NO EDGES");
  facesTab_->setEnabled(state_->getValue(ShowField::FacesAvailable).toBool());
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
  colorToState(ShowField::DefaultTextColor, defaultTextColor_);
  colorToState(ShowField::DefaultMeshColor, defaultMeshColor_);

  QString styleSheet = "QLabel{ background: rgb(" + QString::number(defaultTextColor_.red()) + "," +
    QString::number(defaultTextColor_.green()) + "," + QString::number(defaultTextColor_.blue()) + "); }";
  textColorLabel_->setStyleSheet(styleSheet);
}
