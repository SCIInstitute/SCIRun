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
  WidgetStyleMixin::tabStyle(this->tabWidget);
  WidgetStyleMixin::tableHeaderStyle(this->electrode_tableWidget);
  addCheckBoxManager(GetContactSurfCheckBox_, Parameters::GetContactSurface);
  addSpinBoxManager(refnode_, Parameters::refnode);
  addDoubleSpinBoxManager(pointdistancebound_, Parameters::pointdistancebound);
  addDoubleSpinBoxManager(normal_dot_product_bound_, Parameters::normal_dot_product_bound);

  int max_nr_elc=(state_->getValue(Parameters::number_of_electrodes)).toInt();

  electrode_tableWidget->setColumnCount(4);
  electrode_tableWidget->setRowCount(max_nr_elc);
  QStringList tableHeader;
  tableHeader << "Electrode" << "Tot. inj. current [mA]" << "Real impedance [Ohm*m^2]" << "Surface area [m^2]";
  electrode_tableWidget->setHorizontalHeaderLabels(tableHeader);


  for (int i=0; i<max_nr_elc; i++)
  {
    // setting the name of the electrode
    electrode_tableWidget->setItem(i, 0, new QTableWidgetItem("elc"+QString::number(i)));

    // seting the inital values of the electrodes
    if (i == 0)
    {
      electrode_tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(1.0)));  /// set default current intensity: 1 mA
      electrode_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(5000.0))); /// set default Contact Impedance: 5000 Ohm * m^2
      electrode_tableWidget->setItem(i, 3, new QTableWidgetItem(QString(""))); /// electrode surface area is going to be determined if valid input data are provided
    }
    else if (i == 1)
     {
      electrode_tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(-1.0)));
      electrode_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(5000.0)));
      electrode_tableWidget->setItem(i, 3, new QTableWidgetItem(QString("")));
     }
    else if (i > 1)
      {
       electrode_tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(0.0)));
       electrode_tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(5000.0)));
       electrode_tableWidget->setItem(i, 3, new QTableWidgetItem(QString("")));
      }
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
    std::vector<AlgorithmParameter> imp_elc_vals_in_table;

    for (int i=0; i<rows; i++)
    {
      AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), electrode_tableWidget->item(i,1)->text().toDouble());
      elc_vals_in_table.push_back(elc_i);
      AlgorithmParameter imp_elc_i(Name("imp_elc" + boost::lexical_cast<std::string>(i)), electrode_tableWidget->item(i,2)->text().toDouble());
      imp_elc_vals_in_table.push_back(imp_elc_i);
    }
    state_->setValue(Parameters::ElectrodeTableValues, elc_vals_in_table);
    state_->setValue(Parameters::ImpedanceTableValues, imp_elc_vals_in_table);
  }
}

void SetupRHSforTDCSandTMSDialog::pullSpecial()
{
  int nr_elc=(state_->getValue(Parameters::number_of_electrodes)).toInt();
  int rows=-1;

  if (nr_elc!=-1)
  {
    electrode_tableWidget->setRowCount(static_cast<int>(nr_elc));
    rows = nr_elc;
  } else
  {
    rows = electrode_tableWidget->rowCount();
  }

  if (nr_elc>=SetupTDCSAlgorithm::max_number_of_electrodes )
  {
    nr_elc=SetupTDCSAlgorithm::max_number_of_electrodes;
    rows = nr_elc;
  }

  auto surface_areas = (state_->getValue(Parameters::SurfaceAreaValues)).toVector();
  // obtaining initial values, pulling hasn't been set
  std::vector<AlgorithmParameter> elc_vals_in_table; //electrical electrode charges
  std::vector<AlgorithmParameter> imp_elc_vals_in_table; //electrode impedances
  for (int i=0; i<rows; i++)
  {
    AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), electrode_tableWidget->item(i,1)->text().toDouble());
    elc_vals_in_table.push_back(elc_i);
    AlgorithmParameter imp_elc_i(Name("imp_elc" + boost::lexical_cast<std::string>(i)), electrode_tableWidget->item(i,2)->text().toDouble());
    imp_elc_vals_in_table.push_back(imp_elc_i);
    if (surface_areas.size() == rows)
    {
     double tmp = (surface_areas[i]).toDouble();

     QString text = QString::number(tmp);
     QDoubleValidator validator;
     int index;
     if (validator.validate(text, index) == QValidator::Acceptable)
     {
      electrode_tableWidget->blockSignals(true);
      electrode_tableWidget->item(i, 3)->setText(text);
      electrode_tableWidget->blockSignals(false);
     }
    }
  }
  state_->setValue(Parameters::ElectrodeTableValues, elc_vals_in_table);
  state_->setValue(Parameters::ImpedanceTableValues, imp_elc_vals_in_table);
}
