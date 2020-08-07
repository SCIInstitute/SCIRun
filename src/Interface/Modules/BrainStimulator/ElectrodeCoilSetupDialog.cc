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


#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Interface/Modules/BrainStimulator/ElectrodeCoilSetupDialog.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;

ElectrodeCoilSetupDialog::ElectrodeCoilSetupDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  comboBoxesSetup_(false),
  pushTableFlag_(true)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  WidgetStyleMixin::tableHeaderStyle(this->electrode_coil_tableWidget);
  WidgetStyleMixin::tabStyle(this->tabWidget);
  electrode_coil_tableWidget->setRowCount(1);
  electrode_coil_tableWidget->setColumnCount(ElectrodeCoilSetupAlgorithm::number_of_columns);
  electrode_coil_tableWidget->setItem(0, 0, new QTableWidgetItem(" "));
  electrode_coil_tableWidget->setItem(0, 1, new QTableWidgetItem(" "));
  QStringList tableHeader;
  tableHeader<<"Input #"<<"Type"<<"X"<<"Y"<<"Z"<<"Angle"<<"NX"<<"NY"<<"NZ"<<"thickness";
  electrode_coil_tableWidget->setHorizontalHeaderLabels(tableHeader);
  addCheckBoxManager(ProtoTypeInputCheckbox_, Parameters::ProtoTypeInputCheckbox);
  addCheckBoxManager(AllInputsTDCS_, Parameters::AllInputsTDCS);
  addCheckBoxManager(electrodethicknessCheckBox_, Parameters::ElectrodethicknessCheckBox);
  addCheckBoxManager(invertNormalsCheckBox_, Parameters::InvertNormalsCheckBox);
  addCheckBoxManager(OrientTMSCoilRadialToScalpCheckBox_, Parameters::OrientTMSCoilRadialToScalpCheckBox);
  addCheckBoxManager(PutElectrodesOnScalpCheckBox_, Parameters::PutElectrodesOnScalpCheckBox);
  addCheckBoxManager(InterpolateElectrodeShapeCheckbox_, Parameters::InterpolateElectrodeShapeCheckbox);
  addDoubleSpinBoxManager(electrodethicknessSpinBox_, Parameters::ElectrodethicknessSpinBox);
  connect(electrode_coil_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(validateCell(int, int)));
  connect(AllInputsTDCS_, SIGNAL(stateChanged(int)), this, SLOT(updateStimTypeColumn()));
  connect(invertNormalsCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(updateInvertNormals()));
  connect(ProtoTypeInputCheckbox_, SIGNAL(stateChanged(int)), this, SLOT(togglePrototypeColumnReadOnly(int)));
  connect(ProtoTypeInputComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePrototypeColumnValues(int)));

  connect(PutElectrodesOnScalpCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(toggleThicknessColumnReadOnly(int)));
  connect(electrodethicknessCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(toggleThicknessColumnReadOnly(int)));

  connect(electrodethicknessSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateThicknessColumnValues(double)));
}

