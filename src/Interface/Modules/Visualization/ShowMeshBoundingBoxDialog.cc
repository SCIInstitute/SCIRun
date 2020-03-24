#include <Interface/Modules/Visualization/ShowMeshBoundingBoxDialog.h>
#include <Modules/Visualization/ShowMeshBoundingBox.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

using SMBB = SCIRun::Modules::Visualization::ShowMeshBoundingBox;

ShowMeshBoundingBoxDialog::ShowMeshBoundingBoxDialog(const std::string& name,
                                                     ModuleStateHandle state,
                                                     QWidget* parent/* = 0*/)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

  addSpinBoxManager(xSpinBox_, SMBB::XSize);
  addSpinBoxManager(ySpinBox_, SMBB::YSize);
  addSpinBoxManager(zSpinBox_, SMBB::ZSize);
}
