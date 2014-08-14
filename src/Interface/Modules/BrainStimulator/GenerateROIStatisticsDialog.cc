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
#include <boost/foreach.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms;

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
  //std::cout <<  "ok! ROI dialog" << std::endl;
  auto all_elc_values = optional_any_cast_or_default<Variable>(state_->getTransientValue(Parameters::StatisticsTableValues.name())).getList();
  //std::cout << "# rows: " << all_elc_values.size() << std::endl;
  StatisticsOutput_tableWidget->setRowCount(all_elc_values.size());

  for (int i=0; i<all_elc_values.size(); i++)
  {
   auto col = (all_elc_values[i]).getList();

   //std::cout << "\t# cols: " << col.size() << std::endl;
   int j = 0;
   BOOST_FOREACH(const AlgorithmParameter& ap, col)
   {
    auto tmpstr = ap.getString();
    //std::cout << "\t\t cell value: " << tmpstr  << std::endl;
  
    auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    StatisticsOutput_tableWidget->setItem(i, j, item);
    ++j;
   }
  }

}

