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
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  electrode_coil_tableWidget->setRowCount(1);
  electrode_coil_tableWidget->setColumnCount(10);
  QStringList tableHeader;
  tableHeader<<"Input #"<<"Type"<<"X"<<"Y"<<"Z"<<"Angle"<<"NX"<<"NY"<<"NZ"<<"thickness";
  electrode_coil_tableWidget->setHorizontalHeaderLabels(tableHeader);
  
  addCheckBoxManager(ProtoTypeInputCheckbox_, Parameters::ProtoTypeInputCheckbox);
  addCheckBoxManager(AllInputsTDCS_, Parameters::AllInputsTDCS); 
  ProtoTypeInputCheckbox_->setChecked(false);
  AllInputsTDCS_->setChecked(false);
  addComboBoxManager(ProtoTypeInputComboBox_, Parameters::ProtoTypeInputComboBox);
  connect(AllInputsTDCS_, SIGNAL(clicked()), this, SLOT(push()));
  connect(ProtoTypeInputCheckbox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(electrode_coil_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(validateCell(int, int)));
}

void ElectrodeCoilSetupDialog::validateCell(int row, int col)
{
  QString text = electrode_coil_tableWidget->item(row, col)->text();
  QDoubleValidator validator;
  int index;
  if (validator.validate(text, index) == QValidator::Acceptable)
  {
    push();
  }
  else
  {
    electrode_coil_tableWidget->item(row, col)->setText("???");
  }
}

std::vector<Variable> ElectrodeCoilSetupDialog::validate_numerical_input(int i)
{
    std::vector<Variable> values; 
    int inputport_ind=((QComboBox *)InputPortsVector[i])->currentIndex();
    int stimtype_ind=((QComboBox *)StimTypeVector[i])->currentIndex();
    static const std::string unknown("???");

    values.push_back(Variable(Name("Input #"), boost::lexical_cast<std::string>(inputport_ind)));
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
   if (AllInputsTDCS_->isChecked())
   {
     std::cout << "push: AllInputsTDCS_ true" << std::endl;
   } else
   {
     std::cout << "push: AllInputsTDCS_ false" << std::endl;
   }
   
   if (ProtoTypeInputCheckbox_->isChecked())
   {
     std::cout << "push: ProtoTypeInputCheckbox_ true" << std::endl;
   } else
   {
     std::cout << "push: ProtoTypeInputCheckbox_ false" << std::endl;
   }
   
   state_->setValue(Parameters::AllTDCSInputs, AllInputsTDCS_->isChecked() ? std::string("1") : std::string("0"));
   state_->setValue(Parameters::UseThisPrototype, ProtoTypeInputCheckbox_->isChecked() ? std::string("1") : std::string("0"));
   
   
   std::vector<AlgorithmParameter> vals_in_table;
   int rows = electrode_coil_tableWidget->rowCount();

   if (rows != InputPortsVector.size() || rows != StimTypeVector.size())
   {
      std::cout << "Error: internal allocation of ComboBox vector array failed. " << std::endl;
   }
   
   std::string str;
   for (int i=0; i<rows; i++)
   {
    std::vector<Variable> values=validate_numerical_input(i);
    AlgorithmParameter row_i(Name("row" + boost::lexical_cast<std::string>(i)), values);   
    vals_in_table.push_back(row_i);
   }
   state_->setValue(Parameters::TableValues, vals_in_table);
  }
}

