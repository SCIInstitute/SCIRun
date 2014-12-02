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

#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Interface/Modules/BrainStimulator/ElectrodeCoilSetupDialog.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace boost;

ElectrodeCoilSetupDialog::ElectrodeCoilSetupDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  comboBoxesSetup_(false)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

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
  addDoubleSpinBoxManager(electrodethicknessSpinBox_, Parameters::ElectrodethicknessSpinBox);
  addComboBoxManager(ProtoTypeInputComboBox_, Parameters::ProtoTypeInputComboBox);  
  connect(electrode_coil_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(validateCell(int, int)));
  connect(AllInputsTDCS_, SIGNAL(stateChanged(int)), this, SLOT(updateStimTypeColumn()));
  connect(ProtoTypeInputCheckbox_, SIGNAL(stateChanged(int)), this, SLOT(updatePrototypeColumn()));
  connect(ProtoTypeInputComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePrototypeColumn()));
  connect(electrodethicknessCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(updateThicknessColumn()));
  connect(electrodethicknessSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateThicknessColumn()));
}

void ElectrodeCoilSetupDialog::validateCell(int row, int col)
{
  std::cout << "validateCell " << row << ", " << col << std::endl;
  auto cell = electrode_coil_tableWidget->item(row, col);
  QString text = cell->text();
  QDoubleValidator validator;
  int index;
  if (validator.validate(text, index) == QValidator::Acceptable)
  {
    std::cout << "acceptable" << std::endl;
  }
  else
  {
    std::cout << "unacceptable" << std::endl;
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

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,2)->text()).toStdString());
    values.push_back(Variable(Name("X"), electrode_coil_tableWidget->item(i,2)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("X"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,3)->text()).toStdString());
    values.push_back(Variable(Name("Y"), electrode_coil_tableWidget->item(i,3)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("Y"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,4)->text()).toStdString());
    values.push_back(Variable(Name("Z"), electrode_coil_tableWidget->item(i,4)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("Z"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,5)->text()).toStdString());
    values.push_back(Variable(Name("Angle"), electrode_coil_tableWidget->item(i,5)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("Angle"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,6)->text()).toStdString());
    values.push_back(Variable(Name("NX"), electrode_coil_tableWidget->item(i,6)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("NX"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,7)->text()).toStdString());
    values.push_back(Variable(Name("NY"), electrode_coil_tableWidget->item(i,7)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("NY"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,8)->text()).toStdString());
    values.push_back(Variable(Name("NZ"), electrode_coil_tableWidget->item(i,8)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("NZ"), unknown));
  }

  try
  {
    lexical_cast<double>((electrode_coil_tableWidget->item(i,9)->text()).toStdString());
    values.push_back(Variable(Name("thickness"), electrode_coil_tableWidget->item(i,9)->text().toStdString()));
  }
  catch(bad_lexical_cast &)
  {
    values.push_back(Variable(Name("thickness"), unknown));
  }

  return values;
}

void ElectrodeCoilSetupDialog::push()
{ 
  if (!pulling_)
  {
    std::cout << "\n\n!!! push !!!\n\n" << std::endl;

    pushTable();
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

    std::cout << "______________push table:\n" << vals_in_table << std::endl;
  }
}

