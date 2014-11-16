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

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;


ElectrodeCoilSetupDialog::ElectrodeCoilSetupDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  electrode_coil_tableWidget->setRowCount(1);
  electrode_coil_tableWidget->setColumnCount(9);
  QStringList tableHeader;
  tableHeader<<"Input #"<<"Type"<<"X"<<"Y"<<"Z"<<"NX"<<"NY"<<"NZ"<<"thickness";
  electrode_coil_tableWidget->setHorizontalHeaderLabels(tableHeader);
  
  connect(electrode_coil_tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
}

void ElectrodeCoilSetupDialog::push()
{ 

  if (!pulling_)
  {
   std::vector<AlgorithmParameter> vals_in_table;
   int rows = electrode_coil_tableWidget->rowCount();
   std::cout << "dfdfdfdfdfdd i:" << rows << " " << InputPortsVector.size() << std::endl;
   if (rows != InputPortsVector.size() || rows != StimTypeVector.size())
   {
      std::cout << "Error: internal allocation of ComboBox vector array failed. " << std::endl;
   }
   
   for (int i=0; i<rows; i++)
   {
    std::vector<Variable> values;
    int inputport_ind=((QComboBox *)InputPortsVector[i])->currentIndex();
    int stimtype_ind=((QComboBox *)StimTypeVector[i])->currentIndex();
    
    std::cout << "inputport i:" << i << " ind:" << inputport_ind << std::endl;
    std::cout << "stimtype_ind i:" << i << " ind:" << stimtype_ind << std::endl;
    
    //QComboBox input_module_number = electrode_coil_tableWidget->item(i,0);
    //auto ttt = electrode_coil_tableWidget->getCellWidget(i,0);
    /*values.push_back(Variable(Name("Input #"), electrode_coil_tableWidget->item(i,0)->text().toDouble()));
    values.push_back(Variable(Name("X"), electrode_coil_tableWidget->item(i,1)->text().toDouble()));
    values.push_back(Variable(Name("Y"), electrode_coil_tableWidget->item(i,2)->text().toDouble()));
    values.push_back(Variable(Name("Z"), electrode_coil_tableWidget->item(i,3)->text().toDouble()));    
    values.push_back(Variable(Name("NX"), electrode_coil_tableWidget->item(i,4)->text().toDouble()));
    values.push_back(Variable(Name("NY"), electrode_coil_tableWidget->item(i,5)->text().toDouble()));
    values.push_back(Variable(Name("NZ"), electrode_coil_tableWidget->item(i,6)->text().toDouble()));    
    values.push_back(Variable(Name("thickness"), electrode_coil_tableWidget->item(i,7)->text().toDouble()));*/
    AlgorithmParameter row_i(Name("row" + boost::lexical_cast<std::string>(i)), values);   
    vals_in_table.push_back(row_i);
   }
   state_->setValue(Parameters::TableValues, vals_in_table);
  }
}

void ElectrodeCoilSetupDialog::pull()
{
  Pulling p(this);
 
  auto tableHandle = optional_any_cast_or_default<VariableHandle>(state_->getTransientValue(Parameters::TableValues));

  if (tableHandle)
  {
   auto all_elc_values = tableHandle->toVector();
   electrode_coil_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));
   InputPortsVector.resize(0);
   StimTypeVector.resize(0);
   QStringList type_items;
   type_items<<"???"<<"TMS"<<"tDCS";
   
   for (int i=0; i<all_elc_values.size(); i++)
   {
     auto col = (all_elc_values[i]).toVector();
     int j=0;
     BOOST_FOREACH(const AlgorithmParameter& ap, col)
     {
      auto tmpstr = ap.toString();
      
      auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
      
       if(j==0)
       {
        QStringList inputports_items;	
        QComboBox *InputPorts = new QComboBox();
	QComboBox *StimType = new QComboBox();
	inputports_items << QString::fromStdString("???");
	int nrinput = 0;
	try
	{
	  nrinput = boost::lexical_cast<int>(tmpstr);
	}
	catch( const boost::bad_lexical_cast & )
	{
	  //unable to convert
	}
	
	for (int k=0;k<nrinput;k++)
	{
	 std::ostringstream str;
         str << "USE_MODULE_INPUTPORT_" << std::to_string(k+3);
	 inputports_items << QString::fromStdString(str.str());  
	}
        InputPorts->addItems(inputports_items);
	StimType->addItems(type_items);
	electrode_coil_tableWidget->setCellWidget(i,0,InputPorts);
	electrode_coil_tableWidget->setCellWidget(i,1,StimType);	
	InputPortsVector.push_back(InputPorts);
	StimTypeVector.push_back(StimType);	
	connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(push()));
	connect(StimType, SIGNAL(currentIndexChanged(int)), this, SLOT(push()));	
	item = new QTableWidgetItem(QString::fromStdString(""));
	electrode_coil_tableWidget->setItem(i, j, item);
       } else
          electrode_coil_tableWidget->setItem(i, j, item);	 
      
      if (j>4 && j<=8) 
       electrode_coil_tableWidget->item(i,j)->setTextColor(Qt::red);
	else
	  electrode_coil_tableWidget->item(i,j)->setTextColor(Qt::black);
	    
       ++j;
     }
   }
  } else
  {
    // ?? log a message ? unsure.
  }
  //pull_newVersionToReplaceOld();
}

