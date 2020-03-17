#include <Interface/Modules/Visualization/ShowMeshBoundingBoxDialog.h>
#include <Core/Algorithms/Visualization/ShowMeshBoundingBoxAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Visualization;

ShowMeshBoundingBoxDialog::ShowMeshBoundingBoxDialog(const std::string& name,
                                                     ModuleStateHandle state,
                                                     QWidget* parent/* = 0*/)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

  addSpinBoxManager(xSpinBox_, Parameters::XSize);
  addSpinBoxManager(ySpinBox_, Parameters::YSize);
  addSpinBoxManager(zSpinBox_, Parameters::ZSize);
}
