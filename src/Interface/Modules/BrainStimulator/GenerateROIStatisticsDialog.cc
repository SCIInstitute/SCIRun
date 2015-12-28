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

#include <Interface/Modules/BrainStimulator/GenerateROIStatisticsDialog.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <boost/assign/std/vector.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace boost::assign;

GenerateROIStatisticsDialog::GenerateROIStatisticsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  WidgetStyleMixin::tabStyle(this->tabWidget);
  WidgetStyleMixin::tableHeaderStyle(this->SpecifyROI_tabWidget);
  WidgetStyleMixin::tableHeaderStyle(this->StatisticsOutput_tableWidget);

  QStringList tableHeader1;  /// set default GUI parameter for upper table
  tableHeader1<<" ROI "<<" Avr. " << " Std. " << " Min. " << " Max. " << " # ";
  StatisticsOutput_tableWidget->setHorizontalHeaderLabels(tableHeader1);
  StatisticsOutput_tableWidget->setItem(0, 0, 0);
  StatisticsOutput_tableWidget->setItem(0, 1, 0);
  StatisticsOutput_tableWidget->setItem(0, 2, 0);
  StatisticsOutput_tableWidget->setItem(0, 3, 0);
  StatisticsOutput_tableWidget->setItem(0, 4, 0);
  StatisticsOutput_tableWidget->setItem(0, 5, 0);

  QStringList tableHeader2; /// set default GUI parameter for lower table
  tableHeader2<<" X "<<" Y " << " Z " << " Atlas Material # " << " Radius ";
  SpecifyROI_tabWidget->setHorizontalHeaderLabels(tableHeader2);
  SpecifyROI_tabWidget->setItem(0, 0, new QTableWidgetItem("0"));
  SpecifyROI_tabWidget->setItem(0, 1, new QTableWidgetItem("0"));
  SpecifyROI_tabWidget->setItem(0, 2, new QTableWidgetItem("0"));
  SpecifyROI_tabWidget->setItem(0, 3, new QTableWidgetItem(" "));
  SpecifyROI_tabWidget->setItem(0, 4, new QTableWidgetItem(" 0 "));

  connect(StatisticsOutput_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
  connect(SpecifyROI_tabWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
}

void GenerateROIStatisticsDialog::push()
{

  if (!pulling_)
  {
    QPalette* palette = new QPalette();
    palette->setColor(QPalette::Text,Qt::red);
    StatisticsTableGroupBox->setPalette(*palette);

    /// get user specified ROI data, put it in a DenseMatrix and ship it to state
    auto X = SpecifyROI_tabWidget->item(0,0)->text().toDouble();
    auto Y = SpecifyROI_tabWidget->item(0,1)->text().toDouble();
    auto Z = SpecifyROI_tabWidget->item(0,2)->text().toDouble();
    auto material = SpecifyROI_tabWidget->item(0,3)->text().toDouble();
    auto radius = SpecifyROI_tabWidget->item(0,4)->text().toDouble();
    DenseMatrixHandle specROI(new DenseMatrix(5,1));
    (*specROI) << X, Y, Z, material, radius;

    state_->setTransientValue(GenerateROIStatisticsAlgorithm::SpecifyROI, specROI);
  }
}


void GenerateROIStatisticsDialog::pullSpecial()
{
  auto tableHandle = transient_value_cast<VariableHandle>(state_->getTransientValue(Parameters::StatisticsTableValues));

  if (tableHandle)
  {
    auto all_elc_values = tableHandle->toVector();
    StatisticsOutput_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));

    //TODO: use this example to improve Variable::List syntactical sugar

    /// get the result data from the algorithm and put it in the GUI table
    for (int i=0; i<all_elc_values.size(); i++)
    {
      auto col = (all_elc_values[i]).toVector();

      int j = 0;
      for (const AlgorithmParameter& ap : col)
      {
        auto tmpstr = ap.toString();

        auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        StatisticsOutput_tableWidget->setItem(i, j, item);
        ++j;
      }
    }
  }
  else
  {
    // ?? log a message ? unsure.
  }

  /// get the strings PhysicalUnit / CoordinateSpaceLabel from state (directly from module level) and show it above GUI tables
  std::string PhysicalUnitString = transient_value_cast<std::string>(state_->getTransientValue(Parameters::PhysicalUnitStr));  /// change GUI Labels due to physical unit and used coordinate space
  if (!PhysicalUnitString.empty())
  {
    StatisticsTableGroupBox->setTitle("Statistics for ROIs: " + QString::fromStdString(PhysicalUnitString));
  }

  std::string CoordinateSpaceLabelStr = transient_value_cast<std::string>(state_->getTransientValue(Parameters::CoordinateSpaceLabelStr));  /// change GUI Labels due to physical unit and used coordinate space
  if (!CoordinateSpaceLabelStr.empty())
  {
    ROITableGroupBox->setTitle("Specify ROI: " + QString::fromStdString(CoordinateSpaceLabelStr));
  }
}