void ElectrodeCoilSetupDialog::updateInvertNormals()
{
 auto all_elc_values = (state_->getValue(Parameters::TableValues)).toVector();

  for (int j=0; j<all_elc_values.size(); j++)
  {
   float NX=std::numeric_limits<float>::quiet_NaN(), NY=std::numeric_limits<float>::quiet_NaN(), NZ=std::numeric_limits<float>::quiet_NaN();
   try
   {
    NX=boost::lexical_cast<float>((electrode_coil_tableWidget->item(j,6)->text()).toStdString());
   }
   catch(boost::bad_lexical_cast &)
   {
   }

   try
   {
    NY= boost::lexical_cast<float>((electrode_coil_tableWidget->item(j,7)->text()).toStdString());
   }
   catch(boost::bad_lexical_cast &)
   {
   }

   try
   {
    NZ= boost::lexical_cast<float>((electrode_coil_tableWidget->item(j,8)->text()).toStdString());
   }
   catch(boost::bad_lexical_cast &)
   {
   }

   if(!(IsNan(NX) || IsNan(NY) || IsNan(NZ)))
   {

     if (NX<0) NX=fabs(NX);
       else
         if (NX>0) NX=-1 * fabs(NX);

     if (NY<0) NY=fabs(NY);
       else
         if (NY>0) NY=-1 * fabs(NY);

     if (NZ<0) NZ=fabs(NZ);
       else
         if (NZ>0) NZ=-1 * fabs(NZ);

     auto item_NX = electrode_coil_tableWidget->item(j, 6);
     auto item_NY = electrode_coil_tableWidget->item(j, 7);
     auto item_NZ = electrode_coil_tableWidget->item(j, 8);
     electrode_coil_tableWidget->blockSignals(true);
     if (j==0)
        electrode_coil_tableWidget->scrollToItem(electrode_coil_tableWidget->item(0, 6));

     item_NX->setText(QString::fromStdString(str(boost::format("%.3f") % NX)));
     item_NY->setText(QString::fromStdString(str(boost::format("%.3f") % NY)));
     item_NZ->setText(QString::fromStdString(str(boost::format("%.3f") % NZ)));
     electrode_coil_tableWidget->blockSignals(false);

     push();
   }
 }
}

void ElectrodeCoilSetupDialog::validateCell(int row, int col)
{
  auto cell = electrode_coil_tableWidget->item(row, col);
  QString text = cell->text();
  QDoubleValidator validator;
  int index;
  if (validator.validate(text, index) == QValidator::Acceptable)
  {

  }
  else
  {
    static const QString unknown("???");
    electrode_coil_tableWidget->blockSignals(true);
    cell->setText(unknown);
    electrode_coil_tableWidget->blockSignals(false);
  }
  push();
}

std::vector<Variable> ElectrodeCoilSetupDialog::validate_numerical_input(int i)
{
  std::vector<Variable> values;
  int inputport_ind=((QComboBox *)inputPortsVector_[i])->currentIndex();
  int stimtype_ind=((QComboBox *)stimTypeVector_[i])->currentIndex();
  static const std::string unknown("???");

  values.push_back(Variable(ElectrodeCoilSetupAlgorithm::columnNames[0], boost::lexical_cast<std::string>(inputport_ind)));
  values.push_back(Variable(Name("Type"), boost::lexical_cast<std::string>(stimtype_ind)));

  for (int j=2; j<ElectrodeCoilSetupAlgorithm::number_of_columns; j++)
  {
   try
   {
     boost::lexical_cast<double>((electrode_coil_tableWidget->item(i,j)->text()).toStdString());
    values.push_back(Variable(ElectrodeCoilSetupAlgorithm::columnNames[j], electrode_coil_tableWidget->item(i,j)->text().toStdString()));
   }
   catch(boost::bad_lexical_cast &)
   {
    values.push_back(Variable(ElectrodeCoilSetupAlgorithm::columnNames[j], unknown));
   }
  }

  return values;
}

void ElectrodeCoilSetupDialog::push()
{
  if (!pulling_)
  {
    pushTable();

    auto label = ProtoTypeInputComboBox_->currentText().toStdString();
    if (label != state_->getValue(Parameters::ProtoTypeInputComboBox).toString())
    {
      state_->setValue(Parameters::ProtoTypeInputComboBox, label);
    }
  }
}

void ElectrodeCoilSetupDialog::pushTable()
{
  std::vector<AlgorithmParameter> vals_in_table;
  int rows = electrode_coil_tableWidget->rowCount();

  if (rows == inputPortsVector_.size() && rows == stimTypeVector_.size())
  {
    for (int i=0; i<rows; i++)
    {
      std::vector<Variable> values=validate_numerical_input(i);
      AlgorithmParameter row_i(Name("row" + boost::lexical_cast<std::string>(i)), values);
      vals_in_table.push_back(row_i);
    }
    saved_all_elc_values=vals_in_table;
    state_->setValue(Parameters::TableValues, vals_in_table);

   }
}