void ElectrodeCoilSetupDialog::initialize_comboboxes(int i, std::string& tmpstr)
{
  QStringList type_items;
  type_items<<"???"<<"TMS"<<"tDCS";
  QStringList inputports_items;  
  QComboBox *InputPorts = new QComboBox();
  QComboBox *StimType = new QComboBox();
  inputports_items << QString::fromStdString("???");
  
  int nrinput = (state_->getValue(Parameters::NumberOfPrototypes)).toInt();
    
  for (int k=0;k<nrinput;k++)
  {
   std::ostringstream str;
   str << "USE_MODULE_INPUTPORT_" << k+3;
   inputports_items << QString::fromStdString(str.str());  
  }
  InputPorts->addItems(inputports_items);
  StimType->addItems(type_items);  
  
  if (saved_InputPortsVector.size()>0 && i<saved_InputPortsVector.size())
  {
    int tmp1=saved_InputPortsVector[i];
    int tmp2=saved_StimTypeVector[i];
    if (tmp1 > nrinput+1)
        tmp1=0;
    if (tmp2 > nrinput+1)
        tmp2=0;
   
    InputPorts->setCurrentIndex(tmp1);
    StimType->setCurrentIndex(tmp2);
  }
  
  electrode_coil_tableWidget->setCellWidget(i,0,InputPorts);
  electrode_coil_tableWidget->setCellWidget(i,1,StimType);	 
  InputPortsVector.push_back(InputPorts);
  StimTypeVector.push_back(StimType);	 
  connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(push()));
  connect(StimType, SIGNAL(currentIndexChanged(int)), this, SLOT(push()));	 
}

void ElectrodeCoilSetupDialog::pull()
{
  Pulling p(this);
 
  auto all_elc_values = (state_->getValue(Parameters::TableValues)).toVector();

  if (all_elc_values.size()>0)
  {
   
   if (AllInputsTDCS_->isChecked())
   {
     std::cout << "pull: AllInputsTDCS_ true" << std::endl;
   } else
   {
     std::cout << "pull: AllInputsTDCS_ false" << std::endl;
   }
   
   if (ProtoTypeInputCheckbox_->isChecked())
   {
     std::cout << "pull: ProtoTypeInputCheckbox_ true" << std::endl;
   } else
   {
     std::cout << "pull: ProtoTypeInputCheckbox_ false" << std::endl;
   }
   //std::cout << "pull " << std::endl;
     
   auto button = state_->getValue(Parameters::AllTDCSInputs).toString();
   bool AllTDCSInputsButton("1" == button);
   AllInputsTDCS_->setChecked(AllTDCSInputsButton);
   
   button = state_->getValue(Parameters::UseThisPrototype).toString();
   bool UseThisPrototypeButton("1" == button);
   ProtoTypeInputCheckbox_->setChecked(UseThisPrototypeButton);
   
   electrode_coil_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));
   
   //remember the combobox settings
   saved_InputPortsVector.resize(0);
   saved_StimTypeVector.resize(0);
   if (InputPortsVector.size()!=StimTypeVector.size())
   {
     std::cout << "Internal error" << std::endl;
   }

   for (int i=0; i<InputPortsVector.size(); i++)
   {
    saved_InputPortsVector.push_back((int)((QComboBox *)InputPortsVector[i])->currentIndex());
    saved_StimTypeVector.push_back((int)((QComboBox *)StimTypeVector[i])->currentIndex());
   }

   if ( !(state_->getValue(Parameters::ComboBoxesAreSetup)).toBool())
   {
    InputPortsVector.resize(0);
    StimTypeVector.resize(0);  
   }
   
   for (int i=0; i<all_elc_values.size(); i++)
   {
     auto col = (all_elc_values[i]).toVector();
     int j=0;
     BOOST_FOREACH(const AlgorithmParameter& ap, col)
     {
      auto tmpstr = ap.toString();
      //std::cout << "a:" << tmpstr << std::endl;
      auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
      
       if(j==0)
       {
	 if ( !(state_->getValue(Parameters::ComboBoxesAreSetup)).toBool())
	 {
	  initialize_comboboxes(i, tmpstr); 
	 }
       } 
             
       if (j>=2)
          electrode_coil_tableWidget->setItem(i, j, item);	    
	    
       ++j;
     }
   }
   
   if ( !(state_->getValue(Parameters::ComboBoxesAreSetup)).toBool())
   {
    state_->setValue(Parameters::ComboBoxesAreSetup, true);
   }
  } 
  
  
  
  pull_newVersionToReplaceOld();
}

