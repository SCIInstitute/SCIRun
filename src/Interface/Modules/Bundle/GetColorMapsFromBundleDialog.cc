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


#include <Modules/Legacy/Bundle/GetColorMapsFromBundle.h>
#include <Interface/Modules/Bundle/GetColorMapsFromBundleDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Bundles;

GetColorMapsFromBundleDialog::GetColorMapsFromBundleDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  addComboBoxManager(colorMapComboBox1_, GetColorMapsFromBundle::ColorMapNames[0]);
  addComboBoxManager(colorMapComboBox2_, GetColorMapsFromBundle::ColorMapNames[1]);
  addComboBoxManager(colorMapComboBox3_, GetColorMapsFromBundle::ColorMapNames[2]);
  addComboBoxManager(colorMapComboBox4_, GetColorMapsFromBundle::ColorMapNames[3]);
  addComboBoxManager(colorMapComboBox5_, GetColorMapsFromBundle::ColorMapNames[4]);
  addComboBoxManager(colorMapComboBox6_, GetColorMapsFromBundle::ColorMapNames[5]);
}

void GetColorMapsFromBundleDialog::pullSpecial()
{
  auto names = transient_value_cast<std::vector<std::string>>(state_->getTransientValue(GetColorMapsFromBundle::ColorMapNameList.name()));
  if (matrixNames_ != names)
  {
    matrixNames_ = names;
    //TODO: put into an array, call functions.
    colorMapComboBox1_->clear();
    colorMapComboBox2_->clear();
    colorMapComboBox3_->clear();
    colorMapComboBox4_->clear();
    colorMapComboBox5_->clear();
    colorMapComboBox6_->clear();
    colorMapObjectListWidget_->clear();

    for (const auto& name : names)
    {
      auto qname = QString::fromStdString(name);
      colorMapObjectListWidget_->addItem(qname);
      colorMapComboBox1_->addItem(qname);
      colorMapComboBox2_->addItem(qname);
      colorMapComboBox3_->addItem(qname);
      colorMapComboBox4_->addItem(qname);
      colorMapComboBox5_->addItem(qname);
      colorMapComboBox6_->addItem(qname);
    }
    colorMapComboBox1_->setCurrentIndex(colorMapComboBox1_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[0]).toString())));
    colorMapComboBox2_->setCurrentIndex(colorMapComboBox2_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[1]).toString())));
    colorMapComboBox3_->setCurrentIndex(colorMapComboBox3_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[2]).toString())));
    colorMapComboBox4_->setCurrentIndex(colorMapComboBox4_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[3]).toString())));
    colorMapComboBox5_->setCurrentIndex(colorMapComboBox5_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[4]).toString())));
    colorMapComboBox6_->setCurrentIndex(colorMapComboBox6_->findText(QString::fromStdString(state_->getValue(GetColorMapsFromBundle::ColorMapNames[5]).toString())));
  }
}