void ElectrodeCoilSetupDialog::initialize_comboboxes(int i, std::vector<AlgorithmParameter>& row)
{
  QStringList type_items;
  type_items<<"???"<<"TMS"<<"tDCS";
  QStringList inputports_items;
  QComboBox *InputPorts = new QComboBox();
  QComboBox *StimType = new QComboBox();
  inputports_items << "???";

  int nrinput = (state_->getValue(Parameters::NumberOfPrototypes)).toInt();
  for (int k=0;k<nrinput;k++)
  {
    std::ostringstream str;
    str << "USE_MODULE_INPUTPORT_" << k+3;
    inputports_items << QString::fromStdString(str.str());
  }
  InputPorts->addItems(inputports_items);

  if (i==0)
  {
    ProtoTypeInputComboBox_->setMaxCount(inputports_items.size());
    ProtoTypeInputComboBox_->clear();
    ProtoTypeInputComboBox_->addItems(inputports_items);
  }
  auto value = QString::fromStdString(state_->getValue(Parameters::ProtoTypeInputComboBox).toString());
  if (value != ProtoTypeInputComboBox_->currentText())
  {
    ProtoTypeInputComboBox_->setCurrentIndex(ProtoTypeInputComboBox_->findText(value));
  }
  StimType->addItems(type_items);

  int tmp1=-1, tmp2=-1;

  if (row.size()>=2)
   {
    int prototype_from_state=-1;
    try
    {
     prototype_from_state = boost::lexical_cast<int>(row[0].toString());
    } catch(boost::bad_lexical_cast &)
    {
     prototype_from_state=-1;
    }

    if (prototype_from_state>=0 && prototype_from_state<=nrinput)
      tmp1=prototype_from_state;

    int stimtype_from_state=-1;
    try
    {
     stimtype_from_state = boost::lexical_cast<int>(row[1].toString());
    } catch(boost::bad_lexical_cast &)
    {
     stimtype_from_state=-1;
    }

    if (stimtype_from_state>=0 && stimtype_from_state<=3)  /// TODO: make a static constant
      tmp2=stimtype_from_state;

    InputPorts->setCurrentIndex(tmp1);
    StimType->setCurrentIndex(tmp2);
  }

  electrode_coil_tableWidget->setCellWidget(i,0,InputPorts);
  electrode_coil_tableWidget->setCellWidget(i,1,StimType);
  inputPortsVector_.push_back(InputPorts);
  stimTypeVector_.push_back(StimType);
  connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
  connect(StimType, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
}

void ElectrodeCoilSetupDialog::pushComboBoxChange(int index)
{
  auto name = qobject_cast<QComboBox*>(sender())->currentText();
  push();
}

void ElectrodeCoilSetupDialog::pullSpecial()
{
  const bool AllTDCSInputsButton = state_->getValue(Parameters::AllInputsTDCS).toBool();
  const bool UseThisPrototypeButton = state_->getValue(Parameters::ProtoTypeInputCheckbox).toBool();
  auto all_elc_values = (state_->getValue(Parameters::TableValues)).toVector();

  if (all_elc_values!=saved_all_elc_values || !comboBoxesSetup_)
  {
    if (all_elc_values.size()>0)
    {
      std::string ProtoTypeInputComboBoxString = state_->getValue(Parameters::ProtoTypeInputComboBox).toString();

      if (all_elc_values.size()!=electrode_coil_tableWidget->rowCount())
         comboBoxesSetup_=false;

      electrode_coil_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));

      if (!comboBoxesSetup_)
      {
        inputPortsVector_.resize(0);
        stimTypeVector_.resize(0);
      }

      for (int i=0; i<all_elc_values.size(); i++)
      {
        auto row = (all_elc_values[i]).toVector();
        int j=0;
        for (const AlgorithmParameter& ap : row)
        {
          auto tmpstr = ap.toString();
          auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));

          if(j==0)
          {
            if (!comboBoxesSetup_)
            {
              ///let's call first pull() so that user only sees combo box
              initialize_comboboxes(i, row);
            }

          }

          if (j>=2)
            electrode_coil_tableWidget->setItem(i, j, item);

          ++j;
        }
      }

      // one-time table update from shortcut buttons.
      pushTableFlag_ = false;
      if (!comboBoxesSetup_ && UseThisPrototypeButton)
        togglePrototypeColumnReadOnly(1);
      if (!comboBoxesSetup_ && state_->getValue(Parameters::ElectrodethicknessCheckBox).toBool())
        toggleThicknessColumnReadOnly(1);
      if (!comboBoxesSetup_ && AllTDCSInputsButton)
        updateStimTypeColumn();

      comboBoxesSetup_ = true;
      pushTableFlag_ = true;
    }
  }
  else
  {

  }
}

