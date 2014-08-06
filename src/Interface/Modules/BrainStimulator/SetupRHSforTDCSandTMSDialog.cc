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

#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Interface/Modules/BrainStimulator/SetupRHSforTDCSandTMSDialog.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;

SetupRHSforTDCSandTMSDialog::SetupRHSforTDCSandTMSDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  const int total_electrodes = 128;

  electrode_tableWidget->setRowCount(total_electrodes);
  electrode_tableWidget->setColumnCount(2);

  QStringList tableHeader;
  tableHeader<<"Electrode"<<"Current intensity [mA]";
  electrode_tableWidget->setHorizontalHeaderLabels(tableHeader);

  for (int i=0; i<total_electrodes; i++)
  {
    // setting the name of the electrode
    electrode_tableWidget->setItem(i, 0, new QTableWidgetItem("elc"+QString::number(i)));

    // seting the inital values of the electrodes
      electrode_tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(SetupRHSforTDCSandTMSAlgorithm::initialElectrodeValue(i))));
  }
  
  // connecting all table cell positions (int = row, int = col)
  connect(electrode_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
}

void SetupRHSforTDCSandTMSDialog::push()
{
  if (!pulling_)
  {
    // updating electrode values (from table) into vector then attaching them to state map
    int rows = electrode_tableWidget->rowCount();
    std::vector<AlgorithmParameter> elc_vals_in_table;
    for (int i=0; i<rows; i++)
    {
      AlgorithmParameter elc_i(SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(i), electrode_tableWidget->item(i,1)->text().toDouble());
      elc_vals_in_table.push_back(elc_i);
    }
    state_->setValue(Parameters::ElectrodeTableValues, elc_vals_in_table);
  }
}

void SetupRHSforTDCSandTMSDialog::pull()
{
  Pulling p(this); // prevents from re-entering if statement when next line executes

  // obtaining initial values, pulling hasn't been set
  std::vector<AlgorithmParameter> elc_vals_in_table;
  int rows = electrode_tableWidget->rowCount();
  for (int i=0; i<rows; i++)
  {
    AlgorithmParameter elc_i(SetupRHSforTDCSandTMSAlgorithm::ElecrodeParameterName(i), electrode_tableWidget->item(i,1)->text().toDouble());
    elc_vals_in_table.push_back(elc_i);
  }
  state_->setValue(Parameters::ElectrodeTableValues, elc_vals_in_table);
 
}

