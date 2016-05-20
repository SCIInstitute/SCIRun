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

#include <Interface/Modules/Matlab/ImportFieldsFromMatlabDialog.h>
#include <Modules/Legacy/Matlab/DataIO/ImportFieldsFromMatlab.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#ifndef Q_MOC_RUN
#include <Core/Utils/StringUtil.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Matlab;

ImportFieldsFromMatlabDialog::ImportFieldsFromMatlabDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  portChoices_(Modules::Matlab::ImportFieldsFromMatlab::NUMPORTS, NONE_CHOICE)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addLineEditManager(fileNameLineEdit_, Variables::Filename);
  connect(openFileButton_, SIGNAL(clicked()), this, SLOT(openFile()));
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));
  connect(portListWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(portItemClicked(int)));
  connect(matlabObjectListWidget_, SIGNAL(currentRowChanged(int)), this, SLOT(matlabItemClicked(int)));
}

void ImportFieldsFromMatlabDialog::openFile()
{
  auto file = QFileDialog::getOpenFileName(this, "Open Matlab File", dialogDirectory(), "*.mat");
  if (file.length() > 0)
  {
    fileNameLineEdit_->setText(file);
    updateRecentFile(file);
    pushFileNameToState();
  }
}

void ImportFieldsFromMatlabDialog::pushFileNameToState()
{
  state_->setValue(Variables::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void ImportFieldsFromMatlabDialog::pullSpecial()
{
  auto fields = transient_value_cast<VariableList>(state_->getTransientValue(Parameters::FieldInfoStrings));
  auto infos = toStringVector(fields);
  auto names = toNameVector(fields);
  if (fieldNames_ != names) // probably need to compare info vectors too
  {
    fieldNames_ = names;

    matlabObjectListWidget_->clear();

    for (auto&& infoTup : zip(infos, fieldNames_))
    {
      std::string name, info;
      boost::tie(info, name) = infoTup;
      auto qinfo = QString::fromStdString(info);
      matlabObjectListWidget_->addItem(qinfo);
    }

    auto choices = toStringVector(state_->getValue(Parameters::PortChoices).toVector());
    size_t port = 0;
    for (const auto& choice : choices)
    {
      auto found = std::find(fieldNames_.cbegin(), fieldNames_.cend(), choice);
      if (found != fieldNames_.cend())
      {
        auto index = found - fieldNames_.cbegin();
        portChoices_[port] = index;
      }
      else
        portChoices_[port] = NONE_CHOICE;
      port++;
    }
    matlabObjectListWidget_->addItem("<none>");
  }
}

void ImportFieldsFromMatlabDialog::pushPortChoices()
{
  auto portChoices = makeHomogeneousVariableList([this](size_t i) { return portChoices_[i] >= 0 ? fieldNames_[portChoices_[i]] : "<none>"; }, portListWidget_->count());
  state_->setValue(Parameters::PortChoices, portChoices);
}

void ImportFieldsFromMatlabDialog::portItemClicked(int row)
{
  auto choice = portChoices_[row];
  if (NONE_CHOICE == choice)
    choice = matlabObjectListWidget_->count() - 1;
  
  if (choice >= 0)
    matlabObjectListWidget_->item(choice)->setSelected(true);
}

void ImportFieldsFromMatlabDialog::matlabItemClicked(int row)
{
  auto currentRow = portListWidget_->currentRow();
  if (currentRow >= 0)
    portChoices_[currentRow] = row;
  pushPortChoices();
}