void ElectrodeCoilSetupDialog::updateStimTypeColumn()
{
  const bool AllTDCSInputsButton = AllInputsTDCS_->isChecked();
  for (int i=0; i<stimTypeVector_.size(); i++)
  {
    auto box = (QComboBox *)stimTypeVector_[i];
    if(AllTDCSInputsButton)
    {
      box->blockSignals(true);
      electrode_coil_tableWidget->scrollToItem(electrode_coil_tableWidget->item(i, 1));
      box->setCurrentIndex((int)2);
      box->blockSignals(false);
    }
    box->setEnabled(!AllTDCSInputsButton);
  }
  if (AllTDCSInputsButton && pushTableFlag_)
    pushTable();
}

void ElectrodeCoilSetupDialog::updatePrototypeColumnValues(int index)
{
  const bool UseThisPrototypeButton = ProtoTypeInputCheckbox_->isChecked();

  for (int i=0; i<inputPortsVector_.size(); i++)
  {
    auto box = ((QComboBox *)inputPortsVector_[i]);
    if (UseThisPrototypeButton)
    {
      box->blockSignals(true);
      electrode_coil_tableWidget->scrollToItem(electrode_coil_tableWidget->item(i, 0));
      box->setCurrentIndex(index);
      box->blockSignals(false);
    }
  }
  if (UseThisPrototypeButton && pushTableFlag_)
    push();
}

void ElectrodeCoilSetupDialog::togglePrototypeColumnReadOnly(int state)
{
  const bool UseThisPrototypeButton = state != 0;

  for (int i=0; i<inputPortsVector_.size(); i++)
  {
    auto box = ((QComboBox *)inputPortsVector_[i]);
    box->setEnabled(!UseThisPrototypeButton);
  }
  if (UseThisPrototypeButton)
    updatePrototypeColumnValues(ProtoTypeInputComboBox_->currentIndex());
}

void ElectrodeCoilSetupDialog::updateThicknessColumnValues(double value)
{
  const bool ElectrodethicknessCheckBoxButton = electrodethicknessCheckBox_->isChecked();

  double ElectrodethicknessSpinBoxValue = value;
  QString text = QString::number(ElectrodethicknessSpinBoxValue);
  for (int i=0; i<electrode_coil_tableWidget->rowCount(); i++)
  {
    auto item = electrode_coil_tableWidget->item(i, 9);
    if (item)
    {
      if (ElectrodethicknessCheckBoxButton)
      {
        electrode_coil_tableWidget->blockSignals(true);
        item->setText(text);
        electrode_coil_tableWidget->scrollToItem(item);
        electrode_coil_tableWidget->blockSignals(false);
      }
    }
  }

  if (ElectrodethicknessCheckBoxButton && pushTableFlag_)
    pushTable();
}

void ElectrodeCoilSetupDialog::toggleThicknessColumnReadOnly(int state)
{
  const bool ElectrodethicknessCheckBoxButton = state != 0;

  for (int i=0; i<electrode_coil_tableWidget->rowCount(); i++)
  {
    auto item = electrode_coil_tableWidget->item(i, 9);
    if (item)
    {
      if (ElectrodethicknessCheckBoxButton)
      {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      }
      else
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
  }

  if (ElectrodethicknessCheckBoxButton)
    updateThicknessColumnValues(electrodethicknessSpinBox_->value());
}