void ElectrodeCoilSetupDialog::initialize_comboboxes(int i, std::string& tmpstr)
{
  std::cout << "@@@ initialize_comboboxes: " << i << std::endl;
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
  ProtoTypeInputComboBox_->setCurrentIndex(1);
  StimType->addItems(type_items);  
  if (savedInputPortsVector_.size()>0 && i<savedInputPortsVector_.size())
  {
    int tmp1=savedInputPortsVector_[i];
    int tmp2=savedStimTypeVector_[i];

    if (tmp1 > nrinput || tmp1<0)
      tmp1=0;

    if (tmp2 > nrinput || tmp2<0)
      tmp2=0;

    InputPorts->setCurrentIndex(tmp1);
    StimType->setCurrentIndex(tmp2);
  }
  electrode_coil_tableWidget->setCellWidget(i,0,InputPorts);
  electrode_coil_tableWidget->setCellWidget(i,1,StimType);	 
  inputPortsVector_.push_back(InputPorts);
  stimTypeVector_.push_back(StimType);	
  connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
  connect(StimType, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
  std::cout << "@@@ end init_comb " << i << std::endl;
}

void ElectrodeCoilSetupDialog::pushComboBoxChange(int index)
{
  auto name = qobject_cast<QComboBox*>(sender())->currentText();
  std::cout << "dropdown change: " << name.toStdString() << " new index: " << index << std::endl;
  push();
}

void ElectrodeCoilSetupDialog::pull()
{
  pull_newVersionToReplaceOld();
  
  Pulling p(this);
  
  const bool AllTDCSInputsButton = state_->getValue(Parameters::AllInputsTDCS).toBool(); 
  std::cout << "ALLTDCS: " << AllTDCSInputsButton << std::endl;

  const bool UseThisPrototypeButton = state_->getValue(Parameters::ProtoTypeInputCheckbox).toBool();
  std::cout << "UseThisPrototypeButton: " << UseThisPrototypeButton << std::endl;

  auto all_elc_values = (state_->getValue(Parameters::TableValues)).toVector();

  if (all_elc_values!=saved_all_elc_values || !comboBoxesSetup_)
  {
    if (!comboBoxesSetup_)
      std::cout << "combo boxes not set up yet" << std::endl;
    else
      std::cout << "unequal state vectors: \n~~~new:~~~\n" << all_elc_values << "\n\n~~~old:~~~\n" << saved_all_elc_values << std::endl;

    if (all_elc_values.size()>0)
    {
      std::cout << "pull" << std::endl;

      std::string ProtoTypeInputComboBoxString = state_->getValue(Parameters::ProtoTypeInputComboBox).toString(); 

      

      electrode_coil_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));

      /// remember the combobox settings
      savedInputPortsVector_.resize(inputPortsVector_.size());
      savedStimTypeVector_.resize(inputPortsVector_.size());
      if (inputPortsVector_.size()!=stimTypeVector_.size())
      {
        std::cout << "Internal error" << std::endl;
      }

      for (int i=0; i<inputPortsVector_.size(); i++)
      {
        if (UseThisPrototypeButton && (int)(ProtoTypeInputComboBox_)->currentIndex()>0)
        {      
          savedInputPortsVector_[i]=((int)(ProtoTypeInputComboBox_)->currentIndex()); 
        } else
          savedInputPortsVector_[i]=((int)((QComboBox *)inputPortsVector_[i])->currentIndex());

        if (!AllTDCSInputsButton)
          savedStimTypeVector_[i]=((int)((QComboBox *)stimTypeVector_[i])->currentIndex());   
        else
          savedStimTypeVector_[i]=((int)(2));
      }

      if (!comboBoxesSetup_)
      {
        inputPortsVector_.resize(0);
        stimTypeVector_.resize(0);  
      }

      for (int i=0; i<all_elc_values.size(); i++)
      {
        auto row = (all_elc_values[i]).toVector();
        int j=0;
        BOOST_FOREACH(const AlgorithmParameter& ap, row)
        {
          auto tmpstr = ap.toString();
          auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));

          if(j==0)
          {
            if (!comboBoxesSetup_)
            {
              //let's call first pull() so that user only sees combo box
              initialize_comboboxes(i, tmpstr); 
            }

          } 

          if (j>=2)
            electrode_coil_tableWidget->setItem(i, j, item);	    

          ++j;
        }
      }

      comboBoxesSetup_ = true;
    } 
  }
  else
  {
    std::cout << "equal state vectors: " << saved_all_elc_values << std::endl;
  }
}

void ElectrodeCoilSetupDialog::updateStimTypeColumn()
{
  const bool AllTDCSInputsButton = AllInputsTDCS_->isChecked();
  //std::cout << "ALLTDCS: " << AllTDCSInputsButton << std::endl;
  for (int i=0; i<stimTypeVector_.size(); i++)
  {
    auto box = (QComboBox *)stimTypeVector_[i];
    if(AllTDCSInputsButton)
    {
      box->blockSignals(true);
      box->setCurrentIndex((int)2);
      std::cout << "---changing stim type combobox to 2" << std::endl;
      box->blockSignals(false);
    }
    box->setEnabled(!AllTDCSInputsButton);
  }
  if (AllTDCSInputsButton)
    pushTable();
}

void ElectrodeCoilSetupDialog::updatePrototypeColumn()
{
  const bool UseThisPrototypeButton = ProtoTypeInputCheckbox_->isChecked();
  //std::cout << "UseThisPrototypeButton: " << UseThisPrototypeButton << std::endl;

  for (int i=0; i<inputPortsVector_.size(); i++)
  {
    auto box = ((QComboBox *)inputPortsVector_[i]);
    if(UseThisPrototypeButton)
    {
      box->blockSignals(true);
      std::cout << "---changing input type combobox to " << (int)(ProtoTypeInputComboBox_)->currentIndex() << std::endl;
      box->setCurrentIndex((int)(ProtoTypeInputComboBox_)->currentIndex());
      box->blockSignals(false);
    } 
    box->setEnabled(!UseThisPrototypeButton);
  }
  if (UseThisPrototypeButton)
    pushTable();
}

void ElectrodeCoilSetupDialog::updateThicknessColumn()
{
  const bool ElectrodethicknessCheckBoxButton = electrodethicknessCheckBox_->isChecked();
  std::cout << "ElectrodethicknessCheckBoxButton: " << ElectrodethicknessCheckBoxButton << " count = " << electrode_coil_tableWidget->rowCount() <<std::endl;

  double ElectrodethicknessSpinBoxValue = (state_->getValue(Parameters::ElectrodethicknessSpinBox)).toDouble();     
  QString text = QString::number(ElectrodethicknessSpinBoxValue);
  for (int i=0; i<electrode_coil_tableWidget->rowCount(); i++)
  {
    auto item = electrode_coil_tableWidget->item(i, 9);

    if(ElectrodethicknessCheckBoxButton)
    {
      electrode_coil_tableWidget->blockSignals(true);
      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      item->setText(text);
      electrode_coil_tableWidget->scrollToItem(item);
      electrode_coil_tableWidget->blockSignals(false);
    }
    else
      item->setFlags(item->flags() | Qt::ItemIsEditable);
  }

  if (ElectrodethicknessCheckBoxButton)
    pushTable();
}
