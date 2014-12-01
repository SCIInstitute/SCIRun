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
  electrode_coil_tableWidget->setColumnCount(ElectrodeCoilSetupAlgorithm::number_of_columns);
  electrode_coil_tableWidget->setItem(0, 0, new QTableWidgetItem(QString::fromStdString(" ")));
  electrode_coil_tableWidget->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(" ")));
  QStringList tableHeader;
  tableHeader<<"Input #"<<"Type"<<"X"<<"Y"<<"Z"<<"Angle"<<"NX"<<"NY"<<"NZ"<<"thickness";
  electrode_coil_tableWidget->setHorizontalHeaderLabels(tableHeader);
  addCheckBoxManager(ProtoTypeInputCheckbox_, Parameters::ProtoTypeInputCheckbox);
  addCheckBoxManager(AllInputsTDCS_, Parameters::AllInputsTDCS); 
  addCheckBoxManager(electrodethicknessCheckBox_, Parameters::ElectrodethicknessCheckBox);  
  addDoubleSpinBoxManager(electrodethicknessSpinBox_, Parameters::ElectrodethicknessSpinBox);
  ProtoTypeInputCheckbox_->setChecked(false);
  AllInputsTDCS_->setChecked(false); 
  electrodethicknessCheckBox_->setChecked(false); 
  addComboBoxManager(ProtoTypeInputComboBox_, Parameters::ProtoTypeInputComboBox);  
//  connect(ProtoTypeInputComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(push()));  
  connect(AllInputsTDCS_, SIGNAL(clicked()), this, SLOT(push()));
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
    static const QString unknown("???");
    electrode_coil_tableWidget->item(row, col)->setText(unknown);
    
    auto all_elc_values = (state_->getValue(Parameters::TableValues)).toVector();    
    Variable::List table;
    for (int i=0;i<all_elc_values.size();i++)
    {
     auto row_vector = (all_elc_values[i]).toVector();
     if (i==row)
     {
       Variable new_var(row_vector[col].name(), unknown.toStdString());
       row_vector[col]=new_var;
     } 
     
     table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), row_vector));        
    }
    
    state_->setValue(Parameters::TableValues, table);
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
   std::cout << "push" << std::endl;
   /*state_->setValue(Parameters::AllTDCSInputs, AllInputsTDCS_->isChecked() ? std::string("1") : std::string("0"));
   state_->setValue(Parameters::ElectrodethicknessCheckBox, electrodethicknessCheckBox_->isChecked() ? std::string("1") : std::string("0"));
   state_->setValue(Parameters::UseThisPrototype, ProtoTypeInputCheckbox_->isChecked() ? std::string("1") : std::string("0"));*/

   std::vector<AlgorithmParameter> vals_in_table;
   int rows = electrode_coil_tableWidget->rowCount();

   if (rows == InputPortsVector.size() && rows == StimTypeVector.size())
   {
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
  
  if (i==0)
   { 
     ProtoTypeInputComboBox_->setMaxCount(inputports_items.size());
     ProtoTypeInputComboBox_->addItems(inputports_items);  
   }
  ProtoTypeInputComboBox_->setCurrentIndex(1);
  StimType->addItems(type_items);  
  if (saved_InputPortsVector.size()>0 && i<saved_InputPortsVector.size())
  {
    int tmp1=saved_InputPortsVector[i];
    int tmp2=saved_StimTypeVector[i];

    if (tmp1 > nrinput || tmp1<0)
        tmp1=0;
    
    if (tmp2 > nrinput || tmp2<0)
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
   std::cout << "pull" << std::endl;
  
   std::string ProtoTypeInputComboBoxString = state_->getValue(Parameters::ProtoTypeInputComboBox).toString(); 
   
   bool AllTDCSInputsButton = state_->getValue(Parameters::AllTDCSInputs).toBool(); 
   
   bool ElectrodethicknessCheckBoxButton = state_->getValue(Parameters::ElectrodethicknessCheckBox).toBool();
   
   bool UseThisPrototypeButton = state_->getValue(Parameters::UseThisPrototype).toBool();
    
   electrode_coil_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));
   
   /// remember the combobox settings
   saved_InputPortsVector.resize(InputPortsVector.size());
   saved_StimTypeVector.resize(InputPortsVector.size());
   if (InputPortsVector.size()!=StimTypeVector.size())
   {
     std::cout << "Internal error" << std::endl;
   }

   for (int i=0; i<InputPortsVector.size(); i++)
   {
     if (UseThisPrototypeButton && (int)(ProtoTypeInputComboBox_)->currentIndex()>0)
     {      
       saved_InputPortsVector[i]=((int)(ProtoTypeInputComboBox_)->currentIndex()); 
      } else
          saved_InputPortsVector[i]=((int)((QComboBox *)InputPortsVector[i])->currentIndex());
     
     if (!AllTDCSInputsButton)
       saved_StimTypeVector[i]=((int)((QComboBox *)StimTypeVector[i])->currentIndex());   
        else
          saved_StimTypeVector[i]=((int)(2));
   }
   
   if ( !(state_->getValue(Parameters::ComboBoxesAreSetup)).toBool())
   {
    InputPortsVector.resize(0);
    StimTypeVector.resize(0);  
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
   
   if(AllTDCSInputsButton)
   {
    for (int i=0; i<StimTypeVector.size(); i++)
    {
     ((QComboBox *)StimTypeVector[i])->setCurrentIndex((int)2);
    }
   }  
   if(UseThisPrototypeButton)
   {
    for (int i=0; i<InputPortsVector.size(); i++)
    {
     ((QComboBox *)InputPortsVector[i])->setCurrentIndex((int)(ProtoTypeInputComboBox_)->currentIndex());
    } 
   }
   if(ElectrodethicknessCheckBoxButton)
   {  
    double ElectrodethicknessSpinBoxValue = (state_->getValue(Parameters::ElectrodethicknessSpinBox)).toDouble();     
    QString text = QString::number(ElectrodethicknessSpinBoxValue);
    QDoubleValidator validator;
    
    int index;
    if (validator.validate(text, index) == QValidator::Acceptable)
    {   
     for (int i=0; i<all_elc_values.size(); i++)
     {
      electrode_coil_tableWidget->item(i, 9)->setText(text);
     }

    }
   }
  } 
  
  
  
  pull_newVersionToReplaceOld();
}

