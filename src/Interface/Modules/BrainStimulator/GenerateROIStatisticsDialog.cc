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

#include <Interface/Modules/BrainStimulator/GenerateROIStatisticsDialog.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::BrainStimulator;


GenerateROIStatisticsDialog::GenerateROIStatisticsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  QStringList tableHeader1;
  tableHeader1<<" ROI "<<" Avr. " << " Std. " << " Min. " << " Max. ";
  StatisticsOutput_tableWidget->setHorizontalHeaderLabels(tableHeader1);
  StatisticsOutput_tableWidget->setItem(0, 0, 0);
  StatisticsOutput_tableWidget->setItem(0, 1, 0);
  StatisticsOutput_tableWidget->setItem(0, 2, 0);
  StatisticsOutput_tableWidget->setItem(0, 3, 0);
  StatisticsOutput_tableWidget->setItem(0, 4, 0);
  
  QStringList tableHeader2;
  tableHeader2<<" X "<<" Y " << " Z " << " Atlas Material # " << " Radius ";
  SpecifyROI_tabWidget->setHorizontalHeaderLabels(tableHeader2);
  SpecifyROI_tabWidget->setItem(0, 0, 0);
  SpecifyROI_tabWidget->setItem(0, 1, 0);
  SpecifyROI_tabWidget->setItem(0, 2, 0);
  SpecifyROI_tabWidget->setItem(0, 3, 0);
  
  connect(StatisticsOutput_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
  connect(SpecifyROI_tabWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));  
}

void GenerateROIStatisticsDialog::push()
{
  if (!pulling_)
  {
   //state_->setValue(Parameters::ElectrodeTableValues, elc_vals_in_table); 
  }
}


void GenerateROIStatisticsDialog::pull()
{
  Pulling p(this);
  std::cout <<  "ok!" << std::endl;
  auto all_elc_values = state_->getValue(Parameters::StatisticsTableValues).getList();
  std::cout <<   all_elc_values.size() << std::endl;
  for (int i=0; i<all_elc_values.size(); i++)
  {
   auto tmp = (all_elc_values[i]).getList();
   auto tmpstr = tmp[0].getString();
   std::cout << tmpstr  << std::endl;
   StatisticsOutput_tableWidget->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(tmpstr)));
  }

}

