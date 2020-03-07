#include <Interface/Modules/Visualization/ShowMeshBoundingBoxDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

ShowMeshBoundingBoxDialog::ShowMeshBoundingBoxDialog(const std::string& name,
                                                     ModuleStateHandle state,
                                                     QWidget* parent/* = 0*/)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

  addRadioButtonGroupManager({ ascendButton_, descendButton_ }, Variables::Method);
}